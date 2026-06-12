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

You can also let `lext` built itself with `nob.scm`, you just need `lext` already compiled and do:
```bash
# compile the shared lib of libnob first so we can FFI to nob.h
gcc -O2 -fPIC -shared nob.c -o libnob.so
lext -s nob.scm
```

it will generate a `build` dir with all the artifact and the executable there.

---

## Usage Guide

### 1. Pure Scheme Script Runner Mode (`-s` / `--no-template`)
To run a raw Scheme file directly without template parsing, use the `-s` or `--no-template` flag:
```bash
./lext -s script.scm
```
*Note: In script runner mode, specifying an output file is not allowed and will raise an error.*

### 2. Template Preprocessor Mode
To evaluate embedded Lisp/Scheme code blocks (`@@(...)`) inside template files (e.g. LaTeX, HTML, text):
```bash
./lext input.texm output.tex
```

#### Raw String Literals in Templates
To simplify writing strings containing backslashes (like LaTeX commands) or double quotes without verbose manual escaping, you can use raw strings inside evaluation blocks:
* **Syntax**: `#|r ... |#`
* Within `#|r` and `|#`, all characters are captured raw.
* Double quotes `"` are automatically escaped to `\"` and backslashes `\` to `\\` in the generated Scheme string literal.
* **Example**:
  ```latex
  @@(string-append "Rendered LaTeX command: " #|r\texttt{foo} and "bar"|#)
  ```
  This evaluates to:
  ```latex
  Rendered LaTeX command: \texttt{foo} and "bar"
  ```


### 3. Command-Line Arguments Passing
You can pass custom command-line arguments to your Scheme scripts by adding `--` after the script name. Any arguments following `--` are collected into a list of strings and bound to the global variables `argv` and `*argv*` in the Scheme interpreter:
```bash
./lext -s script.scm -- arg1 arg2 "hello world"
```

Inside your script:
```scheme
(display argv)
;; Output: ("arg1" "arg2" "hello world")
```

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

(define libc (ffi-open #f))
(c-import malloc libc "malloc" pointer (int))
(c-import free   libc "free"   void    (pointer))

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

(define event-ptr (malloc 56)) ; max event size
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


### Helper: Binding C Functions to Scheme Functions (`c-import`)

Instead of invoking `ffi-call` with raw function pointer variables and verbose argument lists every time, you can use a Scheme macro to bind any C function to a clean Scheme function wrapper.

Here is the helper macro:
```scheme
(define-macro (c-import scheme-name lib-handle c-name ret-type arg-types . nfixed)
  (let ((func-ptr (gensym)))
    `(begin
       ;; Resolve symbol once at definition time
       (define ,func-ptr (ffi-sym ,lib-handle ,c-name))
       ;; Create the Scheme wrapper function
       (define (,scheme-name . args)
         (ffi-call ,func-ptr ',ret-type ',arg-types args ,@(if (null? nfixed) '() (list (car nfixed))))))))
```

#### Usage Example:
```scheme
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
