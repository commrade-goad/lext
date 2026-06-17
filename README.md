# Lext: s7 Scheme Interpreter with libffi FFI

`lext` is a custom Scheme interpreter powered by the **s7 Lisp** engine, patched with a robust, ABI-compliant **Foreign Function Interface (FFI)** leveraging `libffi` and some other goodies. It supports calling C functions with scalar parameters, pointers, and custom structs/unions passed by value or reference.

It can operate in two modes:
1. **Scheme Script Runner**: Run pure Scheme scripts directly without any template parsing or extra outputs.
2. **Template Preprocessing Mode**: Process input files with embedded Scheme blocks delimited by `@@(...)` and render the output.

---

## Features

- **Embedded s7 Scheme Engine**: A modern, feature-rich Lisp engine designed for extension and embedding.
- **Dynamic FFI Binding**: Load any `.so` dynamic library at runtime and call its functions.
- **Strict ABI Compliance**: Pass and return primitive types, pointers, structs, and unions by value and reference.

---

## Requirements & Building

### Dependencies
You need a C compiler (`gcc`), GNU `make`, and `libffi` development headers.

### Build Lext
If you dont have the old compatible version of lext then you need to compile the first bootstrap interpreter by running:
```bash
make
```
This produces the `./lext` binary.

After that use that `./lext` binary to build the full featured intepreter by using this command:
```bash
LEXT_HOME=. ./lext -s build.lext
```

This will compile and link the `lext` binary, generating a `build` directory containing the final executable `build/lext`.

If you decide to install it you can run this command:
```bash
PREFIX=$HOME/.local/ LEXT_HOME=. ./lext build.lext install
```

It will log what it do and install `lib` and `bin` at the specified `PREFIX`.

To uninstall you can just do:
```bash
LEXT_HOME=. ./lext build.lext uninstall
```

__NOTE:__

On Unix like platform it will make a new script called `lx` that will embed the LEXT_HOME with your setup so you dont need to setup the env manually.

---

## Usage Guide

### 1. Pure Lext-Scheme Script Runner Mode
To run a raw Lext Scheme script directly without template parsing:
```bash
LEXT_HOME=. ./lext script.lext arg1 arg2 arg3 argn # argv can be accessed using *argv* or argv
```

### 2. Template Preprocessor Mode
To evaluate embedded Lisp/Scheme code blocks (`@@(...)`) inside template files (e.g. LaTeX, HTML, text):
```bash
LEXT_HOME=. ./lext -t input.lext output.tex
```

#### Raw String Literals in Templates
To simplify writing strings containing backslashes (like LaTeX commands) or double quotes without verbose manual escaping, you can use raw strings inside evaluation blocks:
* **Syntax**: `r" ... "`
* Within that block all characters are captured raw.
* Backslashes `\` to `\\` in the generated Scheme string literal.
* **Example**:
  ```latex
  @@(string-append "Rendered LaTeX command: " r"\texttt{foo} and \"bar\"")
  ```
  This evaluates to:
  ```latex
  Rendered LaTeX command: \texttt{foo} and "bar"
  ```

### 3. Module System (`use`) & Namespaces
To easily structure your code, `lext` includes a module loading system. You specify root directories where modules are stored using the `LEXT_HOME` environment variable (using a colon `:` as a path separator).

The `use` function locates and loads the `lib.lext` script inside the specified module subdirectory (e.g. `LEXT_HOME/stdlib/basic/lib.lext`).

#### Dynamic Prefix Namespacing
When you use a module, Lext dynamically prefixes all of its exported symbols with the last path segment of the module string.
* Loading `(use "stdlib/libnob")` defines its symbols as `libnob.cmd-run`, `libnob.delete-file`, etc.
* Loading `(use "stdlib/basic")` defines its symbols as `basic.for`, `basic.while`, etc.
* Loading `(use "stdlib/c")` defines its symbols as `c.malloc`, `c.free`, etc.

#### Project-Local Modules (`:`)
If a module path starts with `:`, Lext bypasses `LEXT_HOME` entirely and resolves the module relative to the current working directory (`cwd`):
* Loading `(use ":stdlib/libnob")` searches for `./stdlib/libnob/lib.lext` in the current directory and prefixes its symbols as `libnob.`.
This prevents collisions with standard libraries located in `LEXT_HOME` and simplifies local project dependencies.

#### Module Exports
Inside a module's `lib.lext` file, the public API is declared using the `(export ...)` macro:
```scheme
;; Inside stdlib/basic/lib.lext
(export while foreach for capture shift)
```
Only symbols listed in `export` (or all symbols not prefixed with `internal-` if `export` is omitted) will be exported to the caller's environment.

#### Removing Prefixes
To strip prefixes and use functions prefix-free:
1. **Globally (`open-namespace`)**:
   ```scheme
   (open-namespace "basic") ;; Expose basic symbols globally
   (open-namespace "c")     ;; Expose c symbols globally (e.g. malloc, free)
   ```
2. **Locally / Scoped (`use-namespace`)**:
   ```scheme
   (use-namespace "libnob"
     (cmd-run '("echo" "hello"))) ;; Exposed only within this block
   ```

### 4. Running the Test Suite (TODO add test target into the build.lext)
Lext includes a heavy stress test suite to verify correct FFI behavior, memory boundaries, and standard library modules.
To run the stress test suite, run:
```bash
LEXT_HOME=. ./lext test/test_stress.lext
```
This runs all checks (including FFI struct/union by value passing, callbacks, and automatic bounds tracking/error catching).

---

## Standard Library Modules

`lext` ships with a set of modular libraries under the `stdlib` directory.

### 1. `basic` Module (`stdlib/basic`)
Contains core language extensions for pure Scheme. Loaded via `(use "stdlib/basic")`. Typically, you open its namespace globally using `(open-namespace "basic")`.

### 2. `c` Module (`stdlib/c`)
Contains low-level C FFI wrappers, libc allocations, and type definitions. Loaded via `(use "stdlib/c")`. Typically, you open its namespace globally using `(open-namespace "c")` after basic is opened.

### 3. `s7` Compatibility & Linter Modules (`stdlib/s7`)
Provides standard s7 Scheme extensions and developer tools, fully compatible with the native FFI runtime:
* **`stdlib/s7/r7rs`**: Scheme R7RS standard compatibility.
* **`stdlib/s7/lint`**: Native s7 Scheme code linter.
  ```scheme
  (use "stdlib/s7/lint")
  (lint.lint "my_script.lext")
  ```
* **Other Modules**: `json`, `loop`, `case`, `mockery`, `profile`, `reactive`, `write`, `stuff` can be imported in the same manner.

### 4. Built-in Hash Table Support
The underlying `s7` engine features native, high-performance hash tables. You do not need external libraries for hash map operations. Built-in procedures include:

* `(make-hash-table [size])` - Creates a new hash table.
* `(hash-table-set! table key value)` - Associates key with value in table.
* `(hash-table-ref table key)` - Retrieves value associated with key.
* `(hash-table? obj)` - Returns `#t` if the object is a hash table.

---

## Code Examples

### Example 1: Standard C Math & IO (Process Space)
```scheme
(define lib (ffi-open #f))

;; Call double cos(double) from libm
(define cos-sym (ffi-sym lib "cos"))
(define result (ffi-call cos-sym 'double '(double) '(0.0)))
(display (format #f "cos(0.0) = ~A\n" result)) ; Prints: cos(0.0) = 1.0

;; Call int puts(const char*) from libc
(define puts-sym (ffi-sym lib "puts"))
(ffi-call puts-sym 'int '(string) '("Hello from libc!"))

;; Call variadic int printf(const char* format, ...) from libc (1 fixed arg, 3 total args)
;; Note: We only need to list the fixed type '(string), the rest are inferred!
(define printf-sym (ffi-sym lib "printf"))
(ffi-call printf-sym 'int '(string) '("Hello %s, your lucky number is %d and float is %f!\n" "Lisp Hacker" 42 3.14) 1)
```

### Example 2: Structs and Unions by Value
Suppose you have the following C code compiled in `libtest.so`:
```c
typedef struct {
    int x;
    int y;
} Point;

typedef union {
    int i;
    double d;
} IntOrDouble;

Point add_points(Point p1, Point p2) {
    Point r = {p1.x + p2.x, p1.y + p2.y};
    return r;
}

double print_union(IntOrDouble u, int is_double) {
    return is_double ? u.d : (double)u.i;
}
```

You can register and call these in Scheme:
```scheme
(define test-lib (ffi-open "./libtest.so"))

;; Register typedefs
(ffi-typedef 'Point '(struct int int))
(ffi-typedef 'IntOrDouble '(union int double))

;; Call struct by value function
(define add-points-sym (ffi-sym test-lib "add_points"))
(define res-point (ffi-call add-points-sym 'Point '(Point Point) '((10 20) (30 40))))
(display (format #f "Point Result: x=~A, y=~A\n" (car res-point) (cadr res-point))) ; Prints: x=40, y=60

;; Call union by value function
(define print-union-sym (ffi-sym test-lib "print_union"))
(define res-union (ffi-call print-union-sym 'double '(IntOrDouble int) '((1 3.14) 1)))
(display (format #f "Union double val: ~A\n" res-union)) ; Prints: 3.14
```

### Example 3: Opening a SDL2 Window
```scheme
(define sdl (ffi-open "libSDL2.so"))

(define sdl-init (ffi-sym sdl "SDL_Init"))
(define sdl-create-window (ffi-sym sdl "SDL_CreateWindow"))
(define sdl-delay (ffi-sym sdl "SDL_Delay"))
(define sdl-destroy-window (ffi-sym sdl "SDL_DestroyWindow"))
(define sdl-quit (ffi-sym sdl "SDL_Quit"))

;; Initialize Video (SDL_INIT_VIDEO = 32)
(ffi-call sdl-init 'int '(int) '(32))

;; Create SDL2 window (SDL_WINDOWPOS_UNDEFINED = 536805376, SDL_WINDOW_SHOWN = 4)
(define win (ffi-call sdl-create-window 'pointer 
                      '(string int int int int int) 
                      '("FFI SDL Window" 536805376 536805376 640 480 4)))

;; Hold window for 3 seconds
(ffi-call sdl-delay 'void '(int) '(3000))

;; Clean up window and quit
(ffi-call sdl-destroy-window 'void '(pointer) (list win))
(ffi-call sdl-quit 'void '() '())
```

### Example 4: SDL2 Event Polling with Pointer Casting
Below is a complete loop that clears the screen to blue, polls events, checks their type via casting, and exits when closed or after 5 seconds:

```scheme
(use "stdlib/basic" "stdlib/c")
(open-namespace "basic")
(open-namespace "c")

(define sdl (ffi-open "libSDL2.so"))

;; Constants
(define SDL_RENDERER_ACCELERATED 2)
(define SDL_INIT_VIDEO 32)
(define SDL_QUIT #x100)
(define SDL_KEYDOWN #x300)
(define SDL_MOUSEMOTION #x400)

;; Imports
(c-import sdl-init                 sdl "SDL_Init"               int     (int))
(c-import sdl-create-window        sdl "SDL_CreateWindow"       pointer (string int int int int int))
(c-import sdl-create-renderer      sdl "SDL_CreateRenderer"     pointer (pointer int int))
(c-import sdl-set-render-drawcolor sdl "SDL_SetRenderDrawColor" void    (pointer int int int int))
(c-import sdl-render-clear         sdl "SDL_RenderClear"        void    (pointer))
(c-import sdl-render-present       sdl "SDL_RenderPresent"      void    (pointer))
(c-import sdl-delay                sdl "SDL_Delay"              void    (int))
(c-import sdl-destroy-window       sdl "SDL_DestroyWindow"      void    (pointer))
(c-import sdl-quit                 sdl "SDL_Quit"               void    ())
(c-import sdl-pollevent            sdl "SDL_PollEvent"          int     (pointer))
(c-import sdl-get-ticks            sdl "SDL_GetTicks"           int     ())

;; Define event-specific layouts
(ffi-typedef 'SDL_KeyboardEvent
             '(struct (uint32 type)
                      (uint32 timestamp)
                      (uint32 windowID)
                      (uint8 state)
                      (uint8 repeat)
                      (uint8 padding2)
                      (uint8 padding3)
                      (int32 scancode)
                      (int32 sym)
                      (uint16 mod)
                      (uint32 unused)))

(ffi-typedef 'SDL_MouseMotionEvent
             '(struct (uint32 type)
                      (uint32 timestamp)
                      (uint32 windowID)
                      (uint32 which)
                      (uint32 state)
                      (int32 x)
                      (int32 y)
                      (int32 xrel)
                      (int32 yrel)))

(sdl-init SDL_INIT_VIDEO)
(define win (sdl-create-window "FFI SDL Window" 0 0 640 480 4))
(define ren (sdl-create-renderer win -1 SDL_RENDERER_ACCELERATED))

;; Allocate event buffer using standard safety pattern (56 bytes max event size)
(define event-ptr (calloc 1 (size 'SDL_KeyboardEvent)))
(define start-time (sdl-get-ticks))

(let loop ()
  (if (> (sdl-pollevent event-ptr) 0)
      (let ((type (ffi-deref event-ptr 'int)))
        (cond
          ((= type SDL_QUIT)
           (display "Closed!\n"))
          ((= type SDL_MOUSEMOTION)
           (let ((m (ffi-deref event-ptr 'SDL_MouseMotionEvent)))
             (format #t "Mouse motion: ~A, ~A\n" (cdr (assoc 'x m)) (cdr (assoc 'y m)))
             (loop)))
          (else (loop))))
      (begin
        (sdl-set-render-drawcolor ren 0 0 255 255)
        (sdl-render-clear ren)
        (sdl-render-present ren)
        (sdl-delay 16)
        (if (< (- (sdl-get-ticks) start-time) 5000) (loop)))))

(free event-ptr)
(sdl-destroy-window win)
(sdl-quit)
```

---


### Binding C Functions to Scheme Functions (`c-import`)

Instead of invoking `ffi-call` with raw function pointer variables and verbose argument lists every time, you can use the `c-import` macro (provided out-of-the-box by the `stdlib/c` module) to bind any C function to a clean Scheme function wrapper.

#### Usage Example:
```scheme
(use "stdlib/c")
(open-namespace "c")

(define libc (ffi-open #f))

;; Import standard functions
(c-import c-cos libc "cos" double (double))
(c-import c-puts libc "puts" int (string))

;; Import variadic function (1 fixed type, trailing types auto-inferred)
(c-import c-printf libc "printf" int (string) 1)

;; Call them naturally like native Scheme functions!
(c-cos 0.0)                                             ; Returns 1.0
(c-puts "Hello from bound puts!")
(c-printf "Formatted output: %s %d %f!\n" "Scheme" 42 3.14)
```

---

## Editor Suppor
There is also an emacs major mode shipped with this project at [goodies](./goodies/lext-mode.el)
