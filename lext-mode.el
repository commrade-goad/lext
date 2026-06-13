;;; lext-mode.el --- Lext-Scheme editing mode    -*- lexical-binding: t; -*-

;; Author: Antigravity Pair Programmer
;; Keywords: languages, lisp, lext

;;; Commentary:

;; The major mode for editing Lext-Scheme files.
;; Standalone major mode cloned and modified from Emacs scheme.el,
;; adapted to support Lext FFI primitives, memory sandboxing, custom loops,
;; namespaces, Nob task-runner keywords, type highlighting, and built-in
;; rainbow delimiters.

;;; Code:

(require 'lisp-mode)
(eval-when-compile 'subr-x)

;; --- Rainbow Delimiters Faces ---
(defface lext-rainbow-depth-1 '((t (:foreground "#1e90ff"))) "Depth 1 paren face") ; dodger blue
(defface lext-rainbow-depth-2 '((t (:foreground "#ff8c00"))) "Depth 2 paren face") ; dark orange
(defface lext-rainbow-depth-3 '((t (:foreground "#ffd700"))) "Depth 3 paren face") ; gold
(defface lext-rainbow-depth-4 '((t (:foreground "#32cd32"))) "Depth 4 paren face") ; lime green
(defface lext-rainbow-depth-5 '((t (:foreground "#da70d6"))) "Depth 5 paren face") ; orchid
(defface lext-rainbow-depth-6 '((t (:foreground "#00ffff"))) "Depth 6 paren face") ; cyan
(defface lext-rainbow-depth-7 '((t (:foreground "#ff69b4"))) "Depth 7 paren face") ; hot pink
(defface lext-rainbow-depth-8 '((t (:foreground "#98fb98"))) "Depth 8 paren face") ; pale green
(defface lext-rainbow-depth-9 '((t (:foreground "#bc8f8f"))) "Depth 9 paren face") ; rosy brown

(defun lext-rainbow-highlight-delimiters (limit)
  "Function to scan and apply dynamic rainbow coloring to delimiters up to LIMIT."
  (while (re-search-forward "[][(){}]" limit t)
    (let ((state (syntax-ppss)))
      (when (not (or (nth 3 state) (nth 4 state)))
        (let* ((depth (nth 0 state))
               (char (char-before))
               (is-closing (member char '(?\) ?\] ?\})))
               (adj-depth (if is-closing (1+ depth) depth))
               (face (cond
                      ((<= adj-depth 1) 'lext-rainbow-depth-1)
                      ((= adj-depth 2) 'lext-rainbow-depth-2)
                      ((= adj-depth 3) 'lext-rainbow-depth-3)
                      ((= adj-depth 4) 'lext-rainbow-depth-4)
                      ((= adj-depth 5) 'lext-rainbow-depth-5)
                      ((= adj-depth 6) 'lext-rainbow-depth-6)
                      ((= adj-depth 7) 'lext-rainbow-depth-7)
                      ((= adj-depth 8) 'lext-rainbow-depth-8)
                      (t 'lext-rainbow-depth-9))))
          (put-text-property (match-beginning 0) (match-end 0) 'face face)))))
  nil)

(defvar lext-mode-syntax-table
  (let ((st (make-syntax-table))
        (i 0))
    ;; Symbol constituents
    (while (< i ?0)
      (modify-syntax-entry i "_   " st)
      (setq i (1+ i)))
    (setq i (1+ ?9))
    (while (< i ?A)
      (modify-syntax-entry i "_   " st)
      (setq i (1+ i)))
    (setq i (1+ ?Z))
    (while (< i ?a)
      (modify-syntax-entry i "_   " st)
      (setq i (1+ i)))
    (setq i (1+ ?z))
    (while (< i 128)
      (modify-syntax-entry i "_   " st)
      (setq i (1+ i)))

    ;; Whitespace
    (modify-syntax-entry ?\t "    " st)
    (modify-syntax-entry ?\n ">   " st)
    (modify-syntax-entry ?\f "    " st)
    (modify-syntax-entry ?\r "    " st)
    (modify-syntax-entry ?\s "    " st)

    ;; Delimiters
    (modify-syntax-entry ?\[ "(]  " st)
    (modify-syntax-entry ?\] ")[  " st)
    (modify-syntax-entry ?{ "(}  " st)
    (modify-syntax-entry ?} "){  " st)
    (modify-syntax-entry ?\| "\" 23bn" st)

    (modify-syntax-entry ?\( "()  " st)
    (modify-syntax-entry ?\) ")(  " st)
    (modify-syntax-entry ?\; "<"    st)
    (modify-syntax-entry ?\" "\"   " st)
    (modify-syntax-entry ?' "'   " st)
    (modify-syntax-entry ?` "'   " st)

    ;; Special characters
    (modify-syntax-entry ?, "'   " st)
    (modify-syntax-entry ?@ "'   " st)
    (modify-syntax-entry ?# "' 14" st)
    (modify-syntax-entry ?\\ "\\   " st)
    st))

(defvar lext-mode-abbrev-table nil)
(define-abbrev-table 'lext-mode-abbrev-table ())

(defvar lext-imenu-generic-expression
  `((nil
     ,(rx bol (zero-or-more space)
          "(define"
          (zero-or-one "*")
          (zero-or-one "-public")
          (one-or-more space)
          (zero-or-one "(")
          (group (one-or-more (or word (syntax symbol)))))
     1)
    ("Macros"
     ,(rx bol (zero-or-more space) "("
          (or "define-macro" "define-syntax" "define-syntax-rule")
          (one-or-more space)
          (zero-or-one "(")
          (group (one-or-more (or word (syntax symbol)))))
     1))
  "Imenu generic expression for Lext mode.  See `imenu-generic-expression'.")

(defun lext-mode-variables ()
  (set-syntax-table lext-mode-syntax-table)
  (setq local-abbrev-table lext-mode-abbrev-table)
  (setq-local paragraph-start (concat "$\\|" page-delimiter))
  (setq-local paragraph-separate paragraph-start)
  (setq-local paragraph-ignore-fill-prefix t)
  (setq-local fill-paragraph-function 'lisp-fill-paragraph)
  (setq-local adaptive-fill-mode nil)
  (setq-local indent-line-function 'lisp-indent-line)
  (setq-local parse-sexp-ignore-comments t)
  (setq-local outline-regexp ";;; \\|(....")
  (setq-local add-log-current-defun-function #'lisp-current-defun-name)
  (setq-local comment-start ";")
  (setq-local comment-add 1)
  (setq-local comment-start-skip ";+[ \t]*")
  (setq-local comment-use-syntax t)
  (setq-local comment-column 40)
  (setq-local lisp-indent-function 'lext-indent-function)
  (setq mode-line-process '("" lext-mode-line-process))
  (setq-local imenu-case-fold-search t)
  (setq-local imenu-generic-expression lext-imenu-generic-expression)
  (setq-local imenu-syntax-alist '(("+-*/.<>=?!$%_&~^:" . "w")))
  (setq-local syntax-propertize-function #'lext-syntax-propertize)
  (setq font-lock-defaults
        '((lext-font-lock-keywords
           lext-font-lock-keywords-1 lext-font-lock-keywords-2)
          nil t (("+-*/.<>=!?$%_&~^:" . "w") (?#. "w 14"))
          beginning-of-defun
          (font-lock-mark-block-function . mark-defun)))
  (setq-local prettify-symbols-alist lisp-prettify-symbols-alist)
  (setq-local lisp-doc-string-elt-property 'lext-doc-string-elt))

(defvar lext-mode-line-process "")

(defvar-keymap lext-mode-map
  :doc "Keymap for Lext mode.
All commands in `lisp-mode-shared-map' are inherited by this map."
  :parent lisp-mode-shared-map)

(easy-menu-define lext-mode-menu lext-mode-map
  "Menu for Lext mode."
  '("Lext"
    ["Indent Line" lisp-indent-line]
    ["Indent Region" indent-region
     :enable mark-active]
    ["Comment Out Region" comment-region
     :enable mark-active]
    ["Uncomment Out Region" (lambda (beg end)
                                (interactive "r")
                                (comment-region beg end '(4)))
     :enable mark-active]))

(defun lext-mode-commands (map)
  (define-key map "\177" 'backward-delete-char-untabify)
  (define-key map "\e\C-q" 'indent-sexp))

;;;###autoload
(define-derived-mode lext-mode prog-mode "Lext"
  "Major mode for editing Lext-Scheme code.
Editing commands are similar to those of `scheme-mode' and `lisp-mode'."
  (lext-mode-variables))

(defgroup lext nil
  "Editing Lext code."
  :link '(custom-group-link :tag "Font Lock Faces group" font-lock-faces)
  :group 'lisp)

(defconst lext-font-lock-keywords-1
  (eval-when-compile
    (list
     (list (concat "(\\(define\\*?\\("
                   "\\(\\|-public\\|-method\\|-generic\\(-procedure\\)?\\)\\|"
                   "\\(-syntax\\|-macro\\)\\|"
                   "-class"
                   "\\|-module"
                   "\\)\\)\\>"
                   "[ \t]*(*"
                   "\\(\\sw+\\)?")
           '(1 font-lock-keyword-face)
           '(6 (cond ((match-beginning 3) font-lock-function-name-face)
                     ((match-beginning 5) font-lock-variable-name-face)
                     (t font-lock-type-face))
               nil t))
     ))
  "Subdued expressions to highlight in Lext modes.")

(defconst lext-font-lock-keywords-2
  (append
   (eval-when-compile
     (list
      ;; Control structures and Lext additions
      (cons
       (concat
        "(\\_<" (regexp-opt
             '(;; Standard Scheme
               "begin" "call-with-current-continuation" "call/cc"
               "call-with-input-file" "call-with-output-file"
               "call-with-port"
               "case" "cond"
               "do" "else" "for-each" "if" "lambda" "λ"
               "let" "let*" "let-syntax" "letrec" "letrec-syntax"
               "export" "import"
               "let-values" "let*-values"
               "and" "or" "delay" "force"
	       "map" "syntax" "syntax-rules"
	       "when" "unless" "letrec*" "include" "include-ci" "cond-expand"
	       "delay-force" "parameterize" "guard" "case-lambda"
	       "syntax-error" "only" "except" "prefix" "rename" "define-values"
	       "define-record-type" "define-library"
	       "include-library-declarations"
	       "receive"
               ;; Lext extensions
               "use" "c-import" "define-c-struct" "define-c-union" "define-c-enum"
               "c.import" "c.struct" "c.union" "c.enum"
               "with-heap-alloc" "with-alloc" "with-c-string" "with-c-array"
               "with-c-string-array" "capture" "open-namespace" "use-namespace"
               "c.with-heap-alloc" "c.with-alloc" "c.with-c-string" "c.with-c-array"
               "c.with-c-string-array" "bc.capture" "bc.shift"
               "while" "for" "foreach" "shift"
               ) t)
        "\\_>") 1)
      ;; Named-let
      '("(let\\s-+\\(\\sw+\\)"
        (1 font-lock-function-name-face))
      ;;
      ;; FFI Primitive Built-ins (ffi-* and lext-*) & C Memory Operations
      (cons
       (concat
        "\\_<" (regexp-opt
                '("ffi-open" "ffi-sym" "ffi-close" "ffi-call"
                  "ffi-deref" "ffi-set!" "ffi-typedef" "ffi-align"
                  "ffi-size" "ffi-callback"
                  ;; Native lext builtins
                  "lext-calloc" "lext-string->c-string" "lext-c-string-from-ptr"
                  "lext-c-string-array->list" "lext-sv->string" "lext-capture-output"
                  "lext-da-reserve" "lext-malloc-tracked" "lext-free-tracked"
                  "lext-bounds-check"
                  ;; C memory helpers (standard, bc. prefixed, c. prefixed)
                  "malloc" "free" "realloc" "calloc" "deref" "set!" "addr"
                  "null-ptr" "null-ptr?" "c-cast"
                  "c.malloc" "c.free" "c.realloc" "c.calloc" "c.deref" "c.set!"
                  "c.addr" "c.null-ptr" "c.null-ptr?" "c.c-cast" "c.memcpy" "c.memset" "c.ptr+"
                  "c.raw-malloc" "c.raw-free" "c.raw-realloc" "c.raw-calloc" "c.raw-memcpy" "c.raw-memset" "c.raw-ptr+"
                  "c.tptr" "c.tptr-type" "c.tptr-ptr" "c.tptr?" "c.string-from-ptr" "c.string-array->list" "c.string->c-string" "c.size"
                  "bc.malloc" "bc.free" "bc.realloc" "bc.calloc" "bc.deref" "bc.set!"
                  "bc.addr" "bc.null-ptr" "bc.null-ptr?" "bc.c-cast"
                  "bc.malloc-tracked" "bc.free-tracked" "bc.bounds-check"
                  "c.malloc-tracked" "c.free-tracked" "c.bounds-check"
                  ;; Other general library names
                  "translate-ffi-type" "c-size" "c-string-from-ptr" "c-string-array->list"
                  ) t)
        "\\_>")
       'font-lock-builtin-face)
      ;;
      ;; Nob task runner API (nob.*)
      '("\\_<nob\\.[a-zA-Z0-9_-]+\\>" . font-lock-function-name-face)
      ;;
      ;; FFI Default Primitive Types
      (cons
       (concat
        "\\_<" (regexp-opt
                '("void" "int" "uint" "long" "ulong" "size_t"
                  "char" "schar" "uchar" "int8" "uint8"
                  "short" "ushort" "int16" "uint16"
                  "int32" "uint32" "int64" "uint64"
                  "float" "double" "string" "pointer"
                  "struct" "union" "array") t)
        "\\_>")
       'font-lock-type-face)
      ;;
      ;; Memory operator warning faces (prefixed or raw: @, =, &)
      '("\\(?:\\_<c\\.\\|\\_<bc\\.\\|\\_<\\)\\(?:@\\|=\\|&\\)" . font-lock-warning-face)
      ;;
      ;; Raw string prefix (the 'r' before double quote in r"...")
      '("\\_<\\(r\\)\"" 1 font-lock-warning-face)
      ;;
      ;; Lext/Nob C struct type names (capitalized like Nob_Cmd, My_Inner, etc.)
      '("\\_<\\(?:Nob_\\|My_\\)[a-zA-Z0-9_]+\\_>" . font-lock-type-face)
      ;;
      ;; Keywords as builtins
      '("\\<#?:\\sw+\\>" . font-lock-builtin-face)
      ;; Booleans
      '("\\_<#[tf]\\_>" . font-lock-constant-face)
      ))
   lext-font-lock-keywords-1
   ;; Anchored rainbow delimiters matching (placed last to override paren faces)
   '(lext-rainbow-highlight-delimiters)))

(defvar lext-font-lock-keywords lext-font-lock-keywords-1
  "Default expressions to highlight in Lext mode.")

(put 'lambda 'lext-doc-string-elt 2)
(put 'lambda* 'lext-doc-string-elt 2)
(put 'define 'lext-doc-string-elt
     (lambda ()
       (forward-comment (point-max))
       (if (eq (char-after) ?\() 2 0)))
(put 'define* 'lext-doc-string-elt 2)
(put 'case-lambda 'lext-doc-string-elt 1)
(put 'case-lambda* 'lext-doc-string-elt 1)
(put 'define-syntax-rule 'lext-doc-string-elt 2)
(put 'syntax-rules 'lext-doc-string-elt 2)

(defun lext-syntax-proper-regexp-check (end)
  (let* ((state (syntax-ppss))
         (within-str (nth 3 state))
         (start-delim-pos (nth 8 state)))
    (when (and within-str
               (char-equal ?# (char-after start-delim-pos)))
      (while (and (re-search-forward "/" end 'move)
                  (eq -1
                      (% (save-excursion
                           (backward-char)
                           (skip-chars-backward "\\\\"))
                         2))))
      (when (< (point) end)
       (put-text-property (match-beginning 0) (match-end 0)
                          'syntax-table (string-to-syntax "|"))))))

(defun lext-syntax-propertize (beg end)
  (goto-char beg)
  (lext-syntax-propertize-sexp-comment end)
  (lext-syntax-proper-regexp-check end)
  (funcall
   (syntax-propertize-rules
    ("\\(#\\);" (1 (prog1 "< cn"
                     (lext-syntax-propertize-sexp-comment end))))
    ("\\(#\\)/" (1 (when (null (nth 8 (save-excursion
                                        (syntax-ppss (match-beginning 0)))))
                     (put-text-property
                      (match-beginning 1)
                      (match-end 1)
                      'syntax-table (string-to-syntax "|"))
                     (lext-syntax-proper-regexp-check end)
                     nil))))
   (point) end))

(defun lext-syntax-propertize-sexp-comment (end)
  (let ((state (syntax-ppss))
        (checked (point)))
    (when (eq 2 (nth 7 state))
      (named-let loop ((startpos (+ 2 (nth 8 state))))
        (let ((found nil))
          (while
              (progn
                (setq found nil)
                (condition-case nil
                    (save-restriction
                      (narrow-to-region (point-min) end)
                      (goto-char startpos)
                      (forward-sexp 1)
                      (setq found (point)))
                  (scan-error (goto-char end)))
                (let ((limit (min end (or found end))))
                  (when (< checked limit)
                    (goto-char checked)
                    (while (and (re-search-forward "\\(#\\);" limit 'move)
                                (nth 8 (save-excursion
                                         (parse-partial-sexp
                                          startpos (match-beginning 0))))))
                    (setq checked (point))
                    (when (< (point) limit)
                      (put-text-property (match-beginning 1) (match-end 1)
                                         'syntax-table
                                         (string-to-syntax "< cn"))
                      (loop (point))
                      t)))))
          (when found
            (goto-char found)
            (put-text-property (1- found) found
                               'syntax-table (string-to-syntax "> cn"))))))))

(defun lext-syntax-propertize-regexp (end)
  (lext-syntax-proper-regexp-check end))

(defvar calculate-lisp-indent-last-sexp)

(defun lext-indent-function (indent-point state)
  (let ((normal-indent (current-column)))
    (goto-char (1+ (elt state 1)))
    (parse-partial-sexp (point) calculate-lisp-indent-last-sexp 0 t)
    (if (and (elt state 2)
             (not (looking-at "\\sw\\|\\s_")))
        (progn
          (if (not (> (save-excursion (forward-line 1) (point))
                      calculate-lisp-indent-last-sexp))
              (progn (goto-char calculate-lisp-indent-last-sexp)
                     (beginning-of-line)
                     (parse-partial-sexp (point)
                                         calculate-lisp-indent-last-sexp 0 t)))
          (backward-prefix-chars)
          (current-column))
      (let ((function (buffer-substring (point)
                                        (progn (forward-sexp 1) (point))))
            method)
        (setq method (or (get (intern-soft function) 'lext-indent-function)
                         (get (intern-soft function) 'lext-indent-hook)))
        (cond ((or (eq method 'defun)
                   (and (null method)
                        (> (length function) 3)
                        (string-match "\\`def" function)))
               (lisp-indent-defform state indent-point))
              ((integerp method)
               (lisp-indent-specform method state
                                     indent-point normal-indent))
              (method
                (funcall method state indent-point normal-indent)))))))

(defun lext-let-indent (state indent-point normal-indent)
  (skip-chars-forward " \t")
  (if (looking-at "[-a-zA-Z0-9+*/?!@$%^&_:~]")
      (lisp-indent-specform 2 state indent-point normal-indent)
    (lisp-indent-specform 1 state indent-point normal-indent)))

(put 'begin 'lext-indent-function 0)
(put 'case 'lext-indent-function 1)
(put 'delay 'lext-indent-function 0)
(put 'do 'lext-indent-function 2)
(put 'lambda 'lext-indent-function 1)
(put 'let 'lext-indent-function 'lext-let-indent)
(put 'let* 'lext-indent-function 1)
(put 'letrec 'lext-indent-function 1)
(put 'let-values 'lext-indent-function 1)
(put 'let*-values 'lext-indent-function 1)
(put 'and-let* 'lext-indent-function 1)
(put 'sequence 'lext-indent-function 0)
(put 'let-syntax 'lext-indent-function 1)
(put 'letrec-syntax 'lext-indent-function 1)
(put 'syntax-rules 'lext-indent-function 'defun)
(put 'syntax-case 'lext-indent-function 2)
(put 'with-syntax 'lext-indent-function 1)
(put 'library 'lext-indent-function 1)
(put 'eval-when 'lext-indent-function 1)

(put 'call-with-input-file 'lext-indent-function 1)
(put 'call-with-port 'lext-indent-function 1)
(put 'with-input-from-file 'lext-indent-function 1)
(put 'with-input-from-port 'lext-indent-function 1)
(put 'call-with-output-file 'lext-indent-function 1)
(put 'with-output-to-file 'lext-indent-function 1)
(put 'with-output-to-port 'lext-indent-function 1)
(put 'call-with-values 'lext-indent-function 1)
(put 'dynamic-wind 'lext-indent-function 3)

(put 'when 'lext-indent-function 1)
(put 'unless 'lext-indent-function 1)
(put 'letrec* 'lext-indent-function 1)
(put 'parameterize 'lext-indent-function 1)
(put 'define-values 'lext-indent-function 1)
(put 'define-record-type 'lext-indent-function 1)
(put 'define-library 'lext-indent-function 1)
(put 'receive 'lext-indent-function 2)
(put 'match 'lext-indent-function 1)
(put 'match-lambda 'lext-indent-function 0)
(put 'match-lambda* 'lext-indent-function 0)
(put 'match-let 'lext-indent-function 'lext-let-indent)
(put 'match-let* 'lext-indent-function 1)
(put 'match-letrec 'lext-indent-function 1)

;; Lext custom indentation configurations
(put 'use 'lext-indent-function 0)
(put 'c-import 'lext-indent-function 1)
(put 'define-c-struct 'lext-indent-function 1)
(put 'define-c-union 'lext-indent-function 1)
(put 'define-c-enum 'lext-indent-function 1)
(put 'with-heap-alloc 'lext-indent-function 1)
(put 'with-alloc 'lext-indent-function 1)
(put 'with-c-string 'lext-indent-function 1)
(put 'with-c-array 'lext-indent-function 1)
(put 'with-c-string-array 'lext-indent-function 1)
(put 'use-namespace 'lext-indent-function 1)
(put 'open-namespace 'lext-indent-function 1)
(put 'while 'lext-indent-function 1)
(put 'for 'lext-indent-function 1)
(put 'foreach 'lext-indent-function 1)

(put 'bc.while 'lext-indent-function 1)
(put 'bc.for 'lext-indent-function 1)
(put 'bc.foreach 'lext-indent-function 1)
(put 'capture 'lext-indent-function 0)
(put 'bc.capture 'lext-indent-function 0)
(put 'c.with-heap-alloc 'lext-indent-function 1)
(put 'c.with-alloc 'lext-indent-function 1)
(put 'c.with-c-string 'lext-indent-function 1)
(put 'c.with-c-array 'lext-indent-function 1)
(put 'c.with-c-string-array 'lext-indent-function 1)

;;;###autoload
(add-to-list 'auto-mode-alist '("\\.lext\\'" . lext-mode))

(provide 'lext-mode)

;;; lext-mode.el ends here
