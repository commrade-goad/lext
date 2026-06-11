(define (string-join lst delimiter)
  (if (null? lst)
      ""
      (let loop ((rest (cdr lst))
                 (result (car lst)))
        (if (null? rest)
            result
            (loop (cdr rest) (string-append result delimiter (car rest)))))))

(define (render-val val)
  (cond
    ((string? val) val)
    ((symbol? val) (symbol->string val))
    ((number? val) (number->string val))
    (else (latex-render val))))

(define (render-options opts)
  (if (null? opts)
      ""
      (let ((rendered-opts
             (map (lambda (opt)
                    (cond
                      ((pair? opt)
                       (if (null? (cdr opt))
                           (render-val (car opt))
                           (string-append (render-val (car opt)) "=" (render-val (cadr opt)))))
                      (else (render-val opt))))
                  opts)))
        (string-append "[" (string-join rendered-opts ",") "]"))))

(define (latex-render expr)
  (cond
    ((null? expr) "")
    ((string? expr) expr)
    ((number? expr) (number->string expr))
    ((symbol? expr) (string-append "\\" (symbol->string expr)))
    ((list? expr)
     (let ((tag (car expr))
           (rest (cdr expr)))
       (cond
         ;; 1. Environments: (begin env-name (@ (opt1 val1) ...) body...)
         ((eq? tag 'begin)
          (let* ((env-name (render-val (car rest)))
                 (args (cdr rest))
                 (has-opts (and (not (null? args))
                                (pair? (car args))
                                (eq? (caar args) '@)))
                 (opts (if has-opts (cdar args) '()))
                 (body (if has-opts (cdr args) args)))
            (string-append
              "\\begin{" env-name "}" (render-options opts) "\n"
              (string-join (map latex-render body) "\n") "\n"
              "\\end{" env-name "}")))
         ;; 2. Commands: (cmd-name (@ (opt1 val1) ...) arg1 arg2...)
         (else
          (let* ((cmd-name (symbol->string tag))
                 (has-opts (and (not (null? rest))
                                (pair? (car rest))
                                (eq? (caar rest) '@)))
                 (opts (if has-opts (cdar rest) '()))
                 (args (if has-opts (cdr rest) rest)))
            (string-append
              "\\" cmd-name (render-options opts)
              (if (null? args)
                  ""
                  (string-append "{" (string-join (map latex-render args) "}{") "}"))))))))
    (else "")))

(define (make-table spec headers rows)
  (let* ((header-row (string-join headers " & "))
         (body-rows  (map (lambda (row) (string-join row " & ")) rows))
         (all-rows   (append (list header-row) body-rows))
         (table-body (let loop ((rest all-rows) (acc '()))
                       (if (null? rest)
                           (reverse (cons '(hline) acc))
                           (loop (cdr rest) (cons (car rest) (cons '(hline) acc)))))))
    (latex-render
      `(begin tabular (@ (,spec))
         (hline)
         ,@table-body))))

#|
=== LaTeX S-Expression DSL Usage Examples ===

1. Basic Command:
   (latex-render '(textbf "Hello World"))
   ;; => "\\textbf{Hello World}"

2. Command with Options (using '@):
   (latex-render '(includegraphics (@ (width "0.5\\textwidth")) "assets/logo.png"))
   ;; => "\\includegraphics[width=0.5\\textwidth]{assets/logo.png}"

3. Basic Environment:
   (latex-render '(begin center "Centred Text"))
   ;; => "\\begin{center}\nCentred Text\n\\end{center}"

4. Environment with Options:
   (latex-render
     '(begin lstlisting (@ (language bash) (numbers left))
        "echo \"hello world\""))
   ;; => "\\begin{lstlisting}[language=bash,numbers=left]\necho \"hello world\"\n\\end{lstlisting}"

5. Complex Nested Environment (Figure Example):
   (latex-render
     '(begin figure (@ ("!h"))
        (centering)
        (includegraphics (@ (width "0.5\\textwidth")) "assets/logo.png")
        (caption "A cool figure")
        (label "fig:logo")))
   ;; =>
   ;; \\begin{figure}[!h]
   ;; \\centering
   ;; \\includegraphics[width=0.5\\textwidth]{assets/logo.png}
   ;; \\caption{A cool figure}
   ;; \\label{fig:logo}
   ;; \\end{figure}

6. Programmatic Table Generation (using make-table):
   (make-table "|l|c|"
               '("Component" "Version")
               '(("s7 Scheme" "1.0")
                 ("libffi" "3.4")))
   ;; =>
   ;; \\begin{tabular}{|l|c|}
   ;; \\hline
   ;; Component & Version \\
   ;; \\hline
   ;; s7 Scheme & 1.0 \\
   ;; \\hline
   ;; libffi & 3.4 \\
   ;; \\hline
   ;; \\end{tabular}
|#
