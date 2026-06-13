(load "libnob.scm")

(define build-dir "build")

(define (read-file-string filename)
  (let ((port (open-input-file filename)))
    (if port
        (let loop ((chars '()) (c (read-char port)))
          (if (eof-object? c)
              (begin
                (close-input-port port)
                (list->string (reverse chars)))
              (if (or (char-whitespace? c) (char=? c #\newline) (char=? c #\return))
                  (loop chars (read-char port))
                  (loop (cons c chars) (read-char port)))))
        "unknown")))

(define (split-string-by-spaces str)
  (let ((len (string-length str)))
    (let loop ((i 0) (word '()) (words '()))
      (if (>= i len)
          (if (null? word)
              (reverse words)
              (reverse (cons (list->string (reverse word)) words)))
          (let ((c (string-ref str i)))
            (if (char-whitespace? c)
                (if (null? word)
                    (loop (+ i 1) '() words)
                    (loop (+ i 1) '() (cons (list->string (reverse word)) words)))
                (loop (+ i 1) (cons c word) words)))))))

(define (get-git-hash)
  (let ((filepath (string-append build-dir "/" ".git_hash.txt")))
    (if (nob.cmd-run '("git" "rev-parse" "--short" "HEAD") :stdout-path filepath)
        (let ((hash (read-file-string filepath)))
          (nob.delete-file filepath)
          hash)
        "unknown")))

(define (get-pkg-config-cflags)
  (let ((filepath (string-append build-dir "/" ".ffi_cflags.txt")))
    (if (nob.cmd-run '("pkg-config" "--cflags" "libffi") :stdout-path filepath)
        (let ((cflags (read-file-string filepath)))
          (nob.delete-file filepath)
          cflags)
        "-I/usr/lib64/libffi/include")))

(define (get-pkg-config-libs)
  (let ((filepath (string-append build-dir "/" ".ffi_libs.txt")))
    (if (nob.cmd-run '("pkg-config" "--libs" "libffi") :stdout-path filepath)
        (let ((libs (read-file-string filepath)))
          (nob.delete-file filepath)
          libs)
        "-lffi")))

(define (append-builddir path)
  (string-append build-dir "/" path))

(define (build-static-nob)
  (if (or (nob.needs-rebuild? (append-builddir "libnob.o") "nob.c")
          (nob.needs-rebuild? (append-builddir "libnob.o") "nob.h"))
      (if (not (nob.cmd-run `("gcc" "-O2" "-fPIC" "-c" "nob.c" "-o" ,(append-builddir "libnob.o"))))
          (error "Failed to compile libnob.o"))
      (display "libnob.o is up to date.\n")))

;; --- Build Process ---

(display (format #f "\
# ~A self-builder script #
  Why use other language to compile your interpreter
           when you can metaprogram it :)\n\n" *script-name*))

(display "Starting Scheme-based build using libnob...\n")

(if (member "clean" argv string=?)
    (begin
      (display "Cleaning workspace...\n")
      (nob.delete-file (append-builddir "main.o"))
      (nob.delete-file (append-builddir "s7.o"))
      (nob.delete-file (append-builddir "libnob.o"))
      (nob.delete-file (append-builddir "lext"))
      (exit)))

(nob.mkdir-if-not-exists build-dir)

(define git-hash (get-git-hash))
(define ffi-cflags (get-pkg-config-cflags))
(define ffi-libs (get-pkg-config-libs))

(format #t "Git Hash: ~A\n" git-hash)
(format #t "libffi CFLAGS: ~A\n" ffi-cflags)
(format #t "libffi LIBS: ~A\n" ffi-libs)

;; 0. Build static nob
(build-static-nob)

;; 1. Compile s7.o
(if (or (nob.needs-rebuild? (append-builddir "s7.o") "s7/s7.c")
        (nob.needs-rebuild? (append-builddir "s7.o") "s7/s7.h"))
    (begin
      (display "Rebuilding s7.o...\n")
      (if (not (nob.cmd-run `("gcc" "-O2" "-march=native" "-Wall" "-c" "s7/s7.c" "-o" ,(append-builddir "s7.o"))))
          (error "Failed to compile s7.o")))
    (display "s7.o is up to date.\n"))

;; 2. Compile main.o
(if (nob.needs-rebuild? (append-builddir "main.o") "main.c")
    (begin
      (display "Rebuilding main.o...\n")
      (let ((args (append `("gcc" "-O2" "-march=native" "-Wall")
                          (split-string-by-spaces ffi-cflags)
                          `(,(format #f "-DHASHVER=\"~A\"" git-hash) "-c" "main.c" "-o" ,(append-builddir "main.o")))))
        (if (not (nob.cmd-run args))
            (error "Failed to compile main.o"))))
    (display "main.o is up to date.\n"))

;; 3. Link lext
(if (or (nob.needs-rebuild? (append-builddir "lext") (append-builddir "main.o"))
        (nob.needs-rebuild? (append-builddir "lext") (append-builddir "libnob.o"))
        (nob.needs-rebuild? (append-builddir "lext") (append-builddir "s7.o")))
    (begin
      (display "Linking lext...\n")
      (let ((args (append `("gcc" "-rdynamic" ,(append-builddir "main.o") ,(append-builddir "s7.o") ,(append-builddir "libnob.o") "-o" ,(append-builddir "lext") "-lm" "-ldl")
                          (split-string-by-spaces ffi-libs))))
        (if (not (nob.cmd-run args))
            (error "Failed to link lext"))))
    (display "lext is up to date.\n"))

(display "Build finished successfully!\n")
