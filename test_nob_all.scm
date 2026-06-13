(load "libnob.scm")

;; 1. Test String Views
(display "=== Testing String Views ===\n")
(let* ((sv (nob.sv-from-cstr "   Hello String View!   "))
       (trimmed-sv (nob.sv-trim sv))
       (trimmed-str (nob.sv->string trimmed-sv)))
  (display (format #f "Original: ~S\n" (nob.sv->string sv)))
  (display (format #f "Trimmed: ~S\n" trimmed-str)))

;; 2. Test Directory Reading using FFI arrays
(display "\n=== Testing Directory Listing ===\n")
(let ((files (nob.read-entire-dir ".")))
  (display (format #f "Files in current dir: ~A\n" files)))

;; 3. Test Directory Walking (pure Scheme)
(display "\n=== Testing Directory Walking ===\n")
(nob.walk-dir "."
              (lambda (path type level)
                (display (format #f "Walked: ~A (~A) at level ~A\n" path type level))
                ;; Only walk first few files to not print too much, or we can just return #t to keep going.
                ;; Let's keep going but only print if level < 1 (non-recursive files)
                (if (< level 1)
                    #t
                    #f)))

;; 4. Test Command Rendering
(display "\n=== Testing Command Rendering ===\n")
(let ((cmd (nob.cmd-new)))
  (nob.cmd-append cmd "gcc" "-o" "my_binary" "main.c")
  (let ((rendered (nob.cmd-render cmd)))
    (display (format #f "Rendered command: ~S\n" rendered))
    (nob.cmd-free cmd)))

;; 5. Test File Descriptors
(display "\n=== Testing File Descriptors ===\n")
(let ((fd (nob.fd-open-for-write "test_fd_output.txt")))
  (if (>= fd 0)
      (begin
        (display "Successfully opened file for writing.\n")
        (nob.fd-close fd)
        (nob.delete-file "test_fd_output.txt"))
      (display "Failed to open file.\n")))
