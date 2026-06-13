# Lext-Scheme: The Complete Language & FFI Reference Guide

Welcome to the definitive reference manual for **Lext** (Lext-Scheme), a systems-scripting Lisp dialect optimized for metaprogramming, C FFI binding, and template generation.

This document provides a detailed overview of the core runtime engine, the C-side built-in procedures, basic libraries, and how the language interacts with standard Scheme files.

---

## Table of Contents
1. [Core Design & s7 Engine Integration](#1-core-design--s7-engine-integration)
2. [File Extensions & Loading standard `.scm` files](#2-file-extensions--loading-standard-scm-files)
3. [The Module System (`use`) & Search Paths](#3-the-module-system-use--search-paths)
4. [C-Side Core Built-in Reference](#4-c-side-core-built-in-reference)
5. [Standard Library: `basic` Module (`stdlib/basic/lib.lext`)](#5-standard-library-basic-module-stdlibbasicliblext)
6. [Standard Library: `c` Module (`stdlib/c/lib.lext`)](#6-standard-library-c-module-stdlibcliblext)
7. [The Task Runner Library: `libnob` Reference](#7-the-task-runner-library-libnob-reference)
8. [Template Engine & Raw String Literals](#8-template-engine--raw-string-literals)
9. [Emacs Major Mode Integration](#9-emacs-major-mode-integration)

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

## 4. C-Side Core Built-in Reference

The `lext` interpreter registers several low-level primitives in the global environment at startup. These are defined directly in the C source files.

### A. Library & Symbol Resolution (`src/lext_ffi.c`)

* **`(ffi-open path)`**  
  Loads a shared library (`.so`).
  - **Arguments**: `path` (string) - Path to the shared library (e.g. `"libSDL2.so"`). Pass `#f` to target the main process space (allowing access to standard C library functions like `puts` or `cos`).
  - **Returns**: A pointer handle representing the library context.

* **`(ffi-open-lib path)`**  
  An alias for `ffi-open`.

* **`(ffi-sym handle symbol-name)`**  
  Looks up a symbol in a loaded library.
  - **Arguments**:
    - `handle`: Pointer returned from `ffi-open`.
    - `symbol-name` (string): The name of the function/global symbol (e.g. `"SDL_Init"`).
  - **Returns**: A function pointer handle.

* **`(ffi-close handle)`**  
  Closes a library handle.
  - **Arguments**: `handle` - Pointer returned from `ffi-open`.

* **`(ffi-call func-ptr ret-type arg-types arg-vals [nfixed])`**  
  Invokes a C function pointer using `libffi`.
  - **Arguments**:
    - `func-ptr`: Function pointer from `ffi-sym`.
    - `ret-type` (symbol): Return type descriptor (e.g., `'int`, `'void`, `'double`).
    - `arg-types` (list of symbols): Types of the arguments.
    - `arg-vals` (list of values): Scheme arguments matching `arg-types`.
    - `nfixed` (integer, optional): The number of fixed arguments if invoking a variadic C function (e.g., `printf`).
  - **Returns**: The return value of the C function translated back to Scheme.

* **`(ffi-callback scheme-func ret-type arg-types)`**  
  Creates a C-compatible function pointer (closure) that executes a Scheme function when called by C.
  - **Arguments**:
    - `scheme-func`: Scheme procedure.
    - `ret-type` (symbol): Return type descriptor.
    - `arg-types` (list of symbols): Argument types.
  - **Returns**: A C pointer that can be passed as a callback parameter to C functions.

### B. Type Information & Serialization (`src/lext_ffi.c`)

* **`(ffi-typedef name type-desc)`** (Registered in `src/main.c`)  
  Registers a named type layout alias (such as a struct, union, or array) in the FFI layout cache.
  - **Arguments**:
    - `name` (symbol): Unique type name (e.g. `'Point`).
    - `type-desc` (list/symbol): Type layout descriptor (e.g., `'(struct int int)`).

* **`(ffi-deref ptr type-desc)`**  
  Dereferences a C pointer and decodes the memory according to the specified type layout.
  - **Arguments**:
    - `ptr`: C pointer handle.
    - `type-desc` (symbol/list): The type layout descriptor.
  - **Returns**: The decoded Scheme representation of the memory.

* **`(ffi-set! ptr type-desc value)`**  
  Serializes and writes a Scheme value to the memory address pointed to by `ptr`.
  - **Arguments**:
    - `ptr`: C pointer handle.
    - `type-desc` (symbol/list): The type layout descriptor.
    - `value`: Scheme representation.

* **`(ffi-size type)`**  
  Returns the size in bytes of the FFI type.

* **`(ffi-align type)`**  
  Returns the byte alignment required by the FFI type on the host machine.

* **`(c-pointer->integer ptr)`**  
  Casts a raw C pointer handle to an integer memory address.

* **`(integer->c-pointer addr)`**  
  Casts an integer memory address to a raw C pointer.

### C. Tracked Allocations (`src/lext_ffi.c`)

* **`(lext-malloc-tracked size)`**  
  Allocates a block of heap memory and registers its boundary in the interpreter's safety map.

* **`(lext-free-tracked ptr)`**  
  Frees a tracked pointer and unregisters its address.

* **`(lext-bounds-check ptr size)`**  
  Verifies that access to `ptr` for `size` bytes is within its registered allocation boundaries. Raises a runtime error on violation.

### D. Native String & Memory Primitives (`src/lext_builtins.c`)

* **`(lext-calloc nmemb size)`**  
  Calls calloc to allocate zero-initialized heap memory.

* **`(lext-string->c-string str)`**  
  Allocates a null-terminated heap buffer populated with the characters of Scheme string `str`.

* **`(lext-c-string-from-ptr ptr)`**  
  Reads a null-terminated C string (`char*`) and returns a Scheme string.

* **`(lext-c-string-array->list ptr)`**  
  Reads a null-terminated array of string pointers (`char**`) and returns a Scheme list of strings.

* **`(lext-sv->string count data-ptr)`**  
  Converts a length-bound string buffer slice into a Scheme string.

### E. Capture & Directory Primitives

* **`(lext-capture-output thunk)`** (Registered in `src/lext_capture.c`)  
  Evaluates `thunk` (a zero-argument procedure) and captures anything it prints to `stdout`, returning it as a Scheme string.

* **`(lext-walk-dir root proc level)`** (Registered in `src/lext_da.c`)  
  Recursively walks the file directory starting at `root`. For each file or directory found, it invokes `(proc path type level)`.

* **`(lext-da-reserve da-ptr expected-capacity element-size)`** (Registered in `src/lext_da.c`)  
  Ensures that the dynamic array at `da-ptr` has enough capacity to hold `expected-capacity` items, doubling the allocation size if needed.

* **`(lext-da-append-raw da-ptr element-size val-ptr)`** (Registered in `src/lext_da.c`)  
  Appends raw bytes from `val-ptr` to the dynamic array.

---

## 5. Standard Library: `basic` Module (`stdlib/basic/lib.lext`)

This module extends the core Scheme syntax with standard control blocks and namespace managers.

### A. Loop Constructs (Exposed under `bc.` prefix)

* **`bc.while`** (macro)  
  Loops while a condition is true.
  ```scheme
  (let ((i 0))
    (bc.while (< i 3)
      (display i)
      (set! i (+ i 1))))
  ```

* **`bc.for`** (macro)  
  Index range loop. Syntax: `(for (var start end [step]) body ...)`. The `end` boundary is exclusive.
  ```scheme
  ;; Prints: 0 1 2
  (bc.for (i 0 3) (display i))

  ;; Prints: 5 3 1
  (bc.for (i 5 0 -2) (display i))
  ```

* **`bc.foreach`** (macro)  
  Iterates over each item of a Scheme list.
  ```scheme
  (bc.foreach (x '(apple orange banana))
    (display x))
  ```

### B. Namespace Strippers

* **`(open-namespace prefix-arg)`**  
  Globally registers prefix-free copies of all variables matching `prefix-arg` (e.g. `"bc"`, `"c"`), while protecting core compiler symbols.
  ```scheme
  (open-namespace "bc") ;; strips "bc." prefix
  ;; Now you can use while, for, foreach directly!
  ```

* **`(use-namespace prefix-arg . body)`**  
  Temporarily strips prefixes within the lexical scope of `body`.
  ```scheme
  (use-namespace "bc"
    (for (i 0 3) (display i)))
  ```

---

## 6. Standard Library: `c` Module (`stdlib/c/lib.lext`)

This module provides low-level C memory allocations, type builders, and pointer utilities.

### A. Memory Allocations & Helpers

* **`(c.malloc size)`** / **`(c.free ptr)`** / **`(c.realloc ptr size)`** / **`(c.calloc nmemb size)`**  
  Direct interfaces to standard C memory allocators.

* **`(c.deref ptr type . path)`**  
  Dereferences a pointer at a specific field path.

* **`(c.set! ptr type value . path)`**  
  Writes `value` to a pointer at a specific field path.

* **`(c.addr tp . path)`**  
  Gets a typed pointer representing the address of the sub-field at `path`.

* **`(c.null-ptr)`** / **`(c.null-ptr? x)`**  
  Helpers to construct and check for `NULL` pointers.

* **`(c.c-cast tp target-type)`**  
  Changes the type tag associated with a typed pointer.

* **`c.malloc-tracked`** / **`c.free-tracked`** / **`c.bounds-check`**  
  Wrapper definitions for the boundary tracking system.

### B. Accessor & Mutator Macros

* **`c.@`** (Deref path macro)  
  Dereferences a field or array path from a typed pointer.
  - *Syntax*: `(c.@ ptr.field.index)`
  ```scheme
  (c.@ data.points.0.x)
  ```

* **`c.=`** (Write path macro)  
  Writes a value to a field or array path from a typed pointer.
  - *Syntax*: `(c.= ptr.field.index value)`
  ```scheme
  (c.= data.points.0.x 42)
  ```

* **`c.&`** (Address-of macro)  
  Gets a typed pointer pointing to a sub-field or array slot.
  - *Syntax*: `(c.& ptr.field.index)`
  ```scheme
  (define y-addr (c.& data.points.0.y))
  ```

### C. Declarators

* **`define-c-struct`** (macro)  
  Registers a structured layout.
  ```scheme
  (define-c-struct Point
    (x int)
    (y int))
  ```

* **`define-c-union`** (macro)  
  Registers a overlapping union layout.
  ```scheme
  (define-c-union IntOrDouble
    (i int)
    (d double))
  ```

* **`define-c-enum`** (macro)  
  Declares list of named integer enum constants.
  ```scheme
  (define-c-enum State
    :state-pending
    :state-active)
  ```

* **`c-import`** (macro)  
  Imports a C function and binds it to a clean Scheme procedure wrapper.
  - *Syntax*: `(c-import scheme-name lib-handle c-name ret-type arg-types [nfixed])`
  ```scheme
  (c-import c-cos libc "cos" double (double))
  ```

### D. Scoped Sandbox Allocators
Macros that guarantee automatic memory cleanup when control leaves the execution scope:

* **`with-heap-alloc`** (macro)  
  Allocates heap memory for a struct or type descriptor.
  - *Syntax*: `(with-heap-alloc (var type [count]) body ...)`
  ```scheme
  (with-heap-alloc (p Point)
    (c.= p.x 10)
    (display (c.@ p.x)))
  ```

* **`with-c-string`** (macro)  
  Allocates a temporary null-terminated C string (`char*`).
  - *Syntax*: `(with-c-string (var "my string") body ...)`

* **`with-c-array`** (macro)  
  Allocates a temporary primitive array from a Scheme list.
  - *Syntax*: `(with-c-array (var int '(1 2 3)) body ...)`

* **`with-c-string-array`** (macro)  
  Allocates a temporary null-terminated pointer array (`char**`).
  - *Syntax*: `(with-c-string-array (var '("a" "b" "c")) body ...)`

* **`capture`** (macro)  
  Runs the body and captures standard output as a string.

---

## 7. The Task Runner Library: `libnob` Reference

The `libnob` library contains utility bindings for task running, process execution, file descriptors, file systems, arena stack allocations, and dynamic array handling.

> [!NOTE]
> Rather than maintaining a verbose duplicated list of these functions in text documentation, **you are encouraged to read the standard library code directly**. It is fully documented and readable:
> * [stdlib/libnob/lib.lext](file:///home/goad/Documents/dev/lext/stdlib/libnob/lib.lext)

### Summary of APIs available in `libnob`:
* **Process Runner**: `nob.cmd-new`, `nob.cmd-append`, `nob.cmd-run`, `nob.cmd-run-async`, `nob.cmd-free`, `nob.procs-wait-and-reset`, `nob.procs-append-with-flush`, `nob.pipe-create`.
* **Process Pipes / Chains**: `nob.chain-begin`, `nob.chain-cmd`, `nob.chain-end`.
* **File System Operations**: `nob.write-entire-file`, `nob.read-entire-file`, `nob.file-exists`, `nob.delete-file`, `nob.mkdir-if-not-exists`, `nob.copy-file`, `nob.copy-directory-recursively`, `nob.walk-dir`, `nob.get-file-type`, `nob.rename`.
* **Arena stack allocators**: `nob.temp-reset`, `nob.temp-save`, `nob.temp-rewind`, `nob.temp-strdup`, `nob.temp-alloc`.
* **String Views (`SV`)**: `nob.sv-from-cstr`, `nob.sv-trim`, `nob.sv-chop-left`, `nob.sv-chop-by-delim`, `nob.sv-eq`, `nob.sv->string`.
* **Dynamic Arrays**: `nob.da-items`, `nob.da-count`, `nob.da-capacity`, `nob.da-append`, `nob.da-pop`, `nob.da-free`, `nob.da-reserve`.

---

## 8. Template Engine & Raw String Literals

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

## 9. Emacs Major Mode Integration

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
