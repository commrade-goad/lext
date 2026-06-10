#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "s7/s7.h"

#define VERSTRING  "0.1.0"
#ifndef HASHVER
#define HASHVER "unknown"
#endif

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

#include <dlfcn.h>
#include <ffi.h>

static s7_pointer ffi_open(s7_scheme *sc, s7_pointer args) {
    s7_pointer path_arg = s7_car(args);
    const char *path = s7_is_string(path_arg) ? s7_string(path_arg) : NULL;
    void *handle = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
    if (!handle) {
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2, s7_make_string(sc, "ffi-open failed: ~A"),
                                       s7_make_string(sc, dlerror())));
    }
    return s7_make_c_pointer(sc, handle);
}

static s7_pointer ffi_sym(s7_scheme *sc, s7_pointer args) {
    s7_pointer handle_arg = s7_car(args);
    s7_pointer name_arg = s7_cadr(args);
    if (!s7_is_c_pointer(handle_arg)) {
        return s7_wrong_type_arg_error(sc, "ffi-sym", 1, handle_arg, "c-pointer");
    }
    if (!s7_is_string(name_arg)) {
        return s7_wrong_type_arg_error(sc, "ffi-sym", 2, name_arg, "string");
    }
    void *handle = s7_c_pointer(handle_arg);
    const char *name = s7_string(name_arg);
    void *sym = dlsym(handle, name);
    if (!sym) {
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2, s7_make_string(sc, "ffi-sym failed: ~A"),
                                       s7_make_string(sc, dlerror())));
    }
    return s7_make_c_pointer(sc, sym);
}

static s7_pointer ffi_close(s7_scheme *sc, s7_pointer args) {
    s7_pointer handle_arg = s7_car(args);
    if (!s7_is_c_pointer(handle_arg)) {
        return s7_wrong_type_arg_error(sc, "ffi-close", 1, handle_arg, "c-pointer");
    }
    void *handle = s7_c_pointer(handle_arg);
    int res = dlclose(handle);
    return s7_make_integer(sc, res);
}

static ffi_type *parse_ffi_type(s7_scheme *sc, s7_pointer sym) {
    if (!s7_is_symbol(sym)) return NULL;
    const char *name = s7_symbol_name(sym);
    if (strcmp(name, "void") == 0) return &ffi_type_void;
    if (strcmp(name, "int") == 0) return &ffi_type_sint;
    if (strcmp(name, "double") == 0) return &ffi_type_double;
    if (strcmp(name, "float") == 0) return &ffi_type_float;
    if (strcmp(name, "string") == 0) return &ffi_type_pointer;
    if (strcmp(name, "pointer") == 0) return &ffi_type_pointer;
    return NULL;
}

typedef union {
    int i;
    float f;
    double d;
    void *p;
} ffi_val;

static s7_pointer s7_ffi_call(s7_scheme *sc, s7_pointer args) {
    s7_pointer func_arg = s7_car(args);
    s7_pointer ret_type_arg = s7_cadr(args);
    s7_pointer arg_types_list = s7_caddr(args);
    s7_pointer arg_vals_list = s7_cadddr(args);

    if (!s7_is_c_pointer(func_arg)) {
        return s7_wrong_type_arg_error(sc, "ffi-call", 1, func_arg, "c-pointer");
    }
    void *func = s7_c_pointer(func_arg);

    ffi_type *ret_type = parse_ffi_type(sc, ret_type_arg);
    if (!ret_type) {
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2, s7_make_string(sc, "invalid return type: ~S"), ret_type_arg));
    }

    int nargs = s7_list_length(sc, arg_types_list);
    if (nargs != s7_list_length(sc, arg_vals_list)) {
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 1, s7_make_string(sc, "argument types and values count mismatch")));
    }

    ffi_type **arg_types = malloc(nargs * sizeof(ffi_type *));
    void **arg_values = malloc(nargs * sizeof(void *));
    ffi_val *vals = malloc(nargs * sizeof(ffi_val));

    s7_pointer t_curr = arg_types_list;
    s7_pointer v_curr = arg_vals_list;

    for (int i = 0; i < nargs; i++) {
        s7_pointer t_sym = s7_car(t_curr);
        s7_pointer val = s7_car(v_curr);

        ffi_type *t = parse_ffi_type(sc, t_sym);
        if (!t) {
            free(arg_types);
            free(arg_values);
            free(vals);
            return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                            s7_list(sc, 2, s7_make_string(sc, "invalid argument type: ~S"), t_sym));
        }

        arg_types[i] = t;

        if (t == &ffi_type_sint) {
            vals[i].i = (int)s7_integer(val);
            arg_values[i] = &vals[i].i;
        } else if (t == &ffi_type_double) {
            vals[i].d = s7_real(val);
            arg_values[i] = &vals[i].d;
        } else if (t == &ffi_type_float) {
            vals[i].f = (float)s7_real(val);
            arg_values[i] = &vals[i].f;
        } else if (t == &ffi_type_pointer) {
            if (s7_is_string(val)) {
                vals[i].p = (void *)s7_string(val);
            } else if (s7_is_c_pointer(val)) {
                vals[i].p = s7_c_pointer(val);
            } else if (s7_is_null(sc, val)) {
                vals[i].p = NULL;
            } else {
                free(arg_types);
                free(arg_values);
                free(vals);
                return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                                s7_list(sc, 2, s7_make_string(sc, "invalid pointer value: ~S"), val));
            }
            arg_values[i] = &vals[i].p;
        }

        t_curr = s7_cdr(t_curr);
        v_curr = s7_cdr(v_curr);
    }

    ffi_cif cif;
    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, nargs, ret_type, arg_types) != FFI_OK) {
        free(arg_types);
        free(arg_values);
        free(vals);
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 1, s7_make_string(sc, "ffi_prep_cif failed")));
    }

    ffi_val ret_val;
    ffi_call(&cif, FFI_FN(func), &ret_val, arg_values);

    s7_pointer result = s7_nil(sc);
    if (ret_type == &ffi_type_void) {
        result = s7_unspecified(sc);
    } else if (ret_type == &ffi_type_sint) {
        result = s7_make_integer(sc, ret_val.i);
    } else if (ret_type == &ffi_type_double) {
        result = s7_make_real(sc, ret_val.d);
    } else if (ret_type == &ffi_type_float) {
        result = s7_make_real(sc, (double)ret_val.f);
    } else if (ret_type == &ffi_type_pointer) {
        const char *ret_name = s7_symbol_name(ret_type_arg);
        if (strcmp(ret_name, "string") == 0) {
            if (ret_val.p == NULL) {
                result = s7_nil(sc);
            } else {
                result = s7_make_string(sc, (const char *)ret_val.p);
            }
        } else {
            result = s7_make_c_pointer(sc, ret_val.p);
        }
    }

    free(arg_types);
    free(arg_values);
    free(vals);

    return result;
}

// Lexer states for stripping or capturing delimiters
enum State {
    STATE_TEXT,
    STATE_AT1,
    STATE_AT2
};

int main(int argc, char **argv) {
    bool escape_enabled = false;
    char *input_file = NULL;
    char *output_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [options] <input.lext> <output>\n", argv[0]);
            printf("Options:\n");
            printf("  -h, --help      Show this help message\n");
            printf("  -v, --version   Show version information\n");
            printf("  -e, --escape    Enable auto-escaping of backslashes inside string literals in @@(...)\n");
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("lext version " VERSTRING "-" HASHVER "\n");
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--escape") == 0) {
            escape_enabled = true;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            fprintf(stderr, "Usage: %s [options] <input.texm> <output.tex>\n", argv[0]);
            return EXIT_FAILURE;
        } else {
            if (!input_file) {
                input_file = argv[i];
            } else if (!output_file) {
                output_file = argv[i];
            } else {
                fprintf(stderr, "Too many arguments.\n");
                fprintf(stderr, "Usage: %s [options] <input.texm> <output.tex>\n", argv[0]);
                return EXIT_FAILURE;
            }
        }
    }

    if (!input_file || !output_file) {
        fprintf(stderr, "Error: Input and output files are required.\n");
        fprintf(stderr, "Usage: %s [options] <input.texm> <output.tex>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *in = fopen(input_file, "r");
    if (!in) {
        fprintf(stderr, "ERR: Could not open input file: %s\n", input_file);
        return EXIT_FAILURE;
    }

    FILE *out = fopen(output_file, "w");
    if (!out) {
        fprintf(stderr, "ERR: Could not create output file: %s\n", output_file);
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

    // Register FFI bindings
    s7_define_function(s7, "ffi-open", ffi_open, 1, 0, false, "(ffi-open path) loads dynamic library");
    s7_define_function(s7, "ffi-sym", ffi_sym, 2, 0, false, "(ffi-sym handle name) finds symbol in library");
    s7_define_function(s7, "ffi-close", ffi_close, 1, 0, false, "(ffi-close handle) closes dynamic library");
    s7_define_function(s7, "ffi-call", s7_ffi_call, 4, 0, false, "(ffi-call func ret-type arg-types arg-vals) invokes foreign function");

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
                            if (escape_enabled && in_string) {
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
