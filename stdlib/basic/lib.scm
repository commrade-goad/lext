;;;; basic.scm - Ultimate FFI & Dev Stdlib for s7 Scheme FFI
;;;; Ported from Documents/dev/lext-sbcl/basic.lisp

(define internal-basic-libc (ffi-open #f))

;; --- Typed Pointer Wrapper ---
(define (internal-basic-tptr type ptr)
  (cons type ptr))

(define (internal-basic-tptr-type tp)
  (car tp))

(define (internal-basic-tptr-ptr tp)
  (cdr tp))

(define (internal-basic-tptr? x)
  (and (pair? x)
       (c-pointer? (cdr x))))


;; --- Type Translation Matrix ---
(define (translate-ffi-type type)
  (cond
    ((and (pair? type) (eq? (car type) '*))
     'pointer)
    ((and (pair? type) (eq? (car type) 'struct))
     `(struct ,@(map (lambda (field)
                       (if (and (pair? field) (>= (length field) 2))
                           `(,(translate-ffi-type (car field)) ,(cadr field))
                           field))
                     (cdr type))))
    ((and (pair? type) (eq? (car type) 'union))
     `(union ,@(map (lambda (field)
                      (if (and (pair? field) (>= (length field) 2))
                          `(,(translate-ffi-type (car field)) ,(cadr field))
                          field))
                    (cdr type))))
    ((and (pair? type) (eq? (car type) 'array))
     `(array ,(translate-ffi-type (cadr type)) ,(caddr type)))
    (else
     (case type
       ((void) 'void)
       ((int) 'int)
       ((uint) 'uint)
       ((long) 'long)
       ((ulong) 'ulong)
       ((size_t) 'ulong)
       ((char) 'char)
       ((uchar) 'uchar)
       ((float) 'float)
       ((double) 'double)
       ((string) 'string)
       ((pointer) 'pointer)
       ((int8) 'int8)
       ((int16) 'int16)
       ((int32) 'int32)
       ((int64) 'int64)
       ((uint8) 'uint8)
       ((uint16) 'uint16)
       ((uint32) 'uint32)
       ((uint64) 'uint64)
       (else type)))))

;; --- Structs, Unions, Enums Declarators ---
(define-macro (define-c-struct name . fields)
  `(ffi-typedef ',name
                '(struct ,@(map (lambda (f)
                                  `(,(translate-ffi-type (cadr f)) ,(car f)))
                                fields))))

(define-macro (define-c-union name . fields)
  `(ffi-typedef ',name
                '(union ,@(map (lambda (f)
                                 `(,(translate-ffi-type (cadr f)) ,(car f)))
                               fields))))

(define-macro (define-c-enum name . variants)
  (let ((decls '()))
    (let loop ((vars variants) (val 0))
      (if (not (null? vars))
          (let* ((var (car vars))
                 (sym (if (pair? var) (car var) var))
                 (actual-val (if (pair? var) (cadr var) val)))
            (let* ((sym-str (symbol->string sym))
                   (clean-str (if (char=? (string-ref sym-str 0) #\:)
                                  (substring sym-str 1)
                                  sym-str))
                   (clean-sym (string->symbol clean-str)))
              (set! decls (cons `(define ,clean-sym ,actual-val) decls))
              (loop (cdr vars) (+ actual-val 1))))))
    `(begin ,@(reverse decls))))

;; --- FFI Routine Importer ---
(define-macro (c-import scheme-name lib-handle c-name ret-type arg-types . nfixed)
  (let ((translated-ret (translate-ffi-type ret-type))
        (translated-args (map translate-ffi-type arg-types))
        (func-ptr (gensym)))
    `(begin
       (define ,func-ptr (ffi-sym ,lib-handle ,c-name))
       (define (,scheme-name . args)
         ,(if (null? nfixed)
              `(ffi-call ,func-ptr ',translated-ret ',translated-args args)
              `(ffi-call ,func-ptr ',translated-ret ',translated-args args ,(car nfixed)))))))

;; --- Low-Level Memory Helper Functions (from libc) ---
(c-import internal-basic-c-memcpy-raw internal-basic-libc "memcpy" pointer (pointer pointer ulong))
(c-import internal-basic-c-memset-raw internal-basic-libc "memset" pointer (pointer int ulong))
(c-import bc.malloc                   internal-basic-libc "malloc" pointer (ulong))
(c-import bc.free                     internal-basic-libc "free"   void    (pointer))
(c-import bc.realloc                  internal-basic-libc "realloc" pointer (pointer ulong))

(define (bc.calloc nmemb size)
  (lext-calloc nmemb size))

(define (internal-basic-c-memcpy dest src byte-count)
  (internal-basic-c-memcpy-raw (if (internal-basic-tptr? dest) (internal-basic-tptr-ptr dest) dest)
                               (if (internal-basic-tptr? src) (internal-basic-tptr-ptr src) src)
                               byte-count))

(define (internal-basic-c-memset dest byte-value byte-count)
  (internal-basic-c-memset-raw (if (internal-basic-tptr? dest) (internal-basic-tptr-ptr dest) dest)
                               byte-value
                               byte-count))

(define (internal-basic-ptr+ tp offset)
  (if (internal-basic-tptr? tp)
      (internal-basic-tptr (internal-basic-tptr-type tp) (integer->c-pointer (+ (c-pointer->integer (internal-basic-tptr-ptr tp)) offset)))
      (integer->c-pointer (+ (c-pointer->integer tp) offset))))

;; --- Memory Math and Layout Metadata ---
(define (internal-basic-resolve-type type)
  (if (symbol? type)
      (let ((pair (assoc type *ffi-types*)))
        (if pair (cdr pair) type))
      type))

(define (internal-basic-c-align-of type)
  (ffi-align (translate-ffi-type type)))

(define (internal-basic-c-align-to offset alignment)
  (if (= alignment 0)
      offset
      (let ((rem (modulo offset alignment)))
        (if (= rem 0)
            offset
            (+ offset (- alignment rem))))))

(define (c-size type)
  (ffi-size (translate-ffi-type type)))

;; --- Ultimate Pointer Navigator (@) and Mutator (c-set!) ---
(define (internal-basic-struct-field-type struct-name field-name)
  (let ((type-desc (internal-basic-resolve-type struct-name)))
    (if (and (pair? type-desc) (eq? (car type-desc) 'struct))
        (let loop ((fields (cdr type-desc)))
          (if (not (null? fields))
              (let ((f (car fields)))
                (if (and (pair? f) (= (length f) 2))
                    (if (eq? (cadr f) field-name)
                        (car f)
                        (loop (cdr fields)))
                    (loop (cdr fields))))
              #f))
        #f)))

(define (internal-basic-struct-field-offset type-desc field-name)
  (let ((resolved (internal-basic-resolve-type type-desc)))
    (if (and (pair? resolved) (eq? (car resolved) 'struct))
        (let ((offset 0))
          (let loop ((fields (cdr resolved)))
            (if (not (null? fields))
                (let* ((f (car fields))
                       (ftype (car f))
                       (fname (cadr f))
                       (falign (internal-basic-c-align-of ftype)))
                  (set! offset (internal-basic-c-align-to offset falign))
                  (if (eq? fname field-name)
                      offset
                      (begin
                        (set! offset (+ offset (c-size ftype)))
                        (loop (cdr fields)))))
                #f)))
        #f)))

(define (internal-basic-struct-field-type-resolved resolved-type field-name)
  (let loop ((fields (cdr resolved-type)))
    (if (not (null? fields))
        (let ((f (car fields)))
          (if (eq? (cadr f) field-name)
              (car f)
              (loop (cdr fields))))
        #f)))

(define (internal-basic-resolve-path ptr type path)
  (if (null? path)
      (cons ptr type)
      (let* ((step (car path))
             (resolved-type (internal-basic-resolve-type type)))
        (cond
          ((number? step)
           (let* ((elem-type (if (and (pair? resolved-type) (eq? (car resolved-type) 'array))
                                 (cadr resolved-type)
                                 resolved-type))
                  (elem-size (c-size elem-type))
                  (offset (* step elem-size)))
             (internal-basic-resolve-path (internal-basic-ptr+ ptr offset) elem-type (cdr path))))
          ((and (pair? resolved-type) (eq? (car resolved-type) 'struct))
           (let ((offset (internal-basic-struct-field-offset resolved-type step))
                 (field-type (internal-basic-struct-field-type-resolved resolved-type step)))
             (if (not offset)
                 (error "Field not found in struct:" step type)
                 (internal-basic-resolve-path (internal-basic-ptr+ ptr offset) field-type (cdr path)))))
          ((and (pair? resolved-type) (eq? (car resolved-type) 'union))
           (let ((field-type (internal-basic-struct-field-type-resolved resolved-type step)))
             (if (not field-type)
                 (error "Field not found in union:" step type)
                 (internal-basic-resolve-path ptr field-type (cdr path)))))
          (else
           (error "Cannot navigate step on primitive type:" step type))))))

(define (bc.deref ptr type . path)
  (let* ((res (internal-basic-resolve-path ptr type path))
         (target-ptr (car res))
         (target-type (cdr res)))
    (ffi-deref target-ptr target-type)))

(define (bc.set! ptr type value . path)
  (let* ((res (internal-basic-resolve-path ptr type path))
         (target-ptr (car res))
         (target-type (cdr res)))
    (ffi-set! target-ptr target-type value)))

(define (internal-basic-string-split str delim)
  (let ((len (string-length str))
        (delim-char (string-ref delim 0)))
    (let loop ((i 0) (curr '()) (res '()))
      (if (= i len)
          (reverse (cons (list->string (reverse curr)) res))
          (let ((c (string-ref str i)))
            (if (char=? c delim-char)
                (loop (+ i 1) '() (cons (list->string (reverse curr)) res))
                (loop (+ i 1) (cons c curr) res)))))))

(define (internal-basic-string->number-or-symbol str)
  (let ((num (string->number str)))
    (if num num (string->symbol str))))

(define (internal-basic-parse-dot-symbol sym)
  (let* ((parts (internal-basic-string-split (symbol->string sym) ".")))
    (map internal-basic-string->number-or-symbol parts)))

(define (bc.addr tp . path)
  (let* ((ptr (internal-basic-tptr-ptr tp))
         (type (internal-basic-tptr-type tp))
         (res (internal-basic-resolve-path ptr type path))
         (target-ptr (car res))
         (target-type (cdr res)))
    (internal-basic-tptr target-type target-ptr)))

(define-macro (bc.@ obj-path)
  (let* ((parsed (internal-basic-parse-dot-symbol obj-path))
         (obj (car parsed))
         (path (cdr parsed)))
    `(bc.deref (internal-basic-tptr-ptr ,obj) (internal-basic-tptr-type ,obj) ,@(map (lambda (p) `',p) path))))

(define-macro (bc.= obj-path value)
  (let* ((parsed (internal-basic-parse-dot-symbol obj-path))
         (obj (car parsed))
         (path (cdr parsed)))
    `(bc.set! (internal-basic-tptr-ptr ,obj) (internal-basic-tptr-type ,obj) ,value ,@(map (lambda (p) `',p) path))))

(define-macro (bc.& obj-path)
  (let* ((parsed (internal-basic-parse-dot-symbol obj-path))
         (obj (car parsed))
         (path (cdr parsed)))
    `(bc.addr ,obj ,@(map (lambda (p) `',p) path))))

;; --- Memory Sandbox Allocation Macros ---
(define-macro (with-heap-alloc var-type . body)
  (let* ((var (car var-type))
         (type (cadr var-type))
         (count (if (= (length var-type) 3) (caddr var-type) #f))
         (size (if count `(* (c-size ',type) ,count) `(c-size ',type)))
         (ptr (gensym)))
    `(let ((,ptr (bc.malloc ,size)))
       (dynamic-wind
         (lambda () #f)
         (lambda ()
           (let ((,var (internal-basic-tptr ',type ,ptr)))
             ,@body))
         (lambda ()
           (bc.free ,ptr))))))

(define-macro (with-alloc . args)
  `(with-heap-alloc ,@args))

(define-macro (with-c-string var-str . body)
  (let ((var (car var-str))
        (str-expr (cadr var-str))
        (ptr (gensym)))
    `(let* ((,ptr (lext-string->c-string ,str-expr)))
       (dynamic-wind
         (lambda () #f)
         (lambda ()
           (let ((,var (internal-basic-tptr 'char ,ptr)))
             ,@body))
         (lambda ()
           (bc.free ,ptr))))))

(define-macro (with-c-array var-type-list . body)
  (let ((var (car var-type-list))
        (type (cadr var-type-list))
        (lst-expr (caddr var-type-list))
        (lst (gensym))
        (len (gensym))
        (ptr (gensym))
        (size (gensym)))
    `(let* ((,lst ,lst-expr)
            (,len (length ,lst))
            (,size (c-size ',type))
            (,ptr (bc.malloc (* ,len ,size))))
       (let loop ((curr ,lst) (i 0))
         (if (not (null? curr))
             (begin
               (ffi-set! (internal-basic-ptr+ ,ptr (* i ,size)) ',type (car curr))
               (loop (cdr curr) (+ i 1)))))
       (dynamic-wind
         (lambda () #f)
         (lambda ()
           (let ((,var (internal-basic-tptr ',type ,ptr)))
             ,@body))
         (lambda ()
           (bc.free ,ptr))))))

(define-macro (with-c-string-array var-lst . body)
  (let ((var (car var-lst))
        (lst-expr (cadr var-lst))
        (lst (gensym))
        (len (gensym))
        (ptr (gensym))
        (ptrs (gensym))
        (psize (gensym)))
    `(let* ((,lst ,lst-expr)
            (,len (length ,lst))
            (,psize (c-size 'pointer))
            (,ptr (bc.malloc (* (+ ,len 1) ,psize)))
            (,ptrs '()))
       (let loop ((curr ,lst) (i 0))
         (if (not (null? curr))
             (let* ((str (car curr))
                    (slen (string-length str))
                    (sptr (bc.malloc (+ slen 1))))
               (let str-loop ((j 0))
                 (if (< j slen)
                     (begin
                       (ffi-set! (internal-basic-ptr+ sptr j) 'char (char->integer (string-ref str j)))
                       (str-loop (+ j 1)))))
               (ffi-set! (internal-basic-ptr+ sptr slen) 'char 0)
               (ffi-set! (internal-basic-ptr+ ,ptr (* i ,psize)) 'pointer sptr)
               (set! ,ptrs (cons sptr ,ptrs))
               (loop (cdr curr) (+ i 1)))))
       (ffi-set! (internal-basic-ptr+ ,ptr (* ,len ,psize)) 'pointer ())
       (dynamic-wind
         (lambda () #f)
         (lambda ()
           (let ((,var (internal-basic-tptr 'pointer ,ptr)))
             ,@body))
         (lambda ()
           (for-each bc.free ,ptrs)
           (bc.free ,ptr))))))

;; --- Capturing Engine ---
;; Use native lext-capture-output builtin (item #2).
;; The old c-import lines for fflush/dup/dup2/close/tmpfile/fileno/rewind/fclose/fgetc
;; are no longer needed here since everything is done in C.

(define-macro (capture . body)
  `(lext-capture-output (lambda () ,@body)))


;; --- Common Helper Functions ---
(define (bc.null-ptr? x)
  (let ((ptr (if (internal-basic-tptr? x) (internal-basic-tptr-ptr x) x)))
    (or (null? ptr)
        (eq? ptr 0)
        (and (c-pointer? ptr) (= (c-pointer->integer ptr) 0)))))

(define (bc.null-ptr)
  ())

(define (c-string-from-ptr x)
  (let ((ptr (if (internal-basic-tptr? x) (internal-basic-tptr-ptr x) x)))
    (if (bc.null-ptr? ptr)
        ""
        (lext-c-string-from-ptr ptr))))

(define (c-string-array->list tp)
  (let ((ptr (if (internal-basic-tptr? tp) (internal-basic-tptr-ptr tp) tp)))
    (if (bc.null-ptr? ptr)
        '()
        (lext-c-string-array->list ptr))))

(define (bc.c-cast tp target-type)
  (if (internal-basic-tptr? tp)
      (internal-basic-tptr target-type (internal-basic-tptr-ptr tp))
      (internal-basic-tptr target-type tp)))

;; --- Tracked (bounds-checked) allocation helpers (#7) ---
(define (bc.malloc-tracked size) (lext-malloc-tracked size))
(define (bc.free-tracked ptr) (lext-free-tracked ptr))
(define (bc.bounds-check ptr size) (lext-bounds-check ptr size))

;; --- Loop Constructs (while, foreach, for) ---
(define-macro (bc.while cond . body)
  (let ((loop (gensym)))
    `(let ,loop ()
       (if ,cond
           (begin
             ,@body
             (,loop))))))

(define-macro (bc.foreach clause . body)
  (let ((var (car clause))
        (lst (cadr clause))
        (loop (gensym))
        (curr (gensym)))
    `(let ,loop ((,curr ,lst))
       (if (not (null? ,curr))
           (let ((,var (car ,curr)))
             ,@body
             (,loop (cdr ,curr)))))))

(define-macro (bc.for clause . body)
  (cond
    ;; Range style: (for (var start end))
    ((and (list? clause) (= (length clause) 3))
     (let ((var (car clause))
           (start (cadr clause))
           (end (caddr clause))
           (loop (gensym))
           (limit (gensym)))
       `(let ((,limit ,end))
          (let ,loop ((,var ,start))
            (if (< ,var ,limit)
                (begin
                  ,@body
                  (,loop (+ ,var 1))))))))
    ;; Range style with step: (for (var start end step))
    ((and (list? clause) (= (length clause) 4))
     (let ((var (car clause))
           (start (cadr clause))
           (end (caddr clause))
           (step (cadddr clause))
           (loop (gensym))
           (limit (gensym))
           (incr (gensym)))
       `(let ((,limit ,end)
              (,incr ,step))
          (let ,loop ((,var ,start))
            (if (if (> ,incr 0) (< ,var ,limit) (> ,var ,limit))
                (begin
                  ,@body
                  (,loop (+ ,var ,incr))))))))
    (else
     (error "invalid for syntax" clause))))

;; --- Namespace Stripper & Importer Utilities ---
(define *protected-symbols* '(set! = + - * / < > <= >=))

(define-macro (open-namespace prefix)
  (let ((st (symbol-table))
        (prefix-len (string-length prefix))
        (bindings '()))
    (for-each
      (lambda (sym)
        (let ((sym-str (symbol->string sym)))
          (if (and (> (string-length sym-str) prefix-len)
                   (string=? (substring sym-str 0 prefix-len) prefix)
                   (defined? sym))
              (let ((new-sym (string->symbol (substring sym-str prefix-len))))
                (unless (member new-sym *protected-symbols*)
                  (set! bindings (cons `(define ,new-sym ,sym) bindings)))))))
      st)
    `(begin ,@(reverse bindings))))

(define-macro (use-namespace prefix . body)
  (let ((st (symbol-table))
        (prefix-len (string-length prefix))
        (bindings '()))
    (for-each
      (lambda (sym)
        (let ((sym-str (symbol->string sym)))
          (if (and (> (string-length sym-str) prefix-len)
                   (string=? (substring sym-str 0 prefix-len) prefix)
                   (defined? sym))
              (let ((new-sym (string->symbol (substring sym-str prefix-len))))
                (unless (member new-sym *protected-symbols*)
                  (set! bindings (cons `',new-sym (cons sym bindings))))))))
      st)
    `(with-let (sublet (curlet) ,@bindings)
       ,@body)))


