#include "lext_module.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#if !defined(_WIN32)
#  include <unistd.h>
#else
#  include <windows.h>
#endif

#include "helpa.h"

MeowHashTable *lext_loaded_modules = NULL;

void lext_module_init(void) {
    lext_loaded_modules = meow_hash_table_new();
}

/* Split `str` on `c` into an ArrHsv dynamic array */
typedef struct { HStrView *dt; size_t sz; size_t cp; } ArrHsv;

static void split_on_char(ArrHsv *a, const char *str, char c) {
    size_t len = strlen(str);
    size_t j = 0;
    for (size_t i = 0; i <= len; i++) {
        if (str[i] == c || str[i] == '\0') {
            if (i > j) {
                HStrView view = { .dt = (const u8 *)&str[j], .sz = i - j };
                helpa_da_append(*a, view);
            }
            j = i + 1;
        }
    }
}

static void export_bindings_to_current(s7_scheme *sc, s7_pointer env, s7_pointer target_env, const char *prefix, const char *lib_str) {
    s7_pointer exports_sym = s7_make_symbol(sc, "*exports*");
    s7_pointer exports = s7_symbol_local_value(sc, exports_sym, env);
    bool has_exports = (exports != s7_undefined(sc) && s7_is_list(sc, exports));

    s7_pointer bindings = s7_let_to_list(sc, env);
    s7_pointer curr_binding = bindings;

    while (s7_is_pair(curr_binding)) {
        s7_pointer entry = s7_car(curr_binding);
        s7_pointer sym = s7_car(entry);
        s7_pointer val = s7_cdr(entry);

        const char *sym_name = s7_symbol_name(sym);
        if (strchr(sym_name, '.') == NULL) {
            bool should_export = true;
            if (has_exports) {
                should_export = (s7_memq(sc, sym, exports) != s7_f(sc));
            } else {
                should_export = (strncmp(sym_name, "internal-", 9) != 0);
            }

            if (should_export) {
                char *new_sym_name = malloc(strlen(prefix) + 1 + strlen(sym_name) + 1);
                sprintf(new_sym_name, "%s.%s", prefix, sym_name);

                s7_pointer new_sym = s7_make_symbol(sc, new_sym_name);
                s7_define(sc, target_env, new_sym, val);

                free(new_sym_name);
            }
        }
        curr_binding = s7_cdr(curr_binding);
    }
}

static s7_pointer builtin_use_lib(s7_scheme *sc, s7_pointer args) {
    s7_pointer caller_env = s7_curlet(sc);
    const char *env = getenv("LEXT_HOME");
    if (!env)
        return s7_error(sc,
                        s7_make_symbol(sc, "env-error"),
                        s7_make_string(sc, "LEXT_HOME is not set"));

    s7_pointer curr = args;
    while (s7_is_pair(curr)) {
        s7_pointer lib = s7_car(curr);
        const char *lib_str = NULL;

        if      (s7_is_string(lib))  lib_str = s7_string(lib);
        else if (s7_is_symbol(lib))  lib_str = s7_symbol_name(lib);
        else return s7_error(sc,
                             s7_make_symbol(sc, "use-error"),
                             s7_make_string(sc, "module name must be a string or symbol"));

        ArrHsv paths = {0};
        split_on_char(&paths, env, ':');
        HStr path = {0};
        bool loaded = false;

        const char *last_slash = strrchr(lib_str, '/');
        const char *prefix = last_slash ? last_slash + 1 : lib_str;

        for (size_t i = 0; i < paths.sz; i++) {
            HStrView current = paths.dt[i];
            hstr_clear(&path);
            hstr_printf(&path, "%.*s/%s/lib.lext",
                        (int)current.sz, (const char *)current.dt, lib_str);
#if _WIN32
            int exist = GetFileAttributesA((const char *)path.dt) != INVALID_FILE_ATTRIBUTES;
#else
            int exist = access((const char *)path.dt, F_OK) == 0;
#endif
            if (exist) {
                MeowHash key = meow_hash_string((const char *)path.dt);
                s7_pointer cached_env = (s7_pointer)meow_hash_table_get(lext_loaded_modules, key);
                if (cached_env != NULL) {
                    export_bindings_to_current(sc, cached_env, caller_env, prefix, lib_str);
                    loaded = true;
                    break;
                }

                s7_pointer new_env = s7_sublet(sc, caller_env, s7_nil(sc));
                s7_int gc_loc = s7_gc_protect(sc, new_env);

                s7_pointer load_res = s7_load_with_environment(sc, (const char *)path.dt, new_env);
                if (load_res != NULL) {
                    meow_hash_table_set(lext_loaded_modules, key, (void *)new_env);
                    export_bindings_to_current(sc, new_env, caller_env, prefix, lib_str);
                    loaded = true;
                } else {
                    loaded = false;
                    s7_gc_unprotect_at(sc, gc_loc);
                }
                break;
            }
        }

        hstr_free(&path);
        helpa_da_free(paths);

        if (!loaded) {
            HStr err_msg = {0};
            hstr_printf(&err_msg, "module '%s' not found in LEXT_HOME", lib_str);
            s7_pointer err = s7_error(sc,
                                      s7_make_symbol(sc, "use-error"),
                                      s7_make_string(sc, (const char *)err_msg.dt));
            hstr_free(&err_msg);
            return err;
        }

        curr = s7_cdr(curr);
    }
    return s7_unspecified(sc);
}

static s7_pointer builtin_export_macro(s7_scheme *sc, s7_pointer args) {
    return s7_list(sc, 3,
                   s7_make_symbol(sc, "define"),
                   s7_make_symbol(sc, "*exports*"),
                   s7_list(sc, 2, s7_make_symbol(sc, "quote"), args));
}

void lext_module_register(s7_scheme *sc) {
    s7_define_function(sc, "use", builtin_use_lib, 1, 0, true,
                       "(use lib...) loads lib.lext from LEXT_HOME");
    s7_define_macro(sc, "export", builtin_export_macro, 0, 0, true,
                    "(export sym...) defines the public exports for a module");
}
