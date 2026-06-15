/* lext_builtins.c — Items #3, #6, #9, #10: Small native builtins
 *
 *  #3  - lext-string->c-string, lext-c-string-from-ptr
 *  #6  - lext-sv->string
 *  #9  - lext-c-string-array->list
 *  #10 - lext-calloc
 */

#include "lext_builtins.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/* #10: (lext-calloc nmemb size) -> c-pointer                          */
/* ------------------------------------------------------------------ */

static s7_pointer builtin_lext_calloc(s7_scheme *sc, s7_pointer args) {
    s7_pointer nmemb_arg = s7_car(args);
    s7_pointer size_arg  = s7_cadr(args);

    if (!s7_is_integer(nmemb_arg))
        return s7_wrong_type_arg_error(sc, "lext-calloc", 1, nmemb_arg, "integer");
    if (!s7_is_integer(size_arg))
        return s7_wrong_type_arg_error(sc, "lext-calloc", 2, size_arg, "integer");

    size_t nmemb = (size_t)s7_integer(nmemb_arg);
    size_t sz    = (size_t)s7_integer(size_arg);

    void *p = calloc(nmemb, sz);
    if (!p && nmemb != 0 && sz != 0)
        return s7_error(sc, s7_make_symbol(sc, "memory-error"),
                        s7_list(sc, 1, s7_make_string(sc, "lext-calloc: calloc failed")));
    return s7_make_c_pointer(sc, p);
}

/* ------------------------------------------------------------------ */
/* #3: (lext-string->c-string scheme-string) -> c-pointer              */
/* Allocates a null-terminated copy; caller must free with bc.free.    */
/* ------------------------------------------------------------------ */

static s7_pointer builtin_string_to_c_string(s7_scheme *sc, s7_pointer args) {
    s7_pointer str_arg = s7_car(args);
    if (!s7_is_string(str_arg))
        return s7_wrong_type_arg_error(sc, "lext-string->c-string", 1, str_arg, "string");

    const char *s = s7_string(str_arg);
    size_t len = strlen(s);
    char *buf = (char *)malloc(len + 1);
    if (!buf)
        return s7_error(sc, s7_make_symbol(sc, "memory-error"),
                        s7_list(sc, 1, s7_make_string(sc,
                            "lext-string->c-string: malloc failed")));
    memcpy(buf, s, len + 1);
    return s7_make_c_pointer(sc, buf);
}

/* ------------------------------------------------------------------ */
/* #3: (lext-c-string-from-ptr c-pointer) -> Scheme string             */
/* Reads until null byte.                                               */
/* ------------------------------------------------------------------ */

static s7_pointer builtin_c_string_from_ptr(s7_scheme *sc, s7_pointer args) {
    s7_pointer ptr_arg = s7_car(args);
    if (!s7_is_c_pointer(ptr_arg))
        return s7_wrong_type_arg_error(sc, "lext-c-string-from-ptr", 1, ptr_arg, "c-pointer");

    void *p = s7_c_pointer(ptr_arg);
    if (!p)
        return s7_make_string(sc, "");

    return s7_make_string(sc, (const char *)p);
}

/* ------------------------------------------------------------------ */
/* #9: (lext-c-string-array->list c-pointer) -> list of strings        */
/* Takes a pointer to a NULL-terminated char** array.                  */
/* ------------------------------------------------------------------ */

static s7_pointer builtin_c_string_array_to_list(s7_scheme *sc, s7_pointer args) {
    s7_pointer ptr_arg = s7_car(args);

    /* Accept raw c-pointer */
    void *raw = NULL;
    if (s7_is_c_pointer(ptr_arg))
        raw = s7_c_pointer(ptr_arg);
    else if (s7_is_pair(ptr_arg) && s7_is_c_pointer(s7_cdr(ptr_arg)))
        raw = s7_c_pointer(s7_cdr(ptr_arg));
    else
        return s7_wrong_type_arg_error(sc, "lext-c-string-array->list", 1, ptr_arg, "c-pointer");

    if (!raw) return s7_nil(sc);

    char **arr = (char **)raw;
    s7_pointer result = s7_nil(sc);
    s7_pointer last   = s7_nil(sc);

    for (int i = 0; arr[i] != NULL; i++) {
        s7_pointer cell = s7_cons(sc, s7_make_string(sc, arr[i]), s7_nil(sc));
        if (s7_is_null(sc, result)) {
            result = cell;
            last   = cell;
        } else {
            s7_set_cdr(last, cell);
            last = cell;
        }
    }
    return result;
}

/* ------------------------------------------------------------------ */
/* #6: (lext-sv->string count data-ptr) -> Scheme string               */
/* count: integer (number of bytes), data-ptr: c-pointer to char data  */
/* ------------------------------------------------------------------ */

static s7_pointer builtin_sv_to_string(s7_scheme *sc, s7_pointer args) {
    s7_pointer count_arg = s7_car(args);
    s7_pointer data_arg  = s7_cadr(args);

    if (!s7_is_integer(count_arg))
        return s7_wrong_type_arg_error(sc, "lext-sv->string", 1, count_arg, "integer");

    s7_int count = s7_integer(count_arg);
    if (count <= 0) return s7_make_string(sc, "");

    void *data = NULL;
    if      (s7_is_c_pointer(data_arg)) data = s7_c_pointer(data_arg);
    else if (s7_is_pair(data_arg) && s7_is_c_pointer(s7_cdr(data_arg)))
        data = s7_c_pointer(s7_cdr(data_arg));
    else
        return s7_wrong_type_arg_error(sc, "lext-sv->string", 2, data_arg, "c-pointer");

    if (!data) return s7_make_string(sc, "");

    return s7_make_string_with_length(sc, (const char *)data, (s7_int)count);
}

/* ------------------------------------------------------------------ */
/* Weak definition of lext_make_libc_constants                         */
/* ------------------------------------------------------------------ */
__attribute__((weak)) s7_pointer lext_make_libc_constants(s7_scheme *sc, s7_pointer args) {
    return s7_inlet(sc, s7_nil(sc));
}

/* ------------------------------------------------------------------ */
/* UTF-8 Support                                                      */
/* ------------------------------------------------------------------ */

static s7_pointer builtin_lext_utf8_encode(s7_scheme *sc, s7_pointer args) {
    s7_pointer cp_arg = s7_car(args);
    if (!s7_is_integer(cp_arg))
        return s7_wrong_type_arg_error(sc, "lext-utf8-encode", 1, cp_arg, "integer");

    s7_int cp = s7_integer(cp_arg);
    char buf[5] = {0};
    if (cp <= 0x7F) {
        buf[0] = (char)cp;
    } else if (cp <= 0x7FF) {
        buf[0] = (char)(0xC0 | ((cp >> 6) & 0x1F));
        buf[1] = (char)(0x80 | (cp & 0x3F));
    } else if (cp <= 0xFFFF) {
        buf[0] = (char)(0xE0 | ((cp >> 12) & 0x0F));
        buf[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        buf[2] = (char)(0x80 | (cp & 0x3F));
    } else if (cp <= 0x10FFFF) {
        buf[0] = (char)(0xF0 | ((cp >> 18) & 0x07));
        buf[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        buf[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        buf[3] = (char)(0x80 | (cp & 0x3F));
    } else {
        return s7_error(sc, s7_make_symbol(sc, "utf8-error"),
                        s7_list(sc, 1, s7_make_string(sc, "lext-utf8-encode: invalid codepoint")));
    }
    return s7_make_string(sc, buf);
}

static s7_pointer builtin_lext_utf8_decode(s7_scheme *sc, s7_pointer args) {
    s7_pointer str_arg = s7_car(args);
    if (!s7_is_string(str_arg))
        return s7_wrong_type_arg_error(sc, "lext-utf8-decode", 1, str_arg, "string");

    const char *s = s7_string(str_arg);
    size_t len = strlen(s);
    s7_int offset = 0;

    if (s7_is_pair(s7_cdr(args))) {
        s7_pointer offset_arg = s7_cadr(args);
        if (!s7_is_integer(offset_arg))
            return s7_wrong_type_arg_error(sc, "lext-utf8-decode", 2, offset_arg, "integer");
        offset = s7_integer(offset_arg);
    }

    if (offset < 0 || (size_t)offset >= len) {
        return s7_make_integer(sc, -1);
    }

    const unsigned char *p = (const unsigned char *)(s + offset);
    s7_int cp = -1;
    if (p[0] <= 0x7F) {
        cp = p[0];
    } else if ((p[0] & 0xE0) == 0xC0) {
        if ((size_t)offset + 1 < len)
            cp = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
    } else if ((p[0] & 0xF0) == 0xE0) {
        if ((size_t)offset + 2 < len)
            cp = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
    } else if ((p[0] & 0xF8) == 0xF0) {
        if ((size_t)offset + 3 < len)
            cp = ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
    }
    return s7_make_integer(sc, cp);
}

/* ------------------------------------------------------------------ */
/* Registration                                                         */
/* ------------------------------------------------------------------ */

void lext_builtins_register(s7_scheme *sc) {
    s7_define_function(sc, "lext-make-libc-constants", lext_make_libc_constants, 0, 0, false,
                       "(lext-make-libc-constants) returns an inlet containing C preprocessor constants");
    s7_define_function(sc, "lext-calloc", builtin_lext_calloc, 2, 0, false,
                       "(lext-calloc nmemb size) allocates zero-filled memory");
    s7_define_function(sc, "lext-string->c-string", builtin_string_to_c_string, 1, 0, false,
                       "(lext-string->c-string str) copies Scheme string to malloc'd C string");
    s7_define_function(sc, "lext-c-string-from-ptr", builtin_c_string_from_ptr, 1, 0, false,
                       "(lext-c-string-from-ptr ptr) reads null-terminated C string as Scheme string");
    s7_define_function(sc, "lext-c-string-array->list", builtin_c_string_array_to_list, 1, 0, false,
                        "(lext-c-string-array->list ptr) converts NULL-terminated char** to list");
    s7_define_function(sc, "lext-sv->string", builtin_sv_to_string, 2, 0, false,
                       "(lext-sv->string count ptr) decodes a string view data pointer to Scheme string");
    s7_define_function(sc, "lext-utf8-encode", builtin_lext_utf8_encode, 1, 0, false,
                       "(lext-utf8-encode codepoint) encodes an integer codepoint to Scheme string");
    s7_define_function(sc, "lext-utf8-decode", builtin_lext_utf8_decode, 1, 1, false,
                       "(lext-utf8-decode str [offset]) decodes a character from the string at offset");


    /* Register open-namespace and use-namespace globally at startup */
    s7_eval_c_string(sc,
        "(define-macro (open-namespace prefix-arg)\n"
        "  (let* ((protected-syms '(set! = + - * / < > <= >=))\n"
        "         (prefix-raw (if (symbol? prefix-arg) (symbol->string prefix-arg) prefix-arg))\n"
        "         (prefix (if (and (string? prefix-raw)\n"
        "                          (> (string-length prefix-raw) 0)\n"
        "                          (char=? (string-ref prefix-raw (- (string-length prefix-raw) 1)) #\\.))\n"
        "                     prefix-raw\n"
        "                     (string-append prefix-raw \".\")))\n"
        "         (st (symbol-table))\n"
        "         (prefix-len (string-length prefix))\n"
        "         (bindings '()))\n"
        "    (for-each\n"
        "      (lambda (sym)\n"
        "        (let ((sym-str (symbol->string sym)))\n"
        "          (if (and (> (string-length sym-str) prefix-len)\n"
        "                   (string=? (substring sym-str 0 prefix-len) prefix)\n"
        "                   (defined? sym))\n"
        "              (let ((new-sym (string->symbol (substring sym-str prefix-len))))\n"
        "                (unless (member new-sym protected-syms)\n"
        "                  (set! bindings (cons `(define ,new-sym ,sym) bindings)))))))\n"
        "      st)\n"
        "    `(begin ,@(reverse bindings))))\n"
    );

    s7_eval_c_string(sc,
        "(define-macro (use-namespace prefix-arg . body)\n"
        "  (let* ((protected-syms '(set! = + - * / < > <= >=))\n"
        "         (prefix-raw (if (symbol? prefix-arg) (symbol->string prefix-arg) prefix-arg))\n"
        "         (prefix (if (and (string? prefix-raw)\n"
        "                          (> (string-length prefix-raw) 0)\n"
        "                          (char=? (string-ref prefix-raw (- (string-length prefix-raw) 1)) #\\.))\n"
        "                     prefix-raw\n"
        "                     (string-append prefix-raw \".\")))\n"
        "         (st (symbol-table))\n"
        "         (prefix-len (string-length prefix))\n"
        "         (bindings '()))\n"
        "    (for-each\n"
        "      (lambda (sym)\n"
        "        (let ((sym-str (symbol->string sym)))\n"
        "          (if (and (> (string-length sym-str) prefix-len)\n"
        "                   (string=? (substring sym-str 0 prefix-len) prefix)\n"
        "                   (defined? sym))\n"
        "              (let ((new-sym (string->symbol (substring sym-str prefix-len))))\n"
        "                (unless (member new-sym protected-syms)\n"
        "                  (set! bindings (cons `',new-sym (cons sym bindings))))))))\n"
        "      st)\n"
        "    `(with-let (sublet (curlet) ,@bindings)\n"
        "       ,@body)))\n"
    );
}
