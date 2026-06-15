# Features Nice to Have

This document lists future features and improvements to enhance the low-level FFI wrapper and helper libraries.

## 1. Heap Memory Garbage Collection Finalizers
* **Description**: Automatic cleanup of heap-allocated pointers (`malloc`ed structures) when their Scheme-side reference is garbage collected.
* **Why**: Prevents memory leaks in complex, long-running applications where manual `free` tracking might be error-prone, while still allowing C-style low-level management when needed.
* **How**: Hook into s7's garbage collector using finalizers on Scheme C-pointer objects or wrap pointers in custom types that clean up their allocations on GC sweeps.

## 2. FFI Sandbox Bounds Checking ("Safe Mode") [COMPLETED]
* **Description**: An optional debugging/safe mode to check offset calculations and dereferences before making raw memory reads/writes.
* **Why**: FFI code easily crashes the entire interpreter with a Segmentation Fault on out-of-bounds errors. Bounds checking would catch these early and throw a Scheme-level error instead of crashing.
* **How**: Track allocation block sizes in a Scheme-side table when calling `with-alloc` or `malloc`. Before any `bc.@` or `bc.=` expansion, verify that the computed address range falls completely within the bounds of the registered block.
* **Implementation**: Stored and validated on the C side using a MeowHash table. Exposed as `c.malloc-tracked`, `c.free-tracked`, and `c.bounds-check` in the standard `c` library module.

## 3. Platform-Specific Conditional Imports
* **Description**: A macro helper to import different library symbols based on the target OS.
* **Why**: Functions might have different names or availability depending on the platform (e.g. POSIX `fork` vs Windows equivalent, or differences in standard library names).
* **How**: Use the exposed `*platform*` variable to load platform-specific DLLs or dynamically select which function signatures to bind.

## 4. Interactive Struct Inspector (`bc.print-struct`)
* **Description**: A helper function to format and print all nested struct fields from a typed pointer wrapper's memory address.
* **Why**: Makes debugging FFI data layouts in the REPL much easier, instead of dereferencing each nested field individually.
* **How**: Query struct metadata recursively from `*ffi-types*` and call `bc.deref` for each primitive value, displaying a formatted tree structure.

## 5. Auto-Checking Type Safety on `c-import`
* **Description**: Generate type-checking assertions inside the Scheme wrappers produced by `c-import`.
* **Why**: Prevents passing incorrect pointer types or raw scalars into compiled foreign functions, catching bugs at the Scheme boundary instead of crashing the interpreter.
* **How**: Inspect target parameter types (e.g., `'Point`) and wrap the function call in type assertions that verify `(tptr? arg)` and `(eq? (tptr-type arg) 'Point)`.

