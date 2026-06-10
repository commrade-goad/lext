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
  - Auto-escape backslashes with `-e` / `--escape`.

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
*Options:*
- `-e` / `--escape`: Enables automatic escaping of backslashes inside string literals in `@@(...)` blocks.

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
  - `'int`: Signed integer.
  - `'enum`: Signed integer (treated identically to `'int`).
  - `'double`: Double-precision float.
  - `'float`: Single-precision float.
  - `'string`: Maps a Scheme string to `char *` and automatically handles C `char *` to Scheme string conversion on return.
  - `'pointer`: A generic void pointer (`void *`).
- **Compound Types**:
  - **Structs**: `'(struct type1 type2 ...)`
  - **Unions**: `'(union type1 type2 ...)`

---

### Calling Functions

#### `(ffi-call func ret-type arg-types arg-vals [nfixed])`
Invokes a resolved function pointer.
- **Arguments**:
  - `func`: Function pointer handle returned by `ffi-sym`.
  - `ret-type` (symbol/list): The return type descriptor.
  - `arg-types` (list of symbols/lists): Descriptors for the arguments. For variadic calls, you only need to specify the types of the fixed arguments; the types of any trailing variadic arguments will be automatically inferred from their Scheme values.
  - `arg-vals` (list of values): Scheme arguments.
  - `nfixed` (integer, optional): The number of fixed arguments. If specified and greater than 0, the function is called as a variadic function (using `ffi_prep_cif_var` internally). If `nfixed` is specified, `arg-types` only needs to list the types for the fixed arguments; the remaining variadic arguments are automatically inferred (`integer`/`boolean`/`character` -> `'int`, `real` -> `'double`, `string` -> `'string`, `c-pointer`/`nil` -> `'pointer`).

---

### Data Mapping Details

#### Struct Mapping
- **To C**: Pass a flat list of values in order: `(val1 val2 ...)`.
  *Example*: For `'(struct int int)`, pass `(10 20)`.
- **From C**: Returns a flat list representing the struct fields: `(val1 val2 ...)`.

#### Union Mapping
- **To C**: Pass a list containing the 0-indexed position of the active union field and its value: `(active_field_index active_field_val)`.
  *Example*: Given `(ffi-typedef 'IntOrDouble '(union int double))`, to pass the double `3.14` (index 1), pass `'(1 3.14)`.
- **From C**: Returns a list containing the union memory decoded *separately* as every possible member type: `(as_type1 as_type2 ...)`.
  *Example*: Reading an `IntOrDouble` union containing `3.14` returns a list like `(1074339512 3.14)`.

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
