# Lext: s7 Scheme Interpreter with libffi FFI

`lext` is a custom Scheme interpreter powered by the **s7 Lisp** engine, patched with a robust, ABI-compliant **Foreign Function Interface (FFI)** leveraging `libffi`. It supports calling C functions with scalar parameters, pointers, and custom structs/unions passed by value or reference.

It can operate in two modes:
1. **Scheme Script Runner**: Run pure Scheme scripts directly without any template parsing or extra outputs.
2. **Template Preprocessing Mode**: Process input files with embedded Scheme blocks delimited by `@@(...)` and render the output.

---

## Features

- **Embedded s7 Scheme Engine**: A modern, feature-rich Lisp engine designed for extension and embedding.
- **Dynamic FFI Binding**: Load any `.so` dynamic library at runtime and call its functions.
- **Strict ABI Compliance**: Pass and return primitive types, pointers, structs, and unions by value and reference.
- **Command-Line Interface**:
  - Run pure scripts with `-s` or `--no-template`.

---

## Requirements & Building

### Dependencies
You need a C compiler (`gcc`), GNU `make`, and `libffi` development headers.

On Debian/Ubuntu-based systems:
```bash
sudo apt install build-essential libffi-dev pkg-config
```

On Fedora/RHEL:
```bash
sudo dnf install make gcc libffi-devel pkgconf
```

### Build Lext
Compile the interpreter by running:
```bash
make
```
This produces the `./lext` binary.

You can also let `lext` build itself with `build.lext`. You just need `lext` already compiled and run:
```bash
LEXT_HOME=. ./lext -s build.lext
```

This will compile and link the `lext` binary, generating a `build` directory containing the final executable `build/lext`.

---

## Usage Guide

### 1. Pure Scheme Script Runner Mode (`-s` / `--no-template`)
To run a raw Lext Scheme script directly without template parsing, use the `-s` or `--no-template` flag:
```bash
LEXT_HOME=. ./lext -s script.lext
```
*Note: In script runner mode, specifying an output file is not allowed and will raise an error.*

### 2. Template Preprocessor Mode
To evaluate embedded Lisp/Scheme code blocks (`@@(...)`) inside template files (e.g. LaTeX, HTML, text):
```bash
./lext input.texm output.tex
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


### 3. Command-Line Arguments Passing
You can pass custom command-line arguments to your Lext scripts by adding `--` after the script name. Any arguments following `--` are collected into a list of strings and bound to the global variables `argv` and `*argv*` in the Scheme interpreter:
```bash
LEXT_HOME=. ./lext -s script.lext -- arg1 arg2 "hello world"
```

Inside your script:
```scheme
(display argv)
;; Output: ("arg1" "arg2" "hello world")
```

### 4. Module System (`use`) & Namespaces
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

#### Double Loading Prevention & Caching
Lext hashes the absolute path of every evaluated module using a high-performance `MeowHash` key and caches the loaded sublet environment. Subsequent calls to `use` for the same module will map bindings from the cache without re-running the module's file on disk.

### 5. Running the Test Suite
Lext includes a heavy stress test suite to verify correct FFI behavior, memory boundaries, and standard library modules.
To run the stress test suite, run:
```bash
LEXT_HOME=. ./lext -s test/test_stress.lext
```
This runs all checks (including FFI struct/union by value passing, callbacks, and automatic bounds tracking/error catching).

---

## Standard Library Modules

`lext` ships with a set of modular libraries under the `stdlib` directory.

### 1. `basic` Module (`stdlib/basic`)
Contains core language extensions for pure Scheme. Loaded via `(use "stdlib/basic")`. Typically, you open its namespace globally using `(open-namespace "basic")`.

#### **Loop Constructs**
The module provides convenient imperative loops:
* **`while`**:
  ```scheme
  (while (< i 5)
    (display i)
    (set! i (+ i 1)))
  ```
* **`for`**:
  - Range syntax: `(for (var start end [step]) body ...)` (end is exclusive).
  ```scheme
  ;; Incrementing by 1: 0, 1, 2, 3, 4
  (for (i 0 5) (display i))

  ;; Stepping: 10, 8, 6, 4, 2
  (for (i 10 0 -2) (display i))
  ```
* **`foreach`**:
  - Iterates over elements of a list: `(foreach (var list) body ...)`
  ```scheme
  (foreach (x '(apple orange banana))
    (display x))
  ```

---

### 2. `c` Module (`stdlib/c`)
Contains low-level C FFI wrappers, libc allocations, and type definitions. Loaded via `(use "stdlib/c")`. Typically, you open its namespace globally using `(open-namespace "c")` after basic is opened.

#### **Memory Allocations & Helpers**
Provides direct bindings to C library memory routines, prefixed with `c.` (which become prefix-free when the namespace is opened):
* `(c.malloc size)` - Allocates raw C memory.
* `(c.free ptr)` - Frees allocated C memory.
* `(c.realloc ptr size)` - Reallocates C memory.
* `(c.calloc nmemb size)` - Allocates zero-initialized C memory.
* `(c.deref ptr type)` - Reads C memory value at a pointer.
* `(c.set! ptr type value)` - Writes value to C memory at a pointer.
* `(c.null-ptr)` / `(c.null-ptr? ptr)` - Direct null pointer checks.
* `(c.c-cast ptr type)` - Performs pointer type casting.
* `(c.size type)` - Returns the size in bytes of the FFI type layout.

> [!IMPORTANT]
> **Memory Safety Pattern**: Always use `(c.calloc 1 (c.size 'TypeName))` for struct allocation (never hardcoded sizes); use `c.free` to release the memory when done. Lext automatically tracks bounds for these allocations and raises Scheme-level exceptions on out-of-bounds accesses.

#### **Pointer Navigation & Mutating Macros**
These macros make traversing structured C memory structures incredibly concise:
* **`c.@`**: Dereference field path (e.g. `(c.@ obj.inner.val)`)
* **`c.=`**: Set/write field path value (e.g. `(c.= obj.inner.val 42)`)
* **`c.&`**: Retrieve memory address/pointer to field (e.g. `(c.& obj.inner.val)`)

#### **Globally Available FFI Declarators & Sandbox Allocators**
* **Type Declarations**: `(define-c-struct name fields...)`, `(define-c-union name fields...)`, `(define-c-enum name variants...)`, `(translate-ffi-type type)`
* **Sandbox Allocators**: `(with-heap-alloc (var type) body...)`, `(with-c-string (var str) body...)`, `(with-c-array (var type lst) body...)`, `(with-c-string-array (var lst) body...)`

---

### 3. Built-in Hash Table Support
The underlying `s7` engine features native, high-performance hash tables. You do not need external libraries for hash map operations. Built-in procedures include:
* `(make-hash-table [size])` - Creates a new hash table.
* `(hash-table-set! table key value)` - Associates key with value in table.
* `(hash-table-ref table key)` - Retrieves value associated with key.
* `(hash-table? obj)` - Returns `#t` if the object is a hash table.

---

## FFI API Reference

### Library & Symbol Management

#### `(ffi-open path)`
Loads a shared library (`.so`).
- **Arguments**: `path` (string) - Path to the shared library (e.g. `"libSDL2.so"` or `"./libtest.so"`). Pass `#f` to target the main process space (allowing access to standard C library functions like `puts` or `cos`).
- **Returns**: A pointer handle representing the library context.

#### `(ffi-sym handle symbol-name)`
Looks up a symbol in a loaded library.
- **Arguments**:
  - `handle`: Pointer returned from `ffi-open`.
  - `symbol-name` (string): The name of the function/global symbol (e.g. `"SDL_Init"`).
- **Returns**: A function pointer handle.

#### `(ffi-close handle)`
Closes a library handle.
- **Arguments**: `handle` - Pointer returned from `ffi-open`.

---

### Type System & Typedefs

#### `(ffi-typedef name type-desc)`
Registers a named type alias (such as a struct, union, or enum) in the FFI environment.
- **Arguments**:
  - `name` (symbol): Unique type name (e.g. `'Point`).
  - `type-desc` (list/symbol): The type layout descriptor.

#### Supported Type Descriptors
- **Primitive Types**:
  - `'void`: No return value.
  - `'int` / `'enum`: Standard signed integer.
  - `'double`: Double-precision float.
  - `'float`: Single-precision float.
  - `'string`: Maps a Scheme string to `char *` and automatically handles C `char *` to Scheme string conversion on return.
  - `'pointer`: A generic void pointer (`void *`).
  - **Exact-width Integers & Characters**:
    - `'char`, `'schar`, `'uchar`: 8-bit signed/unsigned char types.
    - `'int8`, `'uint8`: 8-bit signed/unsigned integer.
    - `'short`, `'ushort`, `'int16`, `'uint16`: 16-bit signed/unsigned integer.
    - `'int32`, `'uint32`: 32-bit signed/unsigned integer.
    - `'long`, `'ulong`: Machine-width signed/unsigned long.
    - `'int64`, `'uint64`: 64-bit signed/unsigned integer.
- **Compound Types**:
  - **Structs**:
    - Flat: `'(struct type1 type2 ...)`
    - Labeled: `'(struct (type1 label1) (type2 label2) ...)`
  - **Unions**:
    - Flat: `'(union type1 type2 ...)`
    - Labeled: `'(union (type1 label1) (type2 label2) ...)`
  - **Arrays**: `'(array type size)`
    - Represents a contiguous array of `size` elements of type `type`. Translated internally to a compatible struct layout.

---

### Pointer Dereferencing & Modification

#### `(ffi-deref ptr type-desc)`
Dereferences a C pointer handle and decodes the memory according to the specified type layout.
- **Arguments**:
  - `ptr`: C pointer handle (returned by FFI calls).
  - `type-desc` (symbol/list): The type layout descriptor.
- **Returns**: Decoded Scheme representation of the memory.

#### `(ffi-set! ptr type-desc value)`
Serializes and writes a Scheme value to the memory address pointed to by `ptr`.
- **Arguments**:
  - `ptr`: C pointer handle.
  - `type-desc` (symbol/list): The type layout descriptor.
  - `value`: Scheme representation (e.g., flat list or association list for structs).

---

### Data Mapping Details

#### Struct Mapping
- **Flat Structs**:
  - **To C**: Pass a flat list of values in order: `(val1 val2 ...)`.
    *Example*: For `'(struct int int)`, pass `(10 20)`.
  - **From C**: Returns a flat list representing the struct fields: `(val1 val2 ...)`.
- **Labeled Structs**:
  - **To C**: Pass either a flat list `(val1 val2 ...)` or an association list `((label1 . val1) (label2 . val2) ...)`.
  - **From C**: Returns an association list mapping labels to decoded values: `((label1 . val1) (label2 . val2) ...)`.

#### Union Mapping
- **Flat Unions**:
  - **To C**: Pass a list containing the 0-indexed position of the active union field and its value: `(active_field_index active_field_val)`.
    *Example*: Given `(ffi-typedef 'IntOrDouble '(union int double))`, to pass the double `3.14` (index 1), pass `'(1 3.14)`.
  - **From C**: Returns a list containing the union memory decoded *separately* as every possible member type: `(as_type1 as_type2 ...)`.
    *Example*: Reading an `IntOrDouble` union containing `3.14` returns a list like `(1074339512 3.14)`.
- **Labeled Unions**:
  - **To C**: Pass either `(active_field_index active_field_val)` or `(active_field_label active_field_val)`.
  - **From C**: Returns a labeled association list showing all slots: `((label1 . as_type1) (label2 . as_type2) ...)`.

#### Array Mapping
- **To C**: Pass a flat list of items `(item1 item2 ...)` or a vector `#(item1 item2 ...)`.
- **From C**: Returns a flat list of items: `(item1 item2 ...)`.


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

## Editor Support

### Emacs
`lext` includes an Emacs major mode, `lext-mode`, derived from `scheme-mode`. It adds syntax highlighting for Lext-specific constructs:
* Core keywords (`use`, `c-import`, `define-c-struct`, etc.)
* FFI primitives (`ffi-open`, `ffi-sym`, etc.)
* Memory helpers and namespaces (`c.malloc`, `open-namespace`, etc.)
* Task runner API (`nob.*`)

To use it, load the `lext-mode.el` file in your `.emacs` or `init.el` configuration:
```elisp
(load-file "/path/to/lext/lext-mode.el")
```
It automatically associates `.lext` files with `lext-mode`.

