;;;; nob_demo.scm - Comprehensive Demonstration of basic.scm and libnob.scm
(load "libnob.scm")
(load "basic.scm")

(display "=========================================================\n")
(display "         lext FFI & basic.scm Dev Kit Showcase\n")
(display "=========================================================\n\n")

;; 1. Compile C Shared Library using libnob's cmd runner
(display "--- 1. Compiling test_lib.c to libtest.so ---\n")
(let ((cmd-args '("gcc" "-shared" "-fPIC" "test_lib.c" "-o" "libtest.so")))
  (if (nob.cmd-run cmd-args)
      (display "Compilation successful: libtest.so created.\n")
      (error "Failed to compile test_lib.c")))

;; 2. Open and load the dynamic library
(define libtest (ffi-open "./libtest.so"))

;; 3. Define FFI layouts using basic.scm declarators
(display "\n--- 2. Defining C Struct and Union Layouts ---\n")
(define-c-struct Point
  (x int)
  (y int))

(define-c-union IntOrDouble
  (i int)
  (d double))

(display (format #f "c-size Point: ~A bytes\n" (c-size 'Point)))
(display (format #f "c-size IntOrDouble: ~A bytes\n" (c-size 'IntOrDouble)))

;; 4. Import the compiled C functions
(c-import add-points  libtest "add_points"  Point       (Point Point))
(c-import print-union libtest "print_union" double      (IntOrDouble int))

;; 5. Allocate and perform math using Memory Sandbox and Pointer navigation
(display "\n--- 3. Struct Math using Allocators, @, and c-set! ---\n")
(with-alloc (p1 Point)
  (with-alloc (p2 Point)
    ;; Initialize p1 = (10, 20)
    (bc.= p1.x 10)
    (bc.= p1.y 20)
    ;; Initialize p2 = (30, 40)
    (bc.= p2.x 30)
    (bc.= p2.y 40)
    
    (display (format #f "p1 = (~A, ~A)\n" (bc.@ p1.x) (bc.@ p1.y)))
    (display (format #f "p2 = (~A, ~A)\n" (bc.@ p2.x) (bc.@ p2.y)))
    
    ;; Invoke C function (passing structs by value, returning struct by value)
    (let ((result (add-points (bc.@ p1) (bc.@ p2))))
      (display (format #f "add_points result = (~A, ~A)\n" 
                       (cdr (assoc 'x result)) 
                       (cdr (assoc 'y result)))))))

;; 6. Capture C printf calls using POSIX Redirection Engine
(display "\n--- 4. Capturing C Printf Outputs using capture ---\n")
(with-alloc (u IntOrDouble)
  ;; Test 1: Write double to union
  (bc.= u.d 3.14159)
  (display (format #f "Union field 'd read via @: ~A\n" (bc.@ u.d)))
  (let ((output (capture (print-union '(d 3.14159) 1))))
    (display (format #f "Double print output captured: ~S\n" output)))

  ;; Test 2: Write int to union
  (bc.= u.i 1337)
  (display (format #f "Union field 'i read via @: ~A\n" (bc.@ u.i)))
  (let ((output (capture (print-union '(i 1337) 0))))
    (display (format #f "Integer print output captured: ~S\n" output))))

(define (ends-with-scm? path)
  (let ((len (string-length path)))
    (and (>= len 4) (string=? (substring path (- len 4)) ".scm"))))

;; 7. Directory walking
(display "\n--- 5. Traversal using walk-dir ---\n")
(display "Listing workspace Scheme files:\n")
(nob.walk-dir "."
              (lambda (path type level)
                (if (and (eq? type 'regular) (ends-with-scm? path))
                    (display (format #f "  - ~A\n" path)))
                #t))

;; 8. Callback Testing (Scheme -> C -> Scheme)
(display "\n--- 6. Testing Scheme -> C Callbacks ---\n")
(c-import run-callback libtest "run_callback" double (pointer int))

(define (my-scheme-callback val)
  (display (format #f "  Scheme callback invoked with: ~A\n" val))
  (* val 1.5))

(let* ((cb-ptr (ffi-callback my-scheme-callback 'double '(int)))
       (res (run-callback cb-ptr 100)))
  (display (format #f "run-callback returned: ~A (expected: 150.0)\n" res)))

;; 9. Clean up
(display "\nCleaning up libtest.so...\n")
(nob.delete-file "libtest.so")
(display "Done!\n")
