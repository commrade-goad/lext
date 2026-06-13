# Lext-Scheme: The Complete Language & FFI Reference Guide

Welcome to the definitive reference manual for **Lext** (Lext-Scheme), a systems-scripting Lisp dialect optimized for metaprogramming, C FFI binding, task orchestration, and template generation. 

This document provides an exhaustive overview of the syntax, macros, standard libraries, and implementation details of the Lext dialect.

---

## Table of Contents
1. [Core Design & s7 Engine Integration](#1-core-design--s7-engine-integration)
2. [File Extensions & Loading standard `.scm` files](#2-file-extensions--loading-standard-scm-files)
3. [The Module System (`use`) & Search Paths](#3-the-module-system-use--search-paths)
4. [Namespace Management (`open-namespace` & `use-namespace`)](#4-namespace-management-open-namespace--use-namespace)
5. [Syntax Extensions: Imperative Loops](#5-syntax-extensions-imperative-loops)
6. [Type Declarations (`struct`, `union`, `enum`)](#6-type-declarations-struct-union-enum)
7. [Pointer Arithmetic & Memory Navigation (`c.@`, `c.=`, `c.&`)](#7-pointer-arithmetic--memory-navigation-c-c-c)
8. [Memory Sandbox Allocators (`with-heap-alloc`, `with-c-string`, etc.)](#8-memory-sandbox-allocators-with-heap-alloc-with-c-string-etc)
9. [Tracked Bounds-Checked Memory Allocations](#9-tracked-bounds-checked-memory-allocations)
10. [String Views (`SV`) & Temp Allocators](#10-string-views-sv--temp-allocators)
11. [Dynamic Arrays (`da`) API](#11-dynamic-arrays-da-api)
12. [Task Orchestration & Shell Execution (`nob` command runner)](#12-task-orchestration--shell-execution-nob-command-runner)
13. [Headless GUI Programming with SDL2](#13-headless-gui-programming-with-sdl2)
14. [Template Engine & Raw String Literals](#14-template-engine--raw-string-literals)
15. [Emacs Major Mode Integration](#15-emacs-major-mode-integration)

---

## 1. Core Design & s7 Engine Integration

Lext is built on the **s7 Lisp** engine. s7 is a minimal Scheme dialect belonging to the Lisp family, sharing elements of R5RS/R7RS Scheme and Common Lisp. 

Key built-in s7 features available globally in Lext include:
* **Native Environments (First-class Let Scopes)**: Environments can be captured, passed around as values, and inspected via `(curlet)` (current environment) and `(rootlet)` (global environment).
* **First-Class Macros**: Lext supports Standard Lisp macros using `(define-macro (name args) ...)` which evaluate dynamically at runtime when encountered.
* **Native Hash Tables**: Created using `(make-hash-table)`. Association and lookup are done using `(hash-table-set! table key val)` and `(hash-table-ref table key)`.

---

## 2. File Extensions & Loading standard `.scm` files

* The official extension for Lext scripts and libraries is **`.lext`**.
* **Coexistence with `.scm`**: The `lext` binary does not restrict the loading of legacy files. Standard Scheme files (ending in `.scm`) can be loaded normally using the standard Scheme `load` function:
  ```scheme
  ;; Load standard s7 libraries or legacy files normally
  (load "src/s7/stuff.scm")
  ```
* Manually loading standard Scheme helpers (like `stuff.scm` or `r7rs.scm`) is the recommended way to acquire additional Scheme libraries (such as `pretty-print`, `any?`, or SRFI-1 features) that are omitted from the core binary to keep startup time extremely fast (~0ms).

---

## 3. The Module System (`use`) & Search Paths

The `use` function dynamically locates and loads module entry-point libraries. It uses the `LEXT_HOME` environment variable (paths delimited by colon `:`) as search roots.

* When you call `(use "stdlib/basic")`, Lext searches each path in `LEXT_HOME` for a directory named `stdlib/basic` containing a file named `lib.lext`.
* **Double Loading Prevention**: Lext prevents redundant disk reads and variable re-evaluations. The interpreter hashes the absolute path of every evaluated module file using a high-performance **MeowHash** key and caches it in a global tracking table.

### Example:
```scheme
;; Loading multiple standard libraries in one call
(use "stdlib/basic" "stdlib/c" "stdlib/libnob")
```

---

## 4. Namespace Management (`open-namespace` & `use-namespace`)

To prevent imported low-level FFI bindings (such as `c.malloc` or the memory comparison operator `c.=`) from polluting the global environment or overriding core Scheme operators, Lext wraps module interfaces inside prefixes. You can import them using namespace macros:

### A. Global Importing (`open-namespace`)
Strips the namespace prefix and registers the bindings in the current global environment. It automatically appends the trailing dot (`.`) separator if omitted. It protects critical operators (like `=`, `set!`, `+`, `-`, etc.) from being overwritten.
```scheme
(use "stdlib/c")
(open-namespace "c") ;; strips "c." prefix

;; Now you can invoke malloc and free directly without the prefix:
(define ptr (malloc 32))
(free ptr)
```

### B. Lexically Scoped Importing (`use-namespace`)
Temporarily strips the namespace prefix within the macro body. Outside of this block, the prefix-free symbols remain unbound.
```scheme
(use "stdlib/libnob")

(use-namespace "nob"
  (display "Inside namespace block:\n")
  (cmd-run '("echo" "Hello from scoped namespace!")))

;; Outside the block, 'cmd-run' is unbound, avoiding namespace pollution.
```

---

## 5. Syntax Extensions: Imperative Loops

Standard Scheme relies on recursive helper functions or complex `do` loops for iteration. Lext provides three standard, prefix-prefixed macros (`bc.while`, `bc.for`, `bc.foreach`) which become prefix-free when you open the basic namespace `(open-namespace "bc")`:

### A. Loop condition: `while`
Evaluates the body repeatedly while the condition evaluates to `#t`.
```scheme
(let ((i 0))
  (while (< i 5)
    (format #t "i = ~A\n" i)
    (set! i (+ i 1))))
```

### B. Index Range Loop: `for`
* **Syntax**: `(for (variable start end [step]) body ...)` (the `end` bound is exclusive).
* The step size can be positive or negative. Lext automatically adjusts the termination check.
```scheme
;; 1. Simple increment (step defaults to +1): Prints 0 1 2 3 4
(for (i 0 5)
  (display i))

;; 2. Incrementing with custom step: Prints 0 2 4 6 8
(for (i 0 10 2)
  (display i))

;; 3. Decrementing step: Prints 10 7 4 1
(for (i 10 0 -3)
  (display i))
```

### C. Iterable list Loop: `foreach`
Iterates over each element in a Scheme list.
```scheme
(foreach (word '("hello" "world" "from" "lext"))
  (format #t "~A " word))
```

---

## 6. Type Declarations (`struct`, `union`, `enum`)

Lext includes declarative macros to describe C memory layouts to the FFI compiler. 

### A. Declarators
* **`define-c-struct`**: Builds a memory-aligned structure layout.
* **`define-c-union`**: Builds a overlapping union layout.
* **`define-c-enum`**: Binds a sequential enumeration to numeric constants in the environment.

```scheme
(use "stdlib/c")
(open-namespace "c")

;; Declares a aligned struct layout
(define-c-struct Point
  (x int)
  (y int))

;; Declares a union layout (sharing same memory slots)
(define-c-union IntOrDouble
  (i int)
  (d double))

;; Declares an enumeration. Variants can have implicit sequential values or explicit values.
(define-c-enum State
  :state-pending       ;; 0
  :state-active        ;; 1
  (state-error -100)   ;; -100
  :state-terminated)   ;; -99
```

### B. ABI-Compliant Alignment & Caching
When types are registered via `ffi-typedef`, Lext:
1. Translates the nested type fields (resolving pointers, arrays, and sub-structs).
2. Computes the required memory padding and offsets to comply with the host's Application Binary Interface (ABI) requirements.
3. Hashes the parsed type structure with **MeowHash** and caches it inside the FFI engine. Subsequent function calls passing these structures by value are optimized using the pre-cached ABI layout (avoiding parsing the structure representation on every function call).

### C. Scheme Data Mapping Details

| C Type | Scheme Layout (To C) | Scheme Layout (From C) | Example |
| :--- | :--- | :--- | :--- |
| **Flat Struct** | List: `(val1 val2)` | List: `(val1 val2)` | `(10 20)` |
| **Labeled Struct** | Assoc List: `((label . val)...)` | Assoc List: `((label . val)...)` | `((x . 10) (y . 20))` |
| **Flat Union** | List: `(active-index val)` | List of all decoded variants | `'(1 3.14)` -> `'(1074339512 3.14)` |
| **Labeled Union** | Assoc List with single active key | Assoc List of all slot representations | `((d . 3.14))` |
| **Array** | List or Vector: `'(10 20 30)` or `#(10 20 30)` | List: `(10 20 30)` | `(array int 3)` |

---

## 7. Pointer Arithmetic & Memory Navigation (`c.@`, `c.=`, `c.&`)

Lext provides three core macros to make reading, writing, and navigating C memory pointers safe and concise. They automatically resolve structure offsets and alignment padding at runtime.

* **`c.@` (Deref Path)**: Retrieves a value at a path from a pointer.
* **`c.=` (Write Path)**: Writes a value to a path from a pointer.
* **`c.&` (Address Of)**: Returns a typed pointer containing the address of a specific sub-field.

### Navigation Rules
Paths can traverse structs (via symbol fields) and arrays (via integer indexes):

```scheme
;; Assume Point is a struct containing { int x; int y; }
;; Assume MyData is a struct containing { Point points[3]; char *label; }
(with-heap-alloc (data MyData)
  ;; Write to an array member's sub-field
  (c.= data.points.1.x 100)
  (c.= data.points.1.y 200)
  
  ;; Write to a string pointer
  (with-c-string (s "Test Label")
    (c.= data.label s))

  ;; Read values
  (format #t "Label: ~A\n" (c-string-from-ptr (c.@ data.label)))
  (format #t "Point [1]: (~A, ~A)\n" (c.@ data.points.1.x) (c.@ data.points.1.y))

  ;; Get direct pointer to data.points[1].y
  (let ((y-ptr (c.& data.points.1.y)))
    ;; y-ptr is a typed pointer pointing directly to the memory address of y
    (format #t "Value of y via direct pointer dereference: ~A\n" (c.deref y-ptr 'int))))
```

---

## 8. Memory Sandbox Allocators (`with-heap-alloc`, `with-c-string`, etc.)

Memory allocated manually via `c.malloc` must be freed with `c.free`. To prevent memory leaks, Lext provides scoped allocation macros that guarantee cleanup using `dynamic-wind` (even if an error occurs or a non-local escape exit is triggered).

### A. `with-heap-alloc` (Struct Allocator)
Allocates heap memory for a registered type descriptor.
```scheme
;; Allocate a Point struct on the heap
(with-heap-alloc (p Point)
  (c.= p.x 10)
  (c.= p.y 20)
  (format #t "p: ~A, ~A\n" (c.@ p.x) (c.@ p.y))) 
;; 'p' memory is automatically freed here!
```

### B. `with-c-string` (C String Marshaller)
Allocates a temporary null-terminated C string (`char *`) containing the characters of a Scheme string.
```scheme
(with-c-string (c-str "Lext Scheme")
  ;; c-str is a char* pointer
  (c-puts c-str))
;; Memory freed automatically here!
```

### C. `with-c-array` (Primitive Array Marshaller)
Allocates a contiguous C memory buffer populated with values from a Scheme list.
```scheme
(with-c-array (arr int '(100 200 300 400))
  (format #t "Index 2: ~A\n" (c.@ arr.2)))
;; Memory freed automatically here!
```

### D. `with-c-string-array` (String Pointer Array Marshaller)
Allocates a null-terminated array of string pointers (`char **`), ideal for calling C functions like `execvp`.
```scheme
(with-c-string-array (argv '("ls" "-la" "/tmp"))
  ;; argv is a char** pointer. Individual string buffers are also allocated and freed.
  (format #t "Command: ~A\n" (c-string-from-ptr (c.@ argv.0))))
;; All buffers freed automatically here!
```

---

## 9. Tracked Bounds-Checked Memory Allocations

For high-security operations or debugging memory safety issues, Lext contains a built-in bounds-checking system:

* **`(c.malloc-tracked size)`**: Allocates memory and registers its address and size in an internal boundary tracking map.
* **`(c.free-tracked ptr)`**: Removes the tracking entry and frees the memory.
* **`(c.bounds-check ptr size)`**: Verifies that a pointer read or write operation falls entirely within the allocated boundaries. If a boundary violation is detected, the interpreter raises an error immediately, preventing out-of-bounds corruption.

---

## 10. String Views (`SV`) & Temp Allocators

For high-performance string splitting and slicing without generating excessive heap allocations, Lext includes a **String View** library (similar to C++'s `std::string_view`):

### A. String View (`SV`) functions:
* **`(nob.sv-from-cstr string)`**: Creates a string view from a C string.
* **`(nob.sv-trim sv)`**: Returns a new view stripping leading and trailing whitespace.
* **`(nob.sv-chop-left sv bytes)`**: Chops characters from the left, returning the chopped view.
* **`(nob.sv-chop-by-delim sv delimiter-char)`**: Slices the view at a delimiter, updating the view pointer and returning the sliced piece.
* **`(nob.sv->string sv)`**: Congeals the view back into a native Scheme string.

```scheme
(let* ((sv (nob.sv-from-cstr "  key:value  "))
       (trimmed (nob.sv-trim sv)))
  (let ((key-view (nob.sv-chop-by-delim (c.& trimmed) #\:)))
    (format #t "Key: ~S\n" (nob.sv->string key-view))     ;; Prints: Key: "key"
    (format #t "Value: ~S\n" (nob.sv->string trimmed))))  ;; Prints: Value: "value"
```

### B. Temporary Stack Allocator
Lext implements a high-performance **Temp Arena Allocator** for scripts that need rapid allocation of transient string copies:
* `(nob.temp-strdup c-string)`: Duplicates a C string inside the arena.
* `(nob.temp-alloc size)`: Allocates raw memory in the arena.
* `(nob.temp-reset)`: Clears the arena in constant time ($O(1)$).

---

## 11. Dynamic Arrays (`da`) API

Lext standard libraries represent dynamic arrays as standard C structures:
```c
typedef struct {
    void *items;
    size_t count;
    size_t capacity;
} DynamicArray;
```

You can manipulate these arrays directly in Scheme scripts using the `nob.da` API:
* **`(nob.da-items da)`**: Returns the items pointer.
* **`(nob.da-count da)`**: Returns the active element count.
* **`(nob.da-capacity da)`**: Returns the total pre-allocated capacity.
* **`(nob.da-append da value type)`**: Appends a value (types: `'pointer`, `'ulong`, `'int`, `'uchar`) and automatically doubles the capacity if needed.
* **`(nob.da-pop da type)`**: Pops the last item from the array.
* **`(nob.da-first da type)`** / **`(nob.da-last da type)`**: Inspects array boundaries.
* **`(nob.da-free da)`**: Releases the array items buffer.

---

## 12. Task Orchestration & Shell Execution (`nob` command runner)

Lext contains a complete process execution suite, making it a powerful replacement for shell scripts and makefiles.

### A. Core Commands execution
* **`nob.cmd-new`**: Instantiates a new command command-line builder.
* **`nob.cmd-append cmd arg1 arg2 ...`**: Appends arguments.
* **`nob.cmd-run cmd [options]`**: Executes command synchronously.
* **`nob.cmd-run-async cmd [options]`**: Spawns command in background and returns process ID.
* **`nob.cmd-free cmd`**: Releases command memory.

```scheme
(let ((cmd (nob.cmd-new)))
  (nob.cmd-append cmd "gcc" "-O3" "src/main.c" "-o" "build/lext")
  (if (nob.cmd-run cmd)
      (display "Compilation succeeded!\n")
      (error "Compilation failed"))
  (nob.cmd-free cmd))
```

### B. Piped Shell execution (`nob.chain` & `nob.pipe-create`)
Lext supports piped command streams without invoking a system shell wrapper:
```scheme
(let ((pipe (c.malloc 8)))
  (nob.pipe-create pipe)
  ;; Redirect command outputs directly into another command's input:
  (let ((c-echo (nob.cmd-new))
        (c-grep (nob.cmd-new)))
    (nob.cmd-append c-echo "echo" "Lext scripting language")
    (nob.cmd-append c-grep "grep" "Lext")
    
    ;; Piped execution
    (nob.cmd-run c-echo :stdout-fd (c.@ pipe.write))
    (nob.cmd-close-fd (c.@ pipe.write))
    (nob.cmd-run c-grep :stdin-fd (c.@ pipe.read))
    
    (nob.cmd-free c-echo)
    (nob.cmd-free c-grep)
    (c.free pipe)))
```

---

## 13. Headless GUI Programming with SDL2

Lext's dynamic structures allow running high-performance graphics engines like **SDL2** by wrapping struct pointer events and invoking C functions directly.

Below is an abbreviated polling cycle:
```scheme
(use "stdlib/c")
(open-namespace "c")

(define sdl (ffi-open "libSDL2.so"))

;; Import SDL2 procedures
(c-import sdl-init                 sdl "SDL_Init"               int     (int))
(c-import sdl-create-window        sdl "SDL_CreateWindow"       pointer (string int int int int int))
(c-import sdl-pollevent            sdl "SDL_PollEvent"          int     (pointer))
(c-import sdl-destroy-window       sdl "SDL_DestroyWindow"      void    (pointer))
(c-import sdl-quit                 sdl "SDL_Quit"               void    ())

;; Register SDL Keyboard Event Structure for FFI
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

(sdl-init 32) ;; SDL_INIT_VIDEO
(define win (sdl-create-window "Lext SDL Window" 0 0 640 480 4))

(let ((event-ptr (malloc 56))) ;; Event buffer
  (let loop ()
    (if (> (sdl-pollevent event-ptr) 0)
        (let ((type (ffi-deref event-ptr 'int)))
          (cond
            ((= type #x100) (display "Quit Event triggered!\n")) ;; SDL_QUIT
            ((= type #x300)                                     ;; SDL_KEYDOWN
             (let ((key (ffi-deref event-ptr 'SDL_KeyboardEvent)))
               (format #t "Key Pressed: scancode=~A, sym=~A\n" 
                       (cdr (assoc 'scancode key)) 
                       (cdr (assoc 'sym key)))
               (loop)))
            (else (loop))))))
  (free event-ptr))

(sdl-destroy-window win)
(sdl-quit)
```

---

## 14. Template Engine & Raw String Literals

When run in templating mode (without the `-s` script flag), Lext parses files, looking for `@@(...)` expressions.

1. **Standard Evaluation**:
   `@@(string-append "Current Platform: " *platform*)` evaluates to `Current Platform: linux`.
2. **Raw String Literals (`r"..."`)**:
   Standard Scheme strings interpret backslashes as escape sequences. This makes outputting formats like LaTeX commands tedious. Raw string literals solve this:
   - Inside `r"..."`, backslashes `\` are processed literally.
   - The template parser converts them into double-escaped Scheme string literals before evaluation.
   ```latex
   @@(string-append "LaTeX command: " r"\textbf{Section \ref{sec:first}}")
   ```
   *Renders as:* `LaTeX command: \textbf{Section \ref{sec:first}}`

---

## 15. Emacs Major Mode Integration

The major mode **`lext-mode`** (located in `lext-mode.el`) provides a complete development environment for Emacs:

1. **Custom Syntax Highlighting**: Highlight FFI primitives (`ffi-open`, `ffi-sym`), memory macros (`c.@`, `with-heap-alloc`), loop extensions (`while`, `for`, `foreach`), and custom dynamic types (`Nob_Cmd`, etc.).
2. **Built-in Rainbow Delimiters**: A lightweight, fast syntax paren-depth counter is executed on the buffer, coloring parentheses dynamically based on their nesting level to aid readability.
3. **Indentation Rules**: Custom Lisp indentation rules for Lext macros:
   ```elisp
   (put 'with-heap-alloc 'lext-indent-function 1)
   (put 'use-namespace 'lext-indent-function 1)
   ```
4. **Auto-Association**: Configured to load automatically whenever a file ending in `.lext` is opened.
   ```elisp
   (add-to-list 'auto-mode-alist '("\\.lext\\'" . lext-mode))
   ```
