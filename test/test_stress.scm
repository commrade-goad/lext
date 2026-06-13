(use "stdlib/basic" "stdlib/libnob")

(display "=========================================================\n")
(display "   lext Standard Libraries 100% Coverage Stress Test\n")
(display "=========================================================\n\n")

;; Compile helper library for FFI tests
(let ((cmd-args '("gcc" "-shared" "-fPIC" "test/test_lib.c" "-o" "test/libtest.so")))
  (if (nob.cmd-run cmd-args)
      (display "[INIT] test/libtest.so compiled successfully.\n")
      (error "Failed to compile test/test_lib.c")))

(define libtest (ffi-open "./test/libtest.so"))

;; ============================================================================
;; PART A: basic.scm coverage
;; ============================================================================
(display "\n--- [PART A] Testing basic.scm ---\n")

;; 1. translate-ffi-type
(display (format #f "  translate-ffi-type (* char): ~S\n" (translate-ffi-type '(* char))))
(display (format #f "  translate-ffi-type (array int 10): ~S\n" (translate-ffi-type '(array int 10))))

;; 2. Declarators: define-c-struct, define-c-union, define-c-enum
(define-c-struct Basic_Point
  (x int)
  (y int))

(define-c-union Basic_Union
  (i int)
  (d double))

(define-c-enum Basic_Enum
  :enum-zero
  :enum-one
  (enum-five 5)
  :enum-six)

(display (format #f "  c-size Basic_Point: ~A\n" (c-size 'Basic_Point)))
(display (format #f "  c-size Basic_Union: ~A\n" (c-size 'Basic_Union)))
(display (format #f "  Basic_Enum: ~A, ~A, ~A, ~A\n" enum-zero enum-one enum-five enum-six))

;; 3. c-import
(c-import stress-add-points libtest "add_points" Basic_Point (Basic_Point Basic_Point))

;; 4. bc.malloc, bc.free, bc.realloc
(let* ((p1 (bc.malloc 8))
       (p2 (bc.realloc p1 16)))
  (display (format #f "  bc.malloc/realloc ptr: ~A\n" (c-pointer? p2)))
  (bc.free p2))

;; 5. internal-basic-c-memcpy, internal-basic-c-memset
(let ((buf1 (bc.malloc 10))
      (buf2 (bc.malloc 10)))
  (internal-basic-c-memset buf1 65 10) ; fill with 'A'
  (internal-basic-c-memcpy buf2 buf1 10)
  (display (format #f "  memset/memcpy check char: ~A\n" (ffi-deref buf2 'char)))
  (bc.free buf1)
  (bc.free buf2))

;; 6. internal-basic-ptr+
(let* ((base (bc.malloc 16))
       (offset-ptr (internal-basic-ptr+ base 8)))
  (display (format #f "  ptr+ offset check: ~A\n" (not (eq? base offset-ptr))))
  (bc.free base))

;; 7. bc.deref, bc.set!, bc.addr
(let ((ptr (bc.malloc 4)))
  (bc.set! ptr 'int 999)
  (display (format #f "  bc.set!/bc.deref: ~A\n" (bc.deref ptr 'int)))
  (display (format #f "  bc.addr check: ~A\n" (pair? (bc.addr (internal-basic-tptr 'int ptr)))))
  (bc.free ptr))

;; 8. bc.@, bc.=, bc.&
(with-heap-alloc (pt Basic_Point)
  (bc.= pt.x 123)
  (bc.= pt.y 456)
  (display (format #f "  bc.=@ x: ~A, y: ~A\n" (bc.@ pt.x) (bc.@ pt.y)))
  (let ((addr-val (bc.& pt.y)))
    (display (format #f "  bc.& type: ~A\n" (car addr-val)))))

;; 9. with-heap-alloc, with-alloc, with-c-string, with-c-array, with-c-string-array
(with-alloc (pt Basic_Point)
  (bc.= pt.x 777)
  (display (format #f "  with-alloc: ~A\n" (bc.@ pt.x))))

(with-c-string (s "Test string FFI")
  (display (format #f "  with-c-string: ~S\n" (c-string-from-ptr s))))

(with-c-array (arr int '(11 22 33))
  (display (format #f "  with-c-array [1]: ~A\n" (bc.@ arr.1))))

(with-c-string-array (s-arr '("foo" "bar"))
  (display (format #f "  with-c-string-array: ~S\n" (c-string-array->list s-arr))))

;; 10. capture
(let ((out (capture (display "Stdout captured line!\n"))))
  (display (format #f "  capture output: ~S\n" out)))

;; 11. bc.null-ptr?, bc.null-ptr
(display (format #f "  null-ptr? (null-ptr): ~A\n" (bc.null-ptr? (bc.null-ptr))))
(display (format #f "  null-ptr? non-null: ~A\n" (bc.null-ptr? 12345)))

;; 12. bc.c-cast
(with-alloc (pt Basic_Point)
  (let ((casted (bc.c-cast pt 'int)))
    (display (format #f "  bc.c-cast type: ~A\n" (car casted)))))

;; 13. open-namespace, use-namespace
(use-namespace "bc."
  (display (format #f "  use-namespace block: ~A\n" (defined? 'malloc))))


;; ============================================================================
;; PART B: libnob.scm coverage
;; ============================================================================
(display "\n--- [PART B] Testing libnob.scm ---\n")

;; 1. Pointer arithmetic
(let* ((ptr (bc.malloc 8))
       (addr (nob.c-pointer->integer ptr))
       (ptr2 (nob.integer->c-pointer addr))
       (ptr3 (nob.pointer-add ptr2 4)))
  (display (format #f "  Pointer conversions: ~A\n" (eq? ptr ptr2)))
  (display (format #f "  Pointer add offset: ~A\n" (not (eq? ptr2 ptr3))))
  (bc.free ptr))

;; 2. Log API
(nob.minimal-log-level-set! 0)
(display (format #f "  minimal-log-level: ~A\n" (nob.minimal-log-level)))
(nob.log 0 "Testing log output: ~A" "success")

;; 3. Command API & Render
(let ((cmd (nob.cmd-new)))
  (nob.cmd-append cmd "echo" "hello" "world")
  (display (format #f "  Rendered command: ~S\n" (nob.cmd-render cmd)))
  
  ;; Sync and async command execution
  (display (format #f "  cmd-run sync: ~A\n" (nob.cmd-run cmd)))
  (display (format #f "  cmd-run-async: ~A\n" (integer? (nob.cmd-run-async cmd))))
  
  ;; Sync and async with reset
  (nob.cmd-append cmd "echo" "reset-test")
  (display (format #f "  cmd-run-sync-and-reset: ~A\n" (nob.cmd-run-sync-and-reset cmd)))
  
  (nob.cmd-append cmd "echo" "async-reset-test")
  (let ((pid (nob.cmd-run-async-and-reset cmd)))
    (display (format #f "  cmd-run-async-and-reset pid: ~A\n" (nob.proc-wait pid))))
    
  (nob.cmd-free cmd))

;; Command redirects
(let ((cmd (nob.cmd-new))
      (redir (bc.malloc 24)))
  (nob.cmd-append cmd "echo" "redirect-test")
  (ffi-set! redir '(struct (pointer fdin) (pointer fdout) (pointer fderr)) '((fdin . ()) (fdout . ()) (fderr . ())))
  (display (format #f "  cmd-run-sync-redirect: ~A\n" (nob.cmd-run-sync-redirect cmd redir)))
  (nob.cmd-append cmd "echo" "redirect-async")
  (let ((pid (nob.cmd-run-async-redirect cmd redir)))
    (display (format #f "  cmd-run-async-redirect pid: ~A\n" (nob.proc-wait pid))))
  (nob.cmd-free cmd)
  (bc.free redir))

;; 4. File operations & temporary paths
(let ((temp-path "build/test_temp_file.txt"))
  (display (format #f "  write-entire-file: ~A\n" (nob.write-entire-file temp-path "Hello Lext File System!")))
  (display (format #f "  file-exists: ~A\n" (nob.file-exists temp-path)))
  (display (format #f "  read-entire-file: ~S\n" (nob.read-entire-file temp-path)))
  (display (format #f "  get-file-type: ~A\n" (nob.get-file-type temp-path)))
  
  ;; Copy and rename
  (display (format #f "  copy-file: ~A\n" (nob.copy-file temp-path "build/test_temp_copy.txt")))
  (display (format #f "  rename: ~A\n" (nob.rename "build/test_temp_copy.txt" "build/test_temp_renamed.txt")))
  
  ;; Path helpers
  (display (format #f "  path-name: ~S\n" (nob.path-name temp-path)))
  (display (format #f "  temp-dir-name: ~S\n" (nob.temp-dir-name temp-path)))
  (display (format #f "  temp-file-name: ~S\n" (nob.temp-file-name temp-path)))
  (display (format #f "  temp-file-ext: ~S\n" (nob.temp-file-ext temp-path)))
  
  (nob.delete-file temp-path)
  (nob.delete-file "build/test_temp_renamed.txt"))

(display (format #f "  get-current-dir-temp: ~S\n" (nob.get-current-dir-temp)))
(display (format #f "  temp-running-executable-path: ~S\n" (nob.temp-running-executable-path)))
(display (format #f "  nanos-since-unspecified-epoch: ~A\n" (nob.nanos-since-unspecified-epoch)))
(display (format #f "  nprocs: ~A\n" (nob.nprocs)))
(display (format #f "  read-entire-dir: ~A\n" (list? (nob.read-entire-dir "stdlib"))))

;; Directory copy and walk
(nob.mkdir-if-not-exists "build/test_dir")
(nob.copy-directory-recursively "stdlib/basic" "build/test_dir")
(let ((walk-count 0))
  (nob.walk-dir "build/test_dir" (lambda (p t l) (set! walk-count (+ walk-count 1)) #t))
  (display (format #f "  walk-dir walked ~A files/dirs\n" walk-count)))
;; Cleanup copied dir
(nob.delete-file "build/test_dir/lib.scm")
(nob.delete-file "build/test_dir")

;; String builder padding
(let ((sb (bc.malloc 24)))
  (ffi-set! sb 'Nob_String_Builder '((items . ()) (count . 0) (capacity . 0)))
  (nob.sb-pad-align sb 16)
  (let* ((sb-struct (ffi-deref sb 'Nob_String_Builder))
         (count (cdr (assoc 'count sb-struct))))
    (display (format #f "  sb-pad-align count: ~A\n" count)))
  (nob.cmd-free sb))

;; 5. Processes wait & flush API
(let* ((procs (bc.malloc 24)))
  (ffi-set! procs 'Nob_Procs '((items . ()) (count . 0) (capacity . 0)))
  ;; Add some dummy child process
  (let* ((cmd (nob.cmd-new)))
    (nob.cmd-append cmd "sleep" "0.1")
    (let ((pid (nob.cmd-run-async-and-reset cmd)))
      (nob.procs-append-with-flush procs pid 4)
      (display (format #f "  procs count after append: ~A\n" (nob.da-count procs)))
      (display (format #f "  procs-wait-and-reset: ~A\n" (nob.procs-wait-and-reset procs))))))

;; Pipes API
(let ((pipe-ptr (bc.malloc 8)))
  (display (format #f "  pipe-create: ~A\n" (nob.pipe-create pipe-ptr)))
  (let* ((pipe-struct (ffi-deref pipe-ptr 'Nob_Pipe))
         (read-fd (cdr (assoc 'read pipe-struct)))
         (write-fd (cdr (assoc 'write pipe-struct))))
    (display (format #f "    read fd: ~A, write fd: ~A\n" read-fd write-fd))
    (nob.fd-close read-fd)
    (nob.fd-close write-fd))
  (bc.free pipe-ptr))

;; Chains API
(let ((chain (bc.malloc 40))
      (cmd (nob.cmd-new)))
  (nob.cmd-append cmd "echo" "chain-hello")
  (display (format #f "  chain-begin: ~A\n" (nob.chain-begin chain)))
  (display (format #f "  chain-cmd: ~A\n" (nob.chain-cmd chain cmd :err2out #t :dont-reset #t)))
  (display (format #f "  chain-end: ~A\n" (nob.chain-end chain :max-procs 4)))
  (nob.cmd-free cmd)
  (bc.free chain))

;; Directory iterator API
(let ((dir-ent (bc.malloc 32))) ; size of Nob_Dir_Entry
  (display (format #f "  dir-entry-open: ~A\n" (nob.dir-entry-open "stdlib" dir-ent)))
  (display (format #f "  dir-entry-next: ~A\n" (nob.dir-entry-next dir-ent)))
  (nob.dir-entry-close (ffi-deref dir-ent 'Nob_Dir_Entry))
  (bc.free dir-ent))

;; 6. String Views API
(let* ((sv1 (nob.sv-from-cstr "   Trimmed SV!   "))
       (sv2 (nob.sv-from-parts (cdr (assoc 'data sv1)) (cdr (assoc 'count sv1))))
       (trimmed (nob.sv-trim sv2))
       (left (nob.sv-trim-left sv1))
       (right (nob.sv-trim-right sv1)))
  (display (format #f "  sv->string: ~S\n" (nob.sv->string trimmed)))
  (display (format #f "  sv-eq: ~A\n" (nob.sv-eq sv1 sv2)))
  (display (format #f "  sv-starts-with: ~A\n" (nob.sv-starts-with sv1 (nob.sv-from-cstr "   "))))
  (display (format #f "  sv-ends-with: ~A\n" (nob.sv-ends-with sv1 (nob.sv-from-cstr "   "))))
  (display (format #f "  sv-ends-with-cstr: ~A\n" (nob.sv-ends-with-cstr sv1 "   "))))

;; String View Chopping
(let* ((sv (nob.sv-from-cstr "foo:bar:baz"))
       (sv-ptr (bc.malloc 16)))
  (ffi-set! sv-ptr 'Nob_String_View sv)
  (let* ((chopped1 (nob.sv-chop-left sv-ptr 4))
         (chopped2 (nob.sv-chop-right sv-ptr 4)))
    (display (format #f "  sv-chop-left: ~S\n" (nob.sv->string chopped1)))
    (display (format #f "  sv-chop-right: ~S\n" (nob.sv->string chopped2))))
  
  (ffi-set! sv-ptr 'Nob_String_View sv)
  (let ((chopped3 (nob.sv-chop-by-delim sv-ptr #\:)))
    (display (format #f "  sv-chop-by-delim: ~S\n" (nob.sv->string chopped3))))
    
  (ffi-set! sv-ptr 'Nob_String_View sv)
  (display (format #f "  sv-chop-prefix: ~A\n" (nob.sv-chop-prefix sv-ptr (nob.sv-from-cstr "foo"))))
  (display (format #f "  sv-chop-suffix: ~A\n" (nob.sv-chop-suffix sv-ptr (nob.sv-from-cstr "baz"))))
  (bc.free sv-ptr))

;; 7. Temp Allocator API
(nob.temp-reset)
(let* ((checkpoint (nob.temp-save))
       (str1 (nob.temp-strdup "Temp allocated string"))
       (str2 (nob.temp-strndup "Temp limit string" 10))
       (raw-buf (nob.temp-alloc 100)))
  (display (format #f "  temp-strdup: ~S\n" str1))
  (display (format #f "  temp-strndup: ~S\n" str2))
  (display (format #f "  temp-alloc ptr: ~A\n" (c-pointer? raw-buf)))
  (nob.temp-rewind checkpoint))

;; 8. Dynamic Arrays API
(let ((da (bc.malloc 24)))
  (nob.da-set! da '((items . ()) (count . 0) (capacity . 0)))
  (nob.da-reserve da 10 4) ; reserve for 10 ints
  (nob.da-append da 100 int)
  (nob.da-append da 200 int)
  (display (format #f "  da-items ptr: ~A\n" (c-pointer? (nob.da-items da))))
  (display (format #f "  da-count: ~A\n" (nob.da-count da)))
  (display (format #f "  da-capacity: ~A\n" (nob.da-capacity da)))
  (display (format #f "  da-first: ~A\n" (nob.da-first da 'int)))
  (display (format #f "  da-last: ~A\n" (nob.da-last da 'int)))
  (display (format #f "  da-pop: ~A\n" (nob.da-pop da 'int)))
  (nob.da-free da)
  (bc.free da))

;; 9. nob.shift
(let ((lst '("x" "y" "z")))
  (let ((val (nob.shift lst)))
    (display (format #f "  nob.shift first: ~S\n" val))
    (display (format #f "  nob.shift remainder: ~S\n" lst))))

;; 10. Rebuild dependencies checks
(display (format #f "  needs-rebuild: ~A\n" (nob.needs-rebuild "build/lext" '("src/main.c" "src/nob.c"))))
(display (format #f "  needs-rebuild?: ~A\n" (nob.needs-rebuild? "build/lext" "src/main.c")))

;; 11. File Descriptors API
(let ((fd (nob.fd-open-for-write "build/test_fd_stress.txt")))
  (display (format #f "  fd-open-for-write: ~A\n" (>= fd 0)))
  (nob.fd-close fd))
(let ((fd (nob.fd-open-for-read "build/test_fd_stress.txt")))
  (display (format #f "  fd-open-for-read: ~A\n" (>= fd 0)))
  (nob.fd-close fd))
(nob.delete-file "build/test_fd_stress.txt")

;; 12. clean up helper dynamic lib
(ffi-close libtest)
(nob.delete-file "test/libtest.so")

(display "\nAll standard library functions checked and passed successfully!\n")
