(use "stdlib/libnob" "stdlib/basic")

(display "=== 1. Testing Type Translation & Declarations ===\n")
(display (format #f "Translated (* t): ~A\n" (translate-ffi-type '(* t))))
(display (format #f "Translated (* Nob_Cmd): ~A\n" (translate-ffi-type '(* Nob_Cmd))))

(define-c-struct My_Inner
  (val int)
  (flag char))

(define-c-struct My_Outer
  (inner My_Inner)
  (ptr (* My_Inner))
  (name string))

(display (format #f "c-size My_Inner: ~A (expected: 8 due to alignment/padding)\n" (c-size 'My_Inner)))
(display (format #f "c-size My_Outer: ~A (expected: 24 due to alignment/padding)\n" (c-size 'My_Outer)))

(display "\n=== 2. Testing Memory Sandbox Allocation & @ / c-set! ===\n")
(with-heap-alloc (obj My_Outer)
  (bc.= obj.inner.val 42)
  (bc.= obj.inner.flag 120)
  (display (format #f "Read obj.inner.val: ~A\n" (bc.@ obj.inner.val)))
  (display (format #f "Read obj.inner.flag: ~A\n" (integer->char (bc.@ obj.inner.flag)))))

(display "\n=== 3. Testing with-c-string, with-c-array, with-c-string-array ===\n")
(with-c-string (s "Hello from s7 Basic!")
  (display (format #f "C string read: ~S\n" (c-string-from-ptr s))))

(with-c-array (arr int '(10 20 30 40))
  (display (format #f "Array [0]: ~A\n" (bc.@ arr.0)))
  (display (format #f "Array [2]: ~A\n" (bc.@ arr.2))))

(with-c-string-array (args '("arg1" "arg2" "arg3"))
  (display (format #f "String array [0]: ~A\n" (c-string-from-ptr (bc.@ args.0))))
  (display (format #f "String array [1]: ~A\n" (c-string-from-ptr (bc.@ args.1))))
  (display (format #f "String array [2]: ~A\n" (c-string-from-ptr (bc.@ args.2))))
  (display (format #f "String array as list: ~S\n" (c-string-array->list args))))

(display "\n=== 4. Testing capture macro ===\n")
(let ((captured (capture
                  (display "First stdout write.\n")
                  (display "Second stdout write.\n"))))
  (display (format #f "Captured string: ~S\n" captured)))

(display "\n=== 5. Testing Enum macro ===\n")
(define-c-enum my-states
  :state-pending
  :state-active
  (state-error -100))

(display (format #f "state-pending: ~A\n" state-pending))
(display (format #f "state-active: ~A\n" state-active))
(display (format #f "state-error: ~A\n" state-error))

(display "\n=== 6. Testing *platform* variable ===\n")
(display (format #f "Platform: ~S\n" *platform*))

(display "\n=== 7. Testing open-namespace and use-namespace ===\n")

;; Test open-namespace globally
(open-namespace "bc.")
(display (format #f "After open-namespace, malloc is defined: ~A\n" (defined? 'malloc)))
(display (format #f "After open-namespace, free is defined: ~A\n" (defined? 'free)))
;; Verify standard protected operator is not overwritten
(display (format #f "After open-namespace, standard = is still original: ~A\n" (eq? = (symbol->value '= (rootlet)))))

;; Test use-namespace locally (scoped)
(use-namespace "nob."
  (display (format #f "Inside use-namespace, log is defined: ~A\n" (defined? 'log)))
  (display (format #f "Inside use-namespace, cmd-new is defined: ~A\n" (defined? 'cmd-new))))

;; Verify that outside use-namespace, local bindings are not polluted/defined
(display (format #f "Outside use-namespace, log is defined: ~A\n" (defined? 'log)))
(display (format #f "Outside use-namespace, cmd-new is defined: ~A\n" (defined? 'cmd-new)))

(display "\n=== 8. Testing while, for, foreach Loop Constructs ===\n")
(display "Testing while:\n")
(let ((i 0))
  (while (< i 5)
    (display (format #f "  while: ~A\n" i))
    (set! i (+ i 1))))

(display "Testing for (range 0 to 5):\n")
(for (i 0 5)
  (display (format #f "  for: ~A\n" i)))

(display "Testing for with step (10 to 0 step -2):\n")
(for (i 10 0 -2)
  (display (format #f "  for-step: ~A\n" i)))

(display "Testing foreach:\n")
(foreach (x '(apple orange banana))
  (display (format #f "  foreach: ~A\n" x)))


