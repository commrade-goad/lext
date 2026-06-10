#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "s7/s7.h"

// Dynamic resizing string buffer to capture raw Lisp forms
typedef struct {
    char *data;
    size_t cap;
    size_t len;
} Buf;

void buf_init(Buf *b) {
    b->cap = 1024;
    b->len = 0;
    b->data = malloc(b->cap);
    b->data[0] = '\0';
}

void buf_push(Buf *b, char c) {
    if (b->len + 1 >= b->cap) {
        b->cap *= 2;
        b->data = realloc(b->data, b->cap);
    }
    b->data[b->len++] = c;
    b->data[b->len] = '\0';
}

void buf_free(Buf *b) {
    free(b->data);
}

// Lexer states for stripping or capturing delimiters
enum State {
    STATE_TEXT,
    STATE_AT1,
    STATE_AT2
};

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.texm> <output.tex>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *in = fopen(argv[1], "r");
    if (!in) {
        fprintf(stderr, "ERR: Could not open input file: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    FILE *out = fopen(argv[2], "w");
    if (!out) {
        fprintf(stderr, "ERR: Could not create output file: %s\n", argv[2]);
        fclose(in);
        return EXIT_FAILURE;
    }

    // Initialize s7 VM state
    s7_scheme *s7 = s7_init();
    if (!s7) {
        fprintf(stderr, "ERR: Failed to initialize s7 Scheme engine.\n");
        fclose(in);
        fclose(out);
        return EXIT_FAILURE;
    }

    enum State state = STATE_TEXT;
    int c;

    while ((c = fgetc(in)) != EOF) {
        switch (state) {
            case STATE_TEXT:
                if (c == '@') {
                    state = STATE_AT1;
                } else {
                    fputc(c, out);
                }
                break;

            case STATE_AT1:
                if (c == '@') {
                    state = STATE_AT2;
                } else {
                    fputc('@', out);
                    fputc(c, out);
                    state = STATE_TEXT;
                }
                break;

            case STATE_AT2:
                if (c == '(') {
                    // Delimiter matches "@@(". Initialize macro buffer.
                    Buf lisp_buf;
                    buf_init(&lisp_buf);
                    buf_push(&lisp_buf, '('); // Capture the opening parenthesis

                    int depth = 1;
                    int lc;
                    bool in_string = false;
                    bool in_comment = false;
                    bool escape = false;

                    // Safely track internal token blocks to compute exact tree balance
                    while (depth > 0 && (lc = fgetc(in)) != EOF) {
                        if (in_comment) {
                            buf_push(&lisp_buf, lc);
                            if (lc == '\n') in_comment = false;
                            continue;
                        }

                        if (escape) {
                            buf_push(&lisp_buf, lc);
                            escape = false;
                            continue;
                        }

                        if (lc == '\\') {
                            if (in_string) {
                                int next_c = fgetc(in);
                                if (next_c == '"') {
                                    buf_push(&lisp_buf, '\\');
                                    buf_push(&lisp_buf, '"');
                                } else {
                                    buf_push(&lisp_buf, '\\');
                                    buf_push(&lisp_buf, '\\');
                                    if (next_c != EOF) {
                                        ungetc(next_c, in);
                                    }
                                }
                            } else {
                                buf_push(&lisp_buf, '\\');
                                escape = true;
                            }
                            continue;
                        }

                        buf_push(&lisp_buf, lc);
                        if (lc == '"') {
                            in_string = !in_string;
                            continue;
                        }

                        if (!in_string) {
                            if (lc == ';') {
                                in_comment = true;
                            } else if (lc == '(') {
                                depth++;
                            } else if (lc == ')') {
                                depth--;
                            }
                        }
                    }

                    if (depth > 0) {
                        fprintf(stderr, "ERR: Unmatched parenthesis inside Lisp block.\n");
                        buf_free(&lisp_buf);
                        fclose(in);
                        fclose(out);
                        return EXIT_FAILURE;
                    }

                    // Evaluate expression immediately inside s7 environment
                    s7_pointer res = s7_eval_c_string(s7, lisp_buf.data);
                    
                    // Streaming design rule: Evaluate definitions silently. 
                    // Only dump explicitly computed string constants into target file.
                    if (s7_is_string(res)) {
                        fputs(s7_string(res), out);
                    }

                    buf_free(&lisp_buf);
                    state = STATE_TEXT;
                } else {
                    // Fallback when sequence was just false-alarm standard text
                    fputs("@@", out);
                    fputc(c, out);
                    state = STATE_TEXT;
                }
                break;
        }
    }

    // Flush any leftover parsing state artifacts
    if (state == STATE_AT1) {
        fputc('@', out);
    } else if (state == STATE_AT2) {
        fputs("@@", out);
    }

    fclose(in);
    fclose(out);
    return EXIT_SUCCESS;
}
