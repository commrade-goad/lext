;;; libc.scm - Lext FFI-based libc bindings
(provide 'libc.scm)

(use "stdlib/c")

(unless (defined? '*libc*)
  (varlet (rootlet) '*libc*
    (let ((libc-handle (ffi-open #f)))
      ;; Define FFI structures
      (c.define-c-struct utsname
        (sysname (array char 65))
        (nodename (array char 65))
        (release (array char 65))
        (version (array char 65))
        (machine (array char 65))
        (domainname (array char 65)))

      (c.define-c-struct timespec
        (tv_sec long)
        (tv_nsec long))

      (c.define-c-struct timeval
        (tv_sec long)
        (tv_usec long))

      (c.define-c-struct regmatch
        (rm_so int)
        (rm_eo int))

      (ffi-typedef 'regex_t '(array char 64))

      ;; Import raw C functions
      (c.c-import c-time-raw libc-handle "time" ulong (pointer))
      (c.c-import c-uname-raw libc-handle "uname" int (pointer))
      (c.c-import c-clock-gettime-raw libc-handle "clock_gettime" int (int pointer))
      (c.c-import c-regcomp-raw libc-handle "regcomp" int (pointer string int))
      (c.c-import c-regexec-raw libc-handle "regexec" int (pointer string ulong pointer int))
      (c.c-import c-regfree-raw libc-handle "regfree" void (pointer))
      (c.c-import c-regerror-raw libc-handle "regerror" ulong (int pointer pointer ulong))
      (c.c-import c-errno-location libc-handle "__errno_location" pointer ())
      (c.c-import c-strerror-raw libc-handle "strerror" string (int))
      (c.c-import c-getcwd-raw libc-handle "getcwd" string (pointer ulong))
      (c.c-import c-chdir-raw libc-handle "chdir" int (string))
      (c.c-import c-getenv-raw libc-handle "getenv" string (string))
      (c.c-import c-gettimeofday-raw libc-handle "gettimeofday" int (pointer pointer))
      (c.c-import c-memcpy-raw libc-handle "memcpy" pointer (pointer pointer ulong))

      ;; Wrappers for s7 compatibility
      (define (c-time . args)
        (c-time-raw (c.null-ptr)))

      (define (c-uname)
        (c.with-alloc (ut utsname)
          (c-uname-raw ut)
          (list (c.string-from-ptr (c.& ut.sysname))
                (c.string-from-ptr (c.& ut.nodename))
                (c.string-from-ptr (c.& ut.release))
                (c.string-from-ptr (c.& ut.version))
                (c.string-from-ptr (c.& ut.machine)))))

      (define (c-clock-gettime clk-id)
        (c.with-alloc (ts timespec)
          (c-clock-gettime-raw clk-id ts)
          (list 0 (c.@ ts.tv_sec) (c.@ ts.tv_nsec))))

      (define (c-getenvs)
        (let ((env-ptr-addr (ffi-sym libc-handle "environ")))
          (if (c.null-ptr? env-ptr-addr)
              '()
              (let ((env-array (c.deref env-ptr-addr 'pointer)))
                (let loop ((i 0) (res '()))
                  (let ((str-ptr (c.deref env-array 'pointer (* i (c.size 'pointer)))))
                    (if (c.null-ptr? str-ptr)
                        (reverse res)
                        (let* ((entry (c.string-from-ptr str-ptr))
                               (eq-idx (char-position #\= entry)))
                          (if eq-idx
                              (loop (+ i 1) (cons (cons (substring entry 0 eq-idx) (substring entry (+ eq-idx 1))) res))
                              (loop (+ i 1) (cons (cons entry "") res)))))))))))

      (define (c-regex-make)
        (c.tptr 'regex_t (c.malloc (c.size 'regex_t))))

      (define (c-regex-free rg)
        (c.c-free rg))

      (define (c-regerror errcode preg)
        (let ((buf (c.malloc 256)))
          (c-regerror-raw errcode preg buf 256)
          (let ((str (c.string-from-ptr buf)))
            (c.c-free buf)
            str)))

      (define (c-regexec preg str nmatches flags)
        (if (= nmatches 0)
            (c-regexec-raw preg str 0 (c.null-ptr) flags)
            (c.with-c-array (matches regmatch (make-list nmatches '(0 0)))
              (let ((res (c-regexec-raw preg str nmatches matches flags)))
                (if (not (= res 0))
                    res
                    (let ((vec (make-int-vector (* nmatches 2) 0)))
                      (let loop ((i 0))
                        (if (< i nmatches)
                            (let ((match (c.deref matches 'regmatch i)))
                              (set! (vec (* i 2)) (car match))
                              (set! (vec (+ (* i 2) 1)) (cadr match))
                              (loop (+ i 1)))))
                      vec))))))

      (define (c-errno)
        (let ((ptr (c-errno-location)))
          (c.deref ptr 'int)))

      (define (c-getcwd . args)
        (let* ((size (if (and (pair? args) (pair? (cdr args))) (cadr args) 512))
               (buf (c.malloc size)))
          (c-getcwd-raw buf size)
          (let ((res (c.string-from-ptr buf)))
            (c.c-free buf)
            res)))

      (define (c-gettimeofday)
        (c.with-alloc (tv timeval)
          (c-gettimeofday-raw tv (c.null-ptr))
          (list (c.@ tv.tv_sec) (c.@ tv.tv_usec))))

      ;; Create constants inlet using our native code generator's builder
      (let ((env (lext-make-libc-constants)))
        (varlet env
                'getenvs c-getenvs
                'uname   c-uname
                'time    c-time
                'clock_gettime c-clock-gettime
                'regex.make c-regex-make
                'regex.free c-regex-free
                'regcomp  c-regcomp-raw
                'regexec  c-regexec
                'regfree  c-regfree-raw
                'regerror c-regerror
                'errno    c-errno
                'strerror c-strerror-raw
                'getcwd   c-getcwd
                'chdir    c-chdir-raw
                'getenv   c-getenv-raw
                'gettimeofday c-gettimeofday
                'memcpy   c-memcpy-raw)
        (unless (defined? 'CLOCK_REALTIME env)
          (varlet env 'CLOCK_REALTIME 0))
        env))))

