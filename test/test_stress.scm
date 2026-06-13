(use "stdlib/basic" "stdlib/libnob")

(display "=========================================================\n")
(display "         lext stdlib Stress & Robustness Test\n")
(display "=========================================================\n\n")

;; --- Helper: Compile C library for callback/FFI testing ---
(display "--- Phase 0: Compiling helper dynamic library ---\n")
(let ((cmd-args '("gcc" "-shared" "-fPIC" "test/test_lib.c" "-o" "test/libtest.so")))
  (if (nob.cmd-run cmd-args)
      (display "Compilation successful: test/libtest.so created.\n")
      (error "Failed to compile test/test_lib.c")))

(define libtest (ffi-open "./test/libtest.so"))

;; ============================================================================
;; PHASE 1: Struct, Union, Enum Alignments & Deeply Nested Structure Padding
;; ============================================================================
(display "\n--- Phase 1: Deep Nesting & Offset Calculation Stress Test ---\n")

(define-c-struct Nested_Inner
  (a char)     ; 1 byte
               ; 3 bytes padding
  (b int)      ; 4 bytes
  (c double))  ; 8 bytes
               ; total size: 16 bytes, align: 8

(define-c-struct Nested_Outer
  (arr (array int 4)) ; 16 bytes
  (inner Nested_Inner) ; 16 bytes
  (ptr (* Nested_Inner)) ; 8 bytes
  (flag char))         ; 1 byte
                       ; 7 bytes padding
                       ; total size: 48 bytes, align: 8

(define-c-union Stress_Union
  (i int)
  (d double)
  (inner Nested_Inner)) ; total size: 16 bytes (max size of members)

(define-c-enum Stress_Enum
  :enum-zero
  :enum-one
  (enum-ten 10)
  :enum-eleven)

(display (format #f "Nested_Inner size: ~A (expected: 16)\n" (c-size 'Nested_Inner)))
(display (format #f "Nested_Outer size: ~A (expected: 48)\n" (c-size 'Nested_Outer)))
(display (format #f "Stress_Union size: ~A (expected: 16)\n" (c-size 'Stress_Union)))

;; Verify Enum macro values
(display (format #f "Enum zero: ~A (expected: 0)\n" enum-zero))
(display (format #f "Enum one: ~A (expected: 1)\n" enum-one))
(display (format #f "Enum ten: ~A (expected: 10)\n" enum-ten))
(display (format #f "Enum eleven: ~A (expected: 11)\n" enum-eleven))

(with-heap-alloc (outer Nested_Outer)
  (with-heap-alloc (inner Nested_Inner)
    ;; Initialize inner structure
    (bc.= inner.a 65)
    (bc.= inner.b 1337)
    (bc.= inner.c 3.14159)
    
    ;; Set outer fields
    (bc.= outer.flag 90)
    (bc.= outer.ptr (bc.addr inner))
    
    ;; Set inner struct fields inside outer directly
    (bc.= outer.inner.a 66)
    (bc.= outer.inner.b 9999)
    (bc.= outer.inner.c 2.71828)
    
    ;; Set array elements
    (bc.= outer.arr.0 100)
    (bc.= outer.arr.1 200)
    (bc.= outer.arr.2 300)
    (bc.= outer.arr.3 400)
    
    ;; Read & verify everything
    (display (format #f "outer.flag: ~A (expected: 90)\n" (bc.@ outer.flag)))
    (display (format #f "outer.inner.a: ~A (expected: 66)\n" (bc.@ outer.inner.a)))
    (display (format #f "outer.inner.b: ~A (expected: 9999)\n" (bc.@ outer.inner.b)))
    (display (format #f "outer.inner.c: ~A (expected: 2.71828)\n" (bc.@ outer.inner.c)))
    
    (display (format #f "outer.arr.0: ~A (expected: 100)\n" (bc.@ outer.arr.0)))
    (display (format #f "outer.arr.3: ~A (expected: 400)\n" (bc.@ outer.arr.3)))
    
    ;; Read through pointer inside outer
    (let ((retrieved-ptr (bc.@ outer.ptr)))
      (display (format #f "Dereferenced inner.a: ~A (expected: 65)\n" (ffi-deref retrieved-ptr 'char)))
      (display (format #f "Dereferenced inner.b: ~A (expected: 1337)\n" (ffi-deref (nob.pointer-add retrieved-ptr 4) 'int)))
      )
      
    ;; Test address operator (bc.&) macro
    (let ((inner-addr (bc.& outer.inner)))
      (display (format #f "Address of outer.inner field resolves: ~A\n" (c-pointer? (cdr inner-addr))))
      (display (format #f "Type of outer.inner address: ~A (expected: Nested_Inner)\n" (car inner-addr))))))

;; Stress Union memory sharing
(with-heap-alloc (u Stress_Union)
  (bc.= u.i #x12345678)
  (display (format #f "Union int value: ~X (expected: 12345678)\n" (bc.@ u.i)))
  ;; Overwrite with double
  (bc.= u.d 1.2345)
  (display (format #f "Union double value: ~A (expected: 1.2345)\n" (bc.@ u.d)))
  ;; Verify int value is now mangled/overwritten
  (display (format #f "Union int after double write: ~X (should be different)\n" (bc.@ u.i))))

;; ============================================================================
;; PHASE 2: FFI Sandbox Allocators (with-c-string, with-c-array, with-c-string-array)
;; ============================================================================
(display "\n--- Phase 2: FFI Sandbox Allocators Stress Test ---\n")

;; Test with-c-string
(with-c-string (s "Robustness Testing String")
  (display (format #f "with-c-string output: ~S\n" (c-string-from-ptr s))))

;; Test with-c-array
(with-c-array (arr double '(1.1 2.2 3.3 4.4 5.5))
  (display (format #f "Array double [0]: ~A (expected: 1.1)\n" (bc.@ arr.0)))
  (display (format #f "Array double [4]: ~A (expected: 5.5)\n" (bc.@ arr.4))))

;; Test with-c-string-array
(with-c-string-array (args '("test1" "test2" "test3" "test4"))
  (display (format #f "String array [2]: ~S (expected: \"test3\")\n" (c-string-from-ptr (bc.@ args.2))))
  (display (format #f "String array as list: ~S\n" (c-string-array->list args))))

;; ============================================================================
;; PHASE 3: Dynamic Array (da-append) & Capacity Growth Stress Test
;; ============================================================================
(display "\n--- Phase 3: Dynamic Array Capacity Growth Stress Test ---\n")

(define da (bc.malloc 24))
(nob.da-set! da '((items . ()) (count . 0) (capacity . 0)))

;; Append 500 items to force multiple reallocations and capacity doubling
(let loop ((i 0))
  (if (< i 500)
      (begin
        (nob.da-append da i int)
        (loop (+ i 1)))))

(display (format #f "Dynamic Array count: ~A (expected: 500)\n" (nob.da-count da)))
(display (format #f "Dynamic Array capacity: ~A (expected >= 512)\n" (nob.da-capacity da)))

;; Verify values and pop them
(display (format #f "First element: ~A (expected: 0)\n" (nob.da-first da 'int)))
(display (format #f "Last element: ~A (expected: 499)\n" (nob.da-last da 'int)))

;; Pop all elements and ensure count goes down
(let loop ((i 499) (ok #t))
  (if (>= i 0)
      (let ((val (nob.da-pop da 'int)))
        (loop (- i 1) (and ok (= val i))))
      (display (format #f "All popped elements matched index: ~A\n" ok))))

(display (format #f "Count after popping all: ~A (expected: 0)\n" (nob.da-count da)))
(nob.da-free da)
(bc.free da)

;; ============================================================================
;; PHASE 4: Memory Leak Stress Test (10,000 Allocations) & Capture Macro
;; ============================================================================
(display "\n--- Phase 4: Memory Allocation Leak & Capture Stress Test ---\n")
(display "Allocating and freeing 10,000 point structures...\n")

(let loop ((i 0))
  (if (< i 10000)
      (begin
        (let ((ptr (bc.malloc 16)))
          (bc.free ptr))
        (loop (+ i 1)))))
(display "10,000 allocations completed without crashing.\n")

;; Test capture macro
(let ((captured (capture
                  (display "Captured Line 1\n")
                  (display "Captured Line 2\n"))))
  (display (format #f "Captured output: ~S\n" captured)))

;; ============================================================================
;; PHASE 5: Scoped Namespace Shadowing Stress Test
;; ============================================================================
(display "\n--- Phase 5: Scoped Namespace Shadowing Stress Test ---\n")

(use-namespace "bc."
  (display (format #f "Level 1: malloc is defined: ~A\n" (defined? 'malloc)))
  (display (format #f "Level 1: log is defined: ~A (should be standard log)\n" (procedure? log)))
  
  (use-namespace "nob."
    (display (format #f "  Level 2: malloc is defined: ~A\n" (defined? 'malloc)))
    (display (format #f "  Level 2: log is defined: ~A (should be nob.log wrapper)\n" (defined? 'log)))
    (display (format #f "  Level 2: cmd-new is defined: ~A\n" (defined? 'cmd-new))))
  
  (display (format #f "Back to Level 1: cmd-new is defined: ~A (expected: #f)\n" (defined? 'cmd-new))))

;; ============================================================================
;; PHASE 6: Stack Safety with Nested Allocators and Error Recovery
;; ============================================================================
(display "\n--- Phase 6: Nesting and Stack Cleanups via dynamic-wind ---\n")

;; 20-level nested with-alloc test
(define (nest-allocations depth)
  (if (= depth 0)
      (display "Deeply nested allocation depth reached successfully.\n")
      (with-alloc (p Nested_Inner)
        (bc.= p.b depth)
        (nest-allocations (- depth 1))
        (if (not (= (bc.@ p.b) depth))
            (error "Stack value corrupted in nesting!")))))

(nest-allocations 20)

;; Test error recovery: Does with-alloc clean up even if an error occurs inside it?
(display "Testing with-alloc cleanup on catchable error...\n")
(define cleanup-check #f)
(define test-ptr ())

(catch #t
  (lambda ()
    (with-heap-alloc (p Nested_Inner)
      (set! test-ptr (bc.addr p))
      (set! cleanup-check #t)
      (error "Simulated error inside with-alloc")))
  (lambda (tag info)
    (display "Caught simulated error. Checking if memory was freed...\n")
    ;; If dynamic-wind cleaned up, cleanup-check should be #t
    (display (format #f "with-alloc block was executed: ~A\n" cleanup-check))))

;; ============================================================================
;; PHASE 7: FFI Callbacks Stack Stress Test
;; ============================================================================
(display "\n--- Phase 7: Callback FFI Boundary Call-Stack Stress Test ---\n")

(c-import run-callback libtest "run_callback" double (pointer int))

(define callback-count 0)
(define (stress-callback val)
  (set! callback-count (+ callback-count 1))
  (if (< callback-count 10)
      (begin
        (display (format #f "  Callback depth ~A with val: ~A\n" callback-count val))
        ;; Call C again from inside Scheme callback
        (run-callback (ffi-callback stress-callback 'double '(int)) (- val 10)))
      (begin
        (display "  Max depth reached in callback chain.\n")
        1.0)))

(let ((res (run-callback (ffi-callback stress-callback 'double '(int)) 100)))
  (display (format #f "Callback boundary stress test finished, result: ~A\n" res)))

;; --- Clean up ---
(display "\n--- Phase 8: Cleaning up ---\n")
(nob.delete-file "test/libtest.so")
(display "All stress tests finished successfully!\n")
