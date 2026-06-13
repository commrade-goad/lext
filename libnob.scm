(define libnob (ffi-open #f))

(define-macro (c-import scheme-name lib-handle c-name ret-type arg-types . nfixed)
  (let ((func-ptr (gensym)))
    `(begin
       (define ,func-ptr (ffi-sym ,lib-handle ,c-name))
       (define (,scheme-name . args)
         ,(if (null? nfixed)
              `(ffi-call ,func-ptr ',ret-type ',arg-types args)
              `(ffi-call ,func-ptr ',ret-type ',arg-types args ,(car nfixed)))))))

;; Libc memory management
(define libc (ffi-open #f))
(c-import malloc libc "malloc" pointer (int))
(c-import free   libc "free"   void    (pointer))
(c-import realloc libc "realloc" pointer (pointer int))

;; Pointer Arithmetic Helpers
(define (nob.c-pointer->integer ptr)
  (c-pointer->integer ptr))

(define (nob.integer->c-pointer addr)
  (integer->c-pointer addr))

(define (nob.pointer-add ptr offset)
  (integer->c-pointer (+ (c-pointer->integer ptr) offset)))

;; C Struct Type Registrations
(ffi-typedef 'Nob_Cmd
             '(struct (pointer items)
                      (ulong count)
                      (ulong capacity)))

(ffi-typedef 'Nob_Procs
             '(struct (pointer items)
                      (ulong count)
                      (ulong capacity)))

(ffi-typedef 'Nob_Cmd_Opt
             '(struct (pointer async)
                      (ulong max_procs)
                      (uchar dont_reset)
                      (string stdin_path)
                      (string stdout_path)
                      (string stderr_path)))

(ffi-typedef 'Nob_String_Builder
             '(struct (pointer items)
                      (ulong count)
                      (ulong capacity)))

(ffi-typedef 'Nob_File_Paths
             '(struct (pointer items)
                      (ulong count)
                      (ulong capacity)))

(ffi-typedef 'Nob_Dynamic_Array
             '(struct (pointer items)
                      (ulong count)
                      (ulong capacity)))

(ffi-typedef 'Nob_String_View
             '(struct (ulong count)
                      (pointer data)))

(ffi-typedef 'Nob_Pipe
             '(struct (int read)
                      (int write)))

(ffi-typedef 'Nob_Chain
             '(struct (int fdin)
                      (Nob_Cmd cmd)
                      (uchar err2out)))

(ffi-typedef 'Nob_Chain_Begin_Opt
             '(struct (string stdin_path)))

(ffi-typedef 'Nob_Chain_Cmd_Opt
             '(struct (uchar err2out)
                      (uchar dont_reset)))

(ffi-typedef 'Nob_Chain_End_Opt
             '(struct (pointer async)
                      (ulong max_procs)
                      (string stdout_path)
                      (string stderr_path)))

(ffi-typedef 'Nob_Dir_Entry
             '(struct (pointer name)
                      (uchar error)
                      (pointer posix_dir)
                      (pointer posix_ent)))

(ffi-typedef 'Nob_Cmd_Redirect
             '(struct (pointer fdin)
                      (pointer fdout)
                      (pointer fderr)))

;; C Function Imports (prefixed with internal-nob-)
(c-import internal-nob-cmd-append               libnob "nob__cmd_append"                 void    (pointer ulong pointer))
(c-import internal-nob-cmd-run-opt              libnob "nob_cmd_run_opt"                 int     (pointer Nob_Cmd_Opt))
(c-import internal-nob-needs-rebuild1           libnob "nob_needs_rebuild1"              int     (string string))
(c-import internal-nob-delete-file               libnob "nob_delete_file"                 int     (string))
(c-import internal-nob-mkdir-if-nexist           libnob "nob_mkdir_if_not_exists"         int     (string))
(c-import internal-nob-file-exists               libnob "nob_file_exists"                 int     (string))
(c-import internal-nob-copy-file                 libnob "nob_copy_file"                   int     (string string))
(c-import internal-nob-copy-dir-recursively      libnob "nob_copy_directory_recursively"  int     (string string))
(c-import internal-nob-read-entire-file          libnob "nob_read_entire_file"            int     (string pointer))
(c-import internal-nob-write-entire-file         libnob "nob_write_entire_file"           int     (string pointer ulong))
(c-import internal-nob-get-file-type             libnob "nob_get_file_type"               int     (string))
(c-import internal-nob-rename                    libnob "nob_rename"                      int     (string string))
(c-import internal-nob-path-name                 libnob "nob_path_name"                   string  (string))
(c-import internal-nob-get-current-dir-temp      libnob "nob_get_current_dir_temp"        string  ())
(c-import internal-nob-set-current-dir           libnob "nob_set_current_dir"             int     (string))
(c-import internal-nob-temp-dir-name             libnob "nob_temp_dir_name"               string  (string))
(c-import internal-nob-temp-file-name             libnob "nob_temp_file_name"               string  (string))
(c-import internal-nob-temp-file-ext              libnob "nob_temp_file_ext"                string  (string))
(c-import internal-nob-temp-running-exe-path     libnob "nob_temp_running_executable_path" string  ())
(c-import internal-nob-nanos-since-epoch         libnob "nob_nanos_since_unspecified_epoch" uint64 ())
(c-import internal-nob-nprocs                    libnob "nob_nprocs"                      int     ())
(c-import internal-nob-read-entire-dir           libnob "nob_read_entire_dir"             int     (string pointer))
(c-import internal-nob-sb-pad-align              libnob "nob_sb_pad_align"                void    (pointer ulong))
(c-import internal-nob-proc-wait                 libnob "nob_proc_wait"                   int     (int))
(c-import internal-nob-procs-wait                libnob "nob_procs_wait"                  int     (Nob_Procs))
(c-import internal-nob-procs-flush               libnob "nob_procs_flush"                 int     (pointer))
(c-import internal-nob-pipe-create               libnob "nob_pipe_create"                 int     (pointer))
(c-import internal-nob-chain-begin-opt           libnob "nob_chain_begin_opt"             int     (pointer Nob_Chain_Begin_Opt))
(c-import internal-nob-chain-cmd-opt             libnob "nob_chain_cmd_opt"               int     (pointer pointer Nob_Chain_Cmd_Opt))
(c-import internal-nob-chain-end-opt             libnob "nob_chain_end_opt"               int     (pointer Nob_Chain_End_Opt))
(c-import internal-nob-temp-reset                libnob "nob_temp_reset"                  void    ())
(c-import internal-nob-temp-save                 libnob "nob_temp_save"                   ulong   ())
(c-import internal-nob-temp-rewind               libnob "nob_temp_rewind"                 void    (ulong))
(c-import internal-nob-temp-strdup               libnob "nob_temp_strdup"                 string  (string))
(c-import internal-nob-temp-strndup              libnob "nob_temp_strndup"                string  (string ulong))
(c-import internal-nob-temp-alloc                libnob "nob_temp_alloc"                  pointer (ulong))
(c-import internal-nob-log                       libnob "nob_log"                         void    (int string string) 2)

;; Additional C imports for 100% header parity
(c-import internal-nob-cmd-render                libnob "nob_cmd_render"                  void    (Nob_Cmd pointer))
(c-import internal-nob-cmd-run-async             libnob "nob_cmd_run_async"               int     (Nob_Cmd))
(c-import internal-nob-cmd-run-async-and-reset   libnob "nob_cmd_run_async_and_reset"     int     (pointer))
(c-import internal-nob-cmd-run-async-redirect    libnob "nob_cmd_run_async_redirect"      int     (Nob_Cmd Nob_Cmd_Redirect))
(c-import internal-nob-cmd-run-async-redirect-and-reset libnob "nob_cmd_run_async_redirect_and_reset" int (pointer Nob_Cmd_Redirect))
(c-import internal-nob-cmd-run-sync              libnob "nob_cmd_run_sync"                int     (Nob_Cmd))
(c-import internal-nob-cmd-run-sync-and-reset    libnob "nob_cmd_run_sync_and_reset"      int     (pointer))
(c-import internal-nob-cmd-run-sync-redirect     libnob "nob_cmd_run_sync_redirect"       int     (Nob_Cmd Nob_Cmd_Redirect))
(c-import internal-nob-cmd-run-sync-redirect-and-reset libnob "nob_cmd_run_sync_redirect_and_reset" int (pointer Nob_Cmd_Redirect))

(c-import internal-nob-fd-open-for-read          libnob "nob_fd_open_for_read"            int     (string))
(c-import internal-nob-fd-open-for-write         libnob "nob_fd_open_for_write"           int     (string))
(c-import internal-nob-fd-close                  libnob "nob_fd_close"                    void    (int))
(c-import internal-nob-procs-wait-and-reset      libnob "nob_procs_wait_and_reset"        int     (pointer))
(c-import internal-nob-procs-append-with-flush   libnob "nob_procs_append_with_flush"     int     (pointer int ulong))


;; Directory Entry Functions
(c-import internal-nob-dir-entry-open            libnob "nob_dir_entry_open"              int     (string pointer))
(c-import internal-nob-dir-entry-next            libnob "nob_dir_entry_next"              int     (pointer))
(c-import internal-nob-dir-entry-close           libnob "nob_dir_entry_close"             void    (Nob_Dir_Entry))

;; String View Functions
(c-import internal-nob-sv-from-cstr              libnob "nob_sv_from_cstr"                Nob_String_View (string))
(c-import internal-nob-sv-from-parts             libnob "nob_sv_from_parts"               Nob_String_View (pointer ulong))
(c-import internal-nob-sv-trim                   libnob "nob_sv_trim"                     Nob_String_View (Nob_String_View))
(c-import internal-nob-sv-trim-left              libnob "nob_sv_trim_left"                Nob_String_View (Nob_String_View))
(c-import internal-nob-sv-trim-right             libnob "nob_sv_trim_right"               Nob_String_View (Nob_String_View))
(c-import internal-nob-sv-chop-left              libnob "nob_sv_chop_left"                Nob_String_View (pointer ulong))
(c-import internal-nob-sv-chop-right             libnob "nob_sv_chop_right"               Nob_String_View (pointer ulong))
(c-import internal-nob-sv-chop-by-delim          libnob "nob_sv_chop_by_delim"            Nob_String_View (pointer char))
(c-import internal-nob-sv-chop-prefix            libnob "nob_sv_chop_prefix"              int     (pointer Nob_String_View))
(c-import internal-nob-sv-chop-suffix            libnob "nob_sv_chop_suffix"              int     (pointer Nob_String_View))
(c-import internal-nob-sv-eq                     libnob "nob_sv_eq"                       int     (Nob_String_View Nob_String_View))
(c-import internal-nob-sv-starts-with            libnob "nob_sv_starts_with"              int     (Nob_String_View Nob_String_View))
(c-import internal-nob-sv-ends-with              libnob "nob_sv_ends_with"                int     (Nob_String_View Nob_String_View))
(c-import internal-nob-sv-ends-with-cstr         libnob "nob_sv_ends_with_cstr"           int     (Nob_String_View string))

;; --- Scheme Wrapper Functions (prefixed with nob.) ---

;; Global variables
(define nob.minimal-log-level-ptr (ffi-sym libnob "nob_minimal_log_level"))

(define (nob.minimal-log-level)
  (ffi-deref nob.minimal-log-level-ptr 'int))

(define (nob.minimal-log-level-set! val)
  (ffi-set! nob.minimal-log-level-ptr 'int val))

;; nob.log: format-based logging
(define (nob.log level fmt . args)
  (internal-nob-log level "%s" (apply format #f fmt args)))

;; nob.cmd-new: allocate and initialize a new Nob_Cmd
(define (nob.cmd-new)
  (let ((ptr (malloc 24)))
    (ffi-set! ptr 'Nob_Cmd '((items . ()) (count . 0) (capacity . 0)))
    ptr))

;; nob.cmd-append: append one or more arguments to a Nob_Cmd
(define (nob.cmd-append cmd . args)
  (for-each
    (lambda (arg)
      (let ((str-ptr-buf (malloc 8)))
        (ffi-set! str-ptr-buf 'pointer arg)
        (internal-nob-cmd-append cmd 1 str-ptr-buf)
        (free str-ptr-buf)))
    args))

;; nob.cmd-free: free a Nob_Cmd and its internal items buffer
(define (nob.cmd-free cmd)
  (if (and (not (null? cmd)) (not (eq? cmd 0)))
      (let* ((cmd-struct (ffi-deref cmd 'Nob_Cmd))
             (items-ptr (cdr (assoc 'items cmd-struct))))
        (if (not (null? items-ptr))
            (free items-ptr))
        (free cmd))))

;; nob.cmd-run: run a command with Scheme-style keyword arguments.
;; Supports passing either a Nob_Cmd pointer OR a list of strings directly.
(define* (nob.cmd-run cmd-or-args
                      (async '())
                      (max-procs 0)
                      (dont-reset #f)
                      (stdin-path '())
                      (stdout-path '())
                      (stderr-path '()))
  (let* ((is-list (list? cmd-or-args))
         (cmd (if is-list (nob.cmd-new) cmd-or-args)))
    (if is-list
        (apply nob.cmd-append cmd cmd-or-args))
    (let* ((opt-list `((async . ,async)
                       (max_procs . ,max-procs)
                       (dont_reset . ,(if dont-reset 1 0))
                       (stdin_path . ,stdin-path)
                       (stdout_path . ,stdout-path)
                       (stderr_path . ,stderr-path)))
           (ok (not (= (internal-nob-cmd-run-opt cmd opt-list) 0))))
      (if is-list
          (nob.cmd-free cmd))
      ok)))

;; nob.delete-file: delete a file
(define (nob.delete-file path)
  (internal-nob-delete-file path))

;; nob.mkdir-if-not-exists: make directory if it doesn't exist
(define (nob.mkdir-if-not-exists path)
  (internal-nob-mkdir-if-nexist path))

;; nob.file-exists: check if file exists
(define (nob.file-exists path)
  (internal-nob-file-exists path))

;; nob.copy-file: copy file
(define (nob.copy-file src dst)
  (not (= (internal-nob-copy-file src dst) 0)))

;; nob.copy-directory-recursively: recursively copy directory
(define (nob.copy-directory-recursively src dst)
  (not (= (internal-nob-copy-dir-recursively src dst) 0)))

;; nob.read-entire-file: read file into string using string builder FFI
(define (nob.read-entire-file path)
  (let ((sb (malloc 24)))
    (ffi-set! sb 'Nob_String_Builder '((items . ()) (count . 0) (capacity . 0)))
    (let ((success (not (= (internal-nob-read-entire-file path sb) 0))))
      (if success
          (let* ((sb-struct (ffi-deref sb 'Nob_String_Builder))
                 (items-ptr (cdr (assoc 'items sb-struct)))
                 (count (cdr (assoc 'count sb-struct))))
            (if (> count 0)
                (let* ((bytes (ffi-deref items-ptr (list 'array 'char count)))
                       (chars (map integer->char bytes))
                       (str (list->string chars)))
                  (if (not (null? items-ptr)) (free items-ptr))
                  (free sb)
                  str)
                (begin
                  (free sb)
                  "")))
          (begin
            (free sb)
            #f)))))

;; nob.write-entire-file: write string data directly to file
(define (nob.write-entire-file path data)
  (not (= (internal-nob-write-entire-file path data (string-length data)) 0)))

;; nob.get-file-type: returns symbol: 'regular, 'directory, 'symlink, or 'other
(define (nob.get-file-type path)
  (let ((t (internal-nob-get-file-type path)))
    (cond
      ((= t 0) 'regular)
      ((= t 1) 'directory)
      ((= t 2) 'symlink)
      (else 'other))))

;; nob.rename: rename file
(define (nob.rename old-path new-path)
  (not (= (internal-nob-rename old-path new-path) 0)))

;; nob.path-name: returns trailing name component
(define (nob.path-name path)
  (internal-nob-path-name path))

;; nob.get-current-dir-temp: returns path to current directory
(define (nob.get-current-dir-temp)
  (internal-nob-get-current-dir-temp))

;; nob.set-current-dir: set current directory
(define (nob.set-current-dir path)
  (not (= (internal-nob-set-current-dir path) 0)))

;; nob.temp-dir-name: returns directory part of path
(define (nob.temp-dir-name path)
  (internal-nob-temp-dir-name path))

;; nob.temp-file-name: returns file name part of path
(define (nob.temp-file-name path)
  (internal-nob-temp-file-name path))

;; nob.temp-file-ext: returns extension part of path
(define (nob.temp-file-ext path)
  (internal-nob-temp-file-ext path))

;; nob.temp-running-executable-path: returns running binary path
(define (nob.temp-running-executable-path)
  (internal-nob-temp-running-exe-path))

;; nob.nanos-since-unspecified-epoch: returns time in nanoseconds
(define (nob.nanos-since-unspecified-epoch)
  (internal-nob-nanos-since-epoch))

;; nob.nprocs: returns CPU core count
(define (nob.nprocs)
  (internal-nob-nprocs))

;; nob.read-entire-dir: read directory and return list of file names
(define (nob.read-entire-dir parent)
  (let ((children (malloc 24)))
    (ffi-set! children 'Nob_File_Paths '((items . ()) (count . 0) (capacity . 0)))
    (let ((success (not (= (internal-nob-read-entire-dir parent children) 0))))
      (if success
          (let* ((children-struct (ffi-deref children 'Nob_Dynamic_Array))
                 (items-ptr (cdr (assoc 'items children-struct)))
                 (count (cdr (assoc 'count children-struct)))
                 (result '()))
            (let loop ((i 0))
              (if (< i count)
                  (begin
                    (let* ((str-ptr (nob.pointer-add items-ptr (* i 8)))
                           (str (ffi-deref str-ptr 'string)))
                      (set! result (cons str result))
                      (loop (+ i 1))))))
            (if (and (not (null? items-ptr)) (not (eq? items-ptr 0))) (free items-ptr))
            (free children)
            (reverse result))
          (begin
            (free children)
            #f)))))

;; nob.sb-pad-align: pad string builder to alignment size boundary
(define (nob.sb-pad-align sb size)
  (internal-nob-sb-pad-align sb size))

;; nob.proc-wait: wait for a process
(define (nob.proc-wait proc)
  (not (= (internal-nob-proc-wait proc) 0)))

;; nob.procs-wait: wait for dynamic array of processes (by value)
(define (nob.procs-wait procs)
  (not (= (internal-nob-procs-wait procs) 0)))

;; nob.procs-flush: wait for all processes in procs (by pointer) and empty list
(define (nob.procs-flush procs-ptr)
  (not (= (internal-nob-procs-flush procs-ptr) 0)))

;; nob.pipe-create: create a pipe
(define (nob.pipe-create pipe-ptr)
  (not (= (internal-nob-pipe-create pipe-ptr) 0)))

;; Chain API wrappers
(define* (nob.chain-begin chain (stdin-path '()))
  (not (= (internal-nob-chain-begin-opt chain `((stdin_path . ,stdin-path))) 0)))

(define* (nob.chain-cmd chain cmd (err2out #f) (dont-reset #f))
  (not (= (internal-nob-chain-cmd-opt chain cmd `((err2out . ,(if err2out 1 0)) (dont_reset . ,(if dont-reset 1 0)))) 0)))

(define* (nob.chain-end chain (async '()) (max-procs 0) (stdout-path '()) (stderr-path '()))
  (not (= (internal-nob-chain-end-opt chain `((async . ,async) (max_procs . ,max-procs) (stdout_path . ,stdout-path) (stderr_path . ,stderr-path))) 0)))

;; Directory entry iterators
(define (nob.dir-entry-open dir-path dir-ptr)
  (not (= (internal-nob-dir-entry-open dir-path dir-ptr) 0)))

(define (nob.dir-entry-next dir-ptr)
  (not (= (internal-nob-dir-entry-next dir-ptr) 0)))

(define (nob.dir-entry-close dir-val)
  (internal-nob-dir-entry-close dir-val))

;; String View Wrappers
(define (nob.sv-from-cstr cstr)
  (internal-nob-sv-from-cstr cstr))

(define (nob.sv-from-parts data count)
  (internal-nob-sv-from-parts data count))

(define (nob.sv-trim sv)
  (internal-nob-sv-trim sv))

(define (nob.sv-trim-left sv)
  (internal-nob-sv-trim-left sv))

(define (nob.sv-trim-right sv)
  (internal-nob-sv-trim-right sv))

(define (nob.sv-chop-left sv-ptr n)
  (internal-nob-sv-chop-left sv-ptr n))

(define (nob.sv-chop-right sv-ptr n)
  (internal-nob-sv-chop-right sv-ptr n))

(define (nob.sv-chop-by-delim sv-ptr delim)
  (internal-nob-sv-chop-by-delim sv-ptr (char->integer delim)))

(define (nob.sv-chop-prefix sv-ptr prefix)
  (not (= (internal-nob-sv-chop-prefix sv-ptr prefix) 0)))

(define (nob.sv-chop-suffix sv-ptr suffix)
  (not (= (internal-nob-sv-chop-suffix sv-ptr suffix) 0)))

(define (nob.sv-eq a b)
  (not (= (internal-nob-sv-eq a b) 0)))

(define (nob.sv-starts-with sv prefix)
  (not (= (internal-nob-sv-starts-with sv prefix) 0)))

(define (nob.sv-ends-with sv suffix)
  (not (= (internal-nob-sv-ends-with sv suffix) 0)))

(define (nob.sv-ends-with-cstr sv cstr)
  (not (= (internal-nob-sv-ends-with-cstr sv cstr) 0)))

(define (nob.sv->string sv)
  (let* ((data (cdr (assoc 'data sv)))
         (count (cdr (assoc 'count sv))))
    (if (> count 0)
        (let* ((bytes (ffi-deref data (list 'array 'char count)))
               (chars (map integer->char bytes)))
          (list->string chars))
        "")))

;; Temp Allocator wrappers
(define (nob.temp-reset)
  (internal-nob-temp-reset))

(define (nob.temp-save)
  (internal-nob-temp-save))

(define (nob.temp-rewind checkpoint)
  (internal-nob-temp-rewind checkpoint))

(define (nob.temp-strdup cstr)
  (internal-nob-temp-strdup cstr))

(define (nob.temp-strndup cstr size)
  (internal-nob-temp-strndup cstr size))

(define (nob.temp-alloc size)
  (internal-nob-temp-alloc size))

;; --- Dynamic Array Generic Scheme-Macro implementation ---

(define (nob.da-get da)
  (ffi-deref da 'Nob_Dynamic_Array))

(define (nob.da-set! da val)
  (ffi-set! da 'Nob_Dynamic_Array val))

(define (nob.da-update! da items count capacity)
  (nob.da-set! da `((items . ,items) (count . ,count) (capacity . ,capacity))))

(define (nob.da-items da) (cdr (assoc 'items (nob.da-get da))))
(define (nob.da-count da) (cdr (assoc 'count (nob.da-get da))))
(define (nob.da-capacity da) (cdr (assoc 'capacity (nob.da-get da))))

(define (nob.da-free da)
  (let* ((da-struct (nob.da-get da))
         (items (cdr (assoc 'items da-struct))))
    (if (and (not (null? items)) (not (eq? items 0)))
        (free items))
    (nob.da-update! da '() 0 0)))

(define (nob.da-reserve da expected-capacity element-size)
  (let* ((da-struct (nob.da-get da))
         (items (cdr (assoc 'items da-struct)))
         (count (cdr (assoc 'count da-struct)))
         (capacity (cdr (assoc 'capacity da-struct))))
    (if (> expected-capacity capacity)
        (let* ((new-capacity (if (= capacity 0) 256 capacity))
               (new-capacity (let loop ((cap new-capacity))
                               (if (> expected-capacity cap)
                                   (loop (* cap 2))
                                   cap)))
               (new-items (realloc items (* new-capacity element-size))))
          (nob.da-update! da new-items count new-capacity)))))

(define-macro (nob.da-append da item type)
  (let ((da-var (gensym))
        (da-struct (gensym))
        (items (gensym))
        (count (gensym))
        (capacity (gensym))
        (item-ptr (gensym))
        (item-size (gensym)))
    `(let* ((,da-var ,da)
            (,da-struct (nob.da-get ,da-var))
            (,items (cdr (assoc 'items ,da-struct)))
            (,count (cdr (assoc 'count ,da-struct)))
            (,capacity (cdr (assoc 'capacity ,da-struct)))
            (,item-size (cond
                          ((eq? ',type 'pointer) 8)
                          ((eq? ',type 'ulong) 8)
                          ((eq? ',type 'int) 4)
                          ((eq? ',type 'uchar) 1)
                          (else (error "Unsupported type in da-append")))))
       (nob.da-reserve ,da-var (+ ,count 1) ,item-size)
       (let* ((,da-struct (nob.da-get ,da-var))
              (,items (cdr (assoc 'items ,da-struct)))
              (,item-ptr (nob.pointer-add ,items (* ,count ,item-size))))
         (ffi-set! ,item-ptr ',type ,item)
         (nob.da-update! ,da-var ,items (+ ,count 1) (cdr (assoc 'capacity ,da-struct)))))))

(define (nob.da-pop da type)
  (let* ((da-struct (nob.da-get da))
         (items (cdr (assoc 'items da-struct)))
         (count (cdr (assoc 'count da-struct)))
         (capacity (cdr (assoc 'capacity da-struct)) )
         (item-size (cond
                      ((eq? type 'pointer) 8)
                      ((eq? type 'ulong) 8)
                      ((eq? type 'int) 4)
                      ((eq? type 'uchar) 1)
                      (else (error "Unsupported type")))))
    (if (> count 0)
        (let* ((new-count (- count 1))
               (item-ptr (nob.pointer-add items (* new-count item-size)))
               (val (ffi-deref item-ptr type)))
          (nob.da-update! da items new-count capacity)
          val)
        (error "da-pop: empty array"))))

(define (nob.da-first da type)
  (let* ((da-struct (nob.da-get da))
         (items (cdr (assoc 'items da-struct)))
         (count (cdr (assoc 'count da-struct))))
    (if (> count 0)
        (ffi-deref items type)
        (error "da-first: empty array"))))

(define (nob.da-last da type)
  (let* ((da-struct (nob.da-get da))
         (items (cdr (assoc 'items da-struct)))
         (count (cdr (assoc 'count da-struct)))
         (item-size (cond
                      ((eq? type 'pointer) 8)
                      ((eq? type 'ulong) 8)
                      ((eq? type 'int) 4)
                      ((eq? type 'uchar) 1)
                      (else (error "Unsupported type")))))
    (if (> count 0)
        (let* ((item-ptr (nob.pointer-add items (* (- count 1) item-size))))
          (ffi-deref item-ptr type))
        (error "da-last: empty array"))))

;; nob.shift macro for Scheme lists
(define-macro (nob.shift lst)
  `(if (not (null? ,lst))
       (let ((first (car ,lst)))
         (set! ,lst (cdr ,lst))
         first)
       #f))

;; nob.needs-rebuild: check if output needs rebuilding relative to one or more input files
(define (nob.needs-rebuild output inputs)
  (if (string? inputs)
      (internal-nob-needs-rebuild1 output inputs)
      (let loop ((curr inputs))
        (cond
          ((null? curr) 0)
          ((= (internal-nob-needs-rebuild1 output (car curr)) 1) 1)
          (else (loop (cdr curr)))))))

(define (nob.needs-rebuild? output input)
  (not (= (nob.needs-rebuild output input) 0)))

;; --- File Descriptor Wrappers ---
(define (nob.fd-open-for-read path)
  (internal-nob-fd-open-for-read path))

(define (nob.fd-open-for-write path)
  (internal-nob-fd-open-for-write path))

(define (nob.fd-close fd)
  (internal-nob-fd-close fd))

;; --- Procs Wait / Append ---
(define (nob.procs-wait-and-reset procs-ptr)
  (not (= (internal-nob-procs-wait-and-reset procs-ptr) 0)))

(define (nob.procs-append-with-flush procs-ptr proc max-procs-count)
  (not (= (internal-nob-procs-append-with-flush procs-ptr proc max-procs-count) 0)))

;; --- Command Render and Run Wrappers ---
(define (nob.cmd-render cmd)
  (let ((sb (malloc 24)))
    (ffi-set! sb 'Nob_String_Builder '((items . ()) (count . 0) (capacity . 0)))
    (internal-nob-cmd-render (if (c-pointer? cmd) (ffi-deref cmd 'Nob_Cmd) cmd) sb)
    (let* ((sb-struct (ffi-deref sb 'Nob_String_Builder))
           (items-ptr (cdr (assoc 'items sb-struct)))
           (count (cdr (assoc 'count sb-struct)))
           (str (if (> count 0)
                    (let* ((bytes (ffi-deref items-ptr (list 'array 'char count)))
                           (chars (map integer->char bytes)))
                      (list->string chars))
                    "")))
      (if (and (not (null? items-ptr)) (not (eq? items-ptr 0))) (free items-ptr))
      (free sb)
      str)))

(define (nob.cmd-run-async cmd)
  (internal-nob-cmd-run-async (if (c-pointer? cmd) (ffi-deref cmd 'Nob_Cmd) cmd)))

(define (nob.cmd-run-async-and-reset cmd-ptr)
  (internal-nob-cmd-run-async-and-reset cmd-ptr))

(define (nob.cmd-run-async-redirect cmd redirect)
  (internal-nob-cmd-run-async-redirect (if (c-pointer? cmd) (ffi-deref cmd 'Nob_Cmd) cmd) redirect))

(define (nob.cmd-run-async-redirect-and-reset cmd-ptr redirect)
  (internal-nob-cmd-run-async-redirect-and-reset cmd-ptr redirect))

(define (nob.cmd-run-sync cmd)
  (not (= (internal-nob-cmd-run-sync (if (c-pointer? cmd) (ffi-deref cmd 'Nob_Cmd) cmd)) 0)))

(define (nob.cmd-run-sync-and-reset cmd-ptr)
  (not (= (internal-nob-cmd-run-sync-and-reset cmd-ptr) 0)))

(define (nob.cmd-run-sync-redirect cmd redirect)
  (not (= (internal-nob-cmd-run-sync-redirect (if (c-pointer? cmd) (ffi-deref cmd 'Nob_Cmd) cmd) redirect) 0)))

(define (nob.cmd-run-sync-redirect-and-reset cmd-ptr redirect)
  (not (= (internal-nob-cmd-run-sync-redirect-and-reset cmd-ptr redirect) 0)))

;; --- Pure-Scheme Directory Walker ---
;; Equivalent of nob_walk_dir / nob_walk_dir_opt
(define* (nob.walk-dir root func (level 0))
  (let ((files (nob.read-entire-dir root)))
    (if files
        (let loop ((curr files))
          (if (not (null? curr))
              (let* ((file (car curr))
                     (path (if (or (string=? root ".") (string=? root "./"))
                               file
                               (string-append root "/" file))))
                (if (and (not (string=? file ".")) (not (string=? file "..")))
                    (let* ((type (nob.get-file-type path))
                           (continue (func path type level)))
                      (if continue
                          (begin
                            (if (eq? type 'directory)
                                (let ((sub-continue (nob.walk-dir path func (+ level 1))))
                                  (if sub-continue
                                      (loop (cdr curr))
                                      #f))
                                (loop (cdr curr))))
                          #f))
                    (loop (cdr curr))))
              #t))
        #f)))
