;; macros.scm - Native s7 Scheme template macros
;; No escaping needed! The C preprocessor engine handles it at compile-time.

(define (kodelist lang caption label body)
  (with-output-to-string
    (lambda ()
      (display "\begin{figure}[H]\n")
      (display "    \centering\n")
      (format #t "    \begin{lstlisting}[language=~A]\n" lang)
      (display body)
      (display "\n    \end{lstlisting}\n")
      (format #t "    \caption{~A}\n" caption)
      (format #t "    \label{~A}\n" label)
      (display "\end{figure}\n"))))

(define (figuraw path caption label width)
  (with-output-to-string
    (lambda ()
      (display "\begin{figure}[H]\n")
      (display "    \centering\n")
      (format #t "    \IfFileExists{~A}{\n" path)
      (format #t "        \includegraphics[width=~A\textwidth]{~A}\n" width path)
      (display "    }{\n")
      (format #t "        \framebox[~A\textwidth]{\rule{0pt}{150pt}\textbf{[Placeholder: ~A]}}\n" width path)
      (display "    }\n")
      (format #t "    \caption{~A}\n" caption)
      (format #t "    \label{~A}\n" label)
      (display "\end{figure}\n"))))

(define (figura path caption label)
  (figuraw path caption label 0.5))

(define (tabella caption label body)
  (with-output-to-string
    (lambda ()
      (display "\begin{table}[H]\n")
      (display "    \centering\n")
      (display body)
      (display "\n")
      (format #t "    \caption{~A}\n" caption)
      (format #t "    \label{~A}\n" label)
      (display "\end{table}\n"))))
