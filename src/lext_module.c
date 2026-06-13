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

static s7_pointer builtin_use_lib(s7_scheme *sc, s7_pointer args) {
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
                if (meow_hash_table_get(lext_loaded_modules, key)) {
                    loaded = true;
                    break;
                }
                meow_hash_table_set(lext_loaded_modules, key, (void *)1);
                s7_load(sc, (const char *)path.dt);
                loaded = true;
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

void lext_module_register(s7_scheme *sc) {
    s7_define_function(sc, "use", builtin_use_lib, 1, 0, true,
                       "(use lib...) loads lib.lext from LEXT_HOME");
}
