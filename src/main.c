#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "s7/s7.h"
#include "helpa.h"
#include "lext_hash.h"
#include "lext_types.h"
#include "lext_io.h"
#include "lext_ffi.h"
#include "lext_module.h"
#include "lext_capture.h"
#include "lext_da.h"
#include "lext_builtins.h"

#define VERSTRING "1.1.0"
#ifndef HASHVER
#define HASHVER "unknown"
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#  define PLATFORM_NAME "windows"
#elif defined(__APPLE__) || defined(__MACH__)
#  define PLATFORM_NAME "macos"
#elif defined(__linux__)
#  define PLATFORM_NAME "linux"
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#  define PLATFORM_NAME "freebsd"
#else
#  define PLATFORM_NAME "unknown"
#endif

/* ------------------------------------------------------------------ */
/* #11: ffi-typedef C builtin — registers named type AND caches ffi_type */
/* ------------------------------------------------------------------ */

static s7_pointer builtin_ffi_typedef(s7_scheme *sc, s7_pointer args) {
    s7_pointer name_arg = s7_car(args);
    s7_pointer type_arg = s7_cadr(args);
    /* Prepend to *ffi-types* */
    s7_pointer registry = s7_name_to_value(sc, "*ffi-types*");
    s7_define_variable(sc, "*ffi-types*",
        s7_cons(sc, s7_cons(sc, name_arg, type_arg), registry));
    /* Pre-parse and cache the ffi_type if lext_type_cache is ready */
    if (lext_type_cache && s7_is_symbol(name_arg)) {
        TypeAlloc *allocs = NULL;
        ffi_type *ft = lext_parse_ffi_type(sc, type_arg, &allocs);
        if (ft) {
            if (ft->type == FFI_TYPE_STRUCT && ft->size == 0) {
                ffi_cif dummy;
                ffi_prep_cif(&dummy, FFI_DEFAULT_ABI, 0, ft, NULL);
            }
            MeowHash key = meow_hash_string(s7_symbol_name(name_arg));
            meow_hash_table_set(lext_type_cache, key, ft);
            /* allocs intentionally leaked — cached types live forever */
        }
    }
    return s7_unspecified(sc);
}

/* ------------------------------------------------------------------ */
/* Template lexer state                                                 */
/* ------------------------------------------------------------------ */

typedef struct { size_t line; size_t col; } Loc;
typedef struct { Loc *dt; size_t sz; size_t cp; } Locs;

enum State { STATE_TEXT, STATE_AT1, STATE_AT2 };

/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */

int main(int argc, char **argv) {
    bool  no_template = false;
    char *input_file  = NULL;
    char *output_file = NULL;
    int   scheme_argc = 0;
    char **scheme_argv = NULL;
    char *script_name  = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--") == 0) {
            scheme_argc = argc - (i + 1);
            if (scheme_argc > 0) scheme_argv = &argv[i + 1];
            break;
        }
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [options] <input_file> [<output_file>]\n", argv[0]);
            printf("Options:\n");
            printf("  -h, --help         Show this help message\n");
            printf("  -v, --version      Show version information\n");
            printf("  -s, --no-template  Run as a pure Scheme script runner without output\n");
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("lext version " VERSTRING "-" HASHVER "\n");
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--no-template") == 0) {
            no_template = true;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            fprintf(stderr, "Usage: %s [options] <input_file> [<output_file>]\n", argv[0]);
            return EXIT_FAILURE;
        } else {
            if      (!input_file)  { input_file = argv[i]; script_name = argv[i]; }
            else if (!output_file)   output_file = argv[i];
            else {
                fprintf(stderr, "Too many arguments.\n");
                fprintf(stderr, "Usage: %s [options] <input_file> [<output_file>]\n", argv[0]);
                return EXIT_FAILURE;
            }
        }
    }

    if (!input_file) {
        fprintf(stderr, "Error: Input file is required.\n");
        if (no_template)
            fprintf(stderr, "Usage: %s [options] <input_file>\n", argv[0]);
        else
            fprintf(stderr, "Usage: %s [options] <input.texm> <output.tex>\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (!no_template && !output_file) {
        fprintf(stderr, "Error: Output file is required when not in --no-template mode.\n");
        fprintf(stderr, "Usage: %s [options] <input.texm> <output.tex>\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (no_template && output_file) {
        fprintf(stderr, "Error: Output file is not allowed in --no-template mode.\n");
        fprintf(stderr, "Usage: %s [options] <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *in = fopen(input_file, "r");
    if (!in) {
        fprintf(stderr, "ERR: Could not open input file: %s\n", input_file);
        return EXIT_FAILURE;
    }
    if (no_template) {
        fclose(in);
        in = NULL;
    }

    /* ---- Initialise s7 ---- */
    s7_scheme *s7 = s7_init();
    if (!s7) {
        fprintf(stderr, "ERR: Failed to initialize s7 Scheme engine.\n");
        if (in)  fclose(in);
        return EXIT_FAILURE;
    }

    /* ---- Initialise subsystems ---- */
    lext_module_init();

    /* #11: Init type cache */
    lext_type_cache = meow_hash_table_new();

    /* ---- Register builtins ---- */
    s7_define_variable(s7, "*ffi-types*", s7_nil(s7));

    /* #11: ffi-typedef as C builtin with caching */
    s7_define_function(s7, "ffi-typedef", builtin_ffi_typedef, 2, 0, false,
                       "(ffi-typedef name type-desc) registers named FFI type and caches parsed ffi_type");

    lext_module_register(s7);
    lext_ffi_register(s7);
    lext_capture_register(s7);
    lext_da_register(s7);
    lext_builtins_register(s7);

    /* ---- Script environment ---- */
    s7_pointer argv_list = s7_nil(s7);
    for (int i = scheme_argc - 1; i >= 0; i--)
        argv_list = s7_cons(s7, s7_make_string(s7, scheme_argv[i]), argv_list);

    s7_define_variable(s7, "*script-name*", s7_make_string(s7, script_name));
    s7_define_variable(s7, "*argv*",        argv_list);
    s7_define_variable(s7, "argv",          argv_list);
    s7_define_variable(s7, "*platform*",    s7_make_string(s7, PLATFORM_NAME));

    /* ---- Register Error Hook for Exit Status Tracking & Formatting ---- */
    s7_define_variable(s7, "*has-error*", s7_make_boolean(s7, false));
    s7_eval_c_string(s7,
        "(set! (hook-functions *error-hook*)\n"
        "      (cons (lambda (hook)\n"
        "              (set! *has-error* #t)\n"
        "              (let ((e (owlet)))\n"
        "                (if e\n"
        "                    (let ((err-type (e 'error-type))\n"
        "                          (err-data (e 'error-data))\n"
        "                          (err-code (e 'error-code))\n"
        "                          (err-file (e 'error-file))\n"
        "                          (err-line (e 'error-line))\n"
        "                          (err-pos  (e 'error-position)))\n"
        "                      (if (and (pair? err-data) (string? (car err-data)))\n"
        "                          (begin\n"
        "                            (format *stderr* \"~%;\")\n"
        "                            (apply format *stderr* err-data)\n"
        "                            (format *stderr* \"~%\"))\n"
        "                          (format *stderr* \"~%;~S ~S~%\" err-type err-data))\n"
        "                      (if (string? err-file)\n"
        "                          (format *stderr* \";    ~A~%;    ~A, line ~D, position: ~D~%\"\n"
        "                                  err-code err-file err-line err-pos)))))\n"
        "              #t)\n"
        "            (hook-functions *error-hook*)))");

    /* ---- Script-only mode ---- */
    if (no_template) {
        s7_load(s7, input_file);
        s7_pointer has_err = s7_name_to_value(s7, "*has-error*");
        if (s7_is_boolean(has_err) && s7_boolean(s7, has_err)) {
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    /* ----------------------------------------------------------------
     * Template engine — scan file for @@(...) delimiters and evaluate
     * the enclosed Scheme expressions, writing string results to `out_buf`.
     * ---------------------------------------------------------------- */
    Loc loc = {1, 0};
    enum State state = STATE_TEXT;
    int c;
    HStr out_buf;
    hstr_init(&out_buf);

    while ((c = fgetc(in)) != EOF) {
        if (c == '\n') { loc.line++; loc.col = 0; } else loc.col++;

        switch (state) {
        case STATE_TEXT:
            if (c == '@') state = STATE_AT1;
            else          hstr_push(&out_buf, c);
            break;

        case STATE_AT1:
            if (c == '@') state = STATE_AT2;
            else { hstr_push(&out_buf, '@'); hstr_push(&out_buf, c); state = STATE_TEXT; }
            break;

        case STATE_AT2:
            if (c == '(') {
                HStr lisp_buf;
                hstr_init(&lisp_buf);
                hstr_push(&lisp_buf, '(');

                Locs locs = {0};
                helpa_da_append(locs, loc);

                int  depth          = 1;
                int  lc;
                bool in_string      = false;
                bool in_comment     = false;
                bool escape         = false;
                bool in_raw_string  = false;

                while (depth > 0 && (lc = fgetc(in)) != EOF) {
                    if (lc == '\n') { loc.line++; loc.col = 0; } else loc.col++;

                    if (in_raw_string) {
                        if (lc == '\\') {
                            int n = fgetc(in);
                            if      (n == '\\') hstr_append_cstr(&lisp_buf, "\\\\\\\\");
                            else if (n == '"')  hstr_append_cstr(&lisp_buf, "\\\"");
                            else { if (n != EOF) ungetc(n, in); hstr_append_cstr(&lisp_buf, "\\\\"); }
                        } else if (lc == '"') { in_raw_string = false; hstr_push(&lisp_buf, lc); }
                        else hstr_push(&lisp_buf, lc);
                        continue;
                    }

                    if (escape) { hstr_push(&lisp_buf, lc); escape = false; continue; }

                    if (lc == '\\') { hstr_push(&lisp_buf, '\\'); escape = true; continue; }

                    if (in_comment) {
                        hstr_push(&lisp_buf, lc);
                        if (lc == '\n') in_comment = false;
                        continue;
                    }

                    if (!in_string && lc == 'r') {
                        int n = fgetc(in);
                        if (n == '"') { in_raw_string = true; hstr_push(&lisp_buf, '"'); continue; }
                        hstr_push(&lisp_buf, 'r');
                        if (n != EOF) ungetc(n, in);
                        continue;
                    }

                    hstr_push(&lisp_buf, lc);
                    if (lc == '"') { in_string = !in_string; continue; }

                    if (!in_string) {
                        if      (lc == ';') in_comment = true;
                        else if (lc == '(') { helpa_da_append(locs, loc); depth++; }
                        else if (lc == ')') { helpa_da_pop(locs);          depth--; }
                    }
                }

                if (depth > 0) {
                    Loc last = helpa_da_last(locs);
                    fprintf(stderr,
                            "%s:%zu:%zu: ERR: Unmatched parenthesis inside Lisp block "
                            "(remaining depth: %d).\n",
                            script_name, last.line, last.col, depth);
                    hstr_free(&lisp_buf);
                    helpa_da_free(locs);
                    fclose(in);
                    hstr_free(&out_buf);
                    return EXIT_FAILURE;
                }

                s7_pointer res = s7_eval_c_string(s7, (char *)lisp_buf.dt);
                s7_pointer has_err = s7_name_to_value(s7, "*has-error*");
                if (s7_is_boolean(has_err) && s7_boolean(s7, has_err)) {
                    helpa_da_free(locs);
                    hstr_free(&lisp_buf);
                    fclose(in);
                    hstr_free(&out_buf);
                    return EXIT_FAILURE;
                }
                if (s7_is_string(res))
                    hstr_append_cstr(&out_buf, s7_string(res));

                helpa_da_free(locs);
                hstr_free(&lisp_buf);
                state = STATE_TEXT;
            } else {
                hstr_append_cstr(&out_buf, "@@");
                hstr_push(&out_buf, c);
                state = STATE_TEXT;
            }
            break;
        }
    }

    if      (state == STATE_AT1) hstr_push(&out_buf, '@');
    else if (state == STATE_AT2) hstr_append_cstr(&out_buf, "@@");

    fclose(in);

    /* Write buffer to output file only at the very end upon success */
    FILE *out = fopen(output_file, "w");
    if (!out) {
        fprintf(stderr, "ERR: Could not create output file: %s\n", output_file);
        hstr_free(&out_buf);
        return EXIT_FAILURE;
    }
    if (out_buf.sz > 0) {
        fwrite(out_buf.dt, 1, out_buf.sz, out);
    }
    fclose(out);
    hstr_free(&out_buf);
    return EXIT_SUCCESS;
}
