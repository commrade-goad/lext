/* lext_da.c — Item #4: Dynamic array management in C
 *              Item #5: Directory walker in C
 */

#include "lext_da.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#ifndef _WIN32
#  include <sys/types.h>
#  include <dirent.h>
#  include <sys/stat.h>
#  include <unistd.h>
#else
#  include <windows.h>
#endif

/* ------------------------------------------------------------------ */
/* Nob_Dynamic_Array layout (64-bit):
 *   offset  0: void *items
 *   offset  8: uint64_t count
 *   offset 16: uint64_t capacity
 * ------------------------------------------------------------------ */

/* Helper: read/write the three fields directly */
static inline void   *da_items   (void *da) { return *(void **)((char *)da +  0); }
static inline uint64_t da_count  (void *da) { return *(uint64_t *)((char *)da + 8); }
static inline uint64_t da_capacity(void *da){ return *(uint64_t *)((char *)da +16); }

static inline void da_set_items   (void *da, void    *v){ *(void **)   ((char *)da+ 0) = v; }
static inline void da_set_count   (void *da, uint64_t v){ *(uint64_t *)((char *)da+ 8) = v; }
static inline void da_set_capacity(void *da, uint64_t v){ *(uint64_t *)((char *)da+16) = v; }

/* ------------------------------------------------------------------ */
/* (lext-da-reserve da-ptr expected-capacity element-size)             */
/* ------------------------------------------------------------------ */
static s7_pointer builtin_da_reserve(s7_scheme *sc, s7_pointer args) {
    s7_pointer da_arg  = s7_car(args);
    s7_pointer cap_arg = s7_cadr(args);
    s7_pointer esz_arg = s7_caddr(args);

    if (!s7_is_c_pointer(da_arg))
        return s7_wrong_type_arg_error(sc, "lext-da-reserve", 1, da_arg, "c-pointer");
    if (!s7_is_integer(cap_arg))
        return s7_wrong_type_arg_error(sc, "lext-da-reserve", 2, cap_arg, "integer");
    if (!s7_is_integer(esz_arg))
        return s7_wrong_type_arg_error(sc, "lext-da-reserve", 3, esz_arg, "integer");

    void    *da       = s7_c_pointer(da_arg);
    uint64_t expected = (uint64_t)s7_integer(cap_arg);
    uint64_t elem_sz  = (uint64_t)s7_integer(esz_arg);

    if (!da) return s7_unspecified(sc);

    uint64_t capacity = da_capacity(da);
    if (expected <= capacity) return s7_unspecified(sc);

    /* Grow capacity: start at 256, then double */
    uint64_t new_cap = (capacity == 0) ? 256 : capacity;
    while (new_cap < expected) new_cap *= 2;

    void *items = da_items(da);
    void *new_items = realloc(items, new_cap * elem_sz);
    if (!new_items)
        return s7_error(sc, s7_make_symbol(sc, "memory-error"),
                        s7_list(sc, 1, s7_make_string(sc, "lext-da-reserve: realloc failed")));

    da_set_items(da, new_items);
    da_set_capacity(da, new_cap);
    return s7_unspecified(sc);
}

/* ------------------------------------------------------------------ */
/* (lext-da-append-raw da-ptr element-size value-ptr)                  */
/* ------------------------------------------------------------------ */
static s7_pointer builtin_da_append_raw(s7_scheme *sc, s7_pointer args) {
    s7_pointer da_arg  = s7_car(args);
    s7_pointer esz_arg = s7_cadr(args);
    s7_pointer val_arg = s7_caddr(args);

    if (!s7_is_c_pointer(da_arg))
        return s7_wrong_type_arg_error(sc, "lext-da-append-raw", 1, da_arg, "c-pointer");
    if (!s7_is_integer(esz_arg))
        return s7_wrong_type_arg_error(sc, "lext-da-append-raw", 2, esz_arg, "integer");
    if (!s7_is_c_pointer(val_arg))
        return s7_wrong_type_arg_error(sc, "lext-da-append-raw", 3, val_arg, "c-pointer");

    void    *da      = s7_c_pointer(da_arg);
    uint64_t elem_sz = (uint64_t)s7_integer(esz_arg);
    void    *val_ptr = s7_c_pointer(val_arg);

    if (!da) return s7_unspecified(sc);

    uint64_t count    = da_count(da);
    uint64_t capacity = da_capacity(da);

    if (count + 1 > capacity) {
        uint64_t new_cap = (capacity == 0) ? 256 : capacity * 2;
        void *items = da_items(da);
        void *new_items = realloc(items, new_cap * elem_sz);
        if (!new_items)
            return s7_error(sc, s7_make_symbol(sc, "memory-error"),
                            s7_list(sc, 1, s7_make_string(sc,
                                "lext-da-append-raw: realloc failed")));
        da_set_items(da, new_items);
        da_set_capacity(da, new_cap);
    }

    void *items = da_items(da);
    memcpy((char *)items + count * elem_sz, val_ptr, elem_sz);
    da_set_count(da, count + 1);
    return s7_unspecified(sc);
}

/* ------------------------------------------------------------------ */
/* (lext-walk-dir root-path proc level)                                */
/* proc is called as (proc path type-sym level)                        */
/* Returns #t on success, #f on error                                  */
/* ------------------------------------------------------------------ */

#ifndef _WIN32
static s7_pointer walk_dir_posix(s7_scheme *sc, const char *root,
                                 s7_pointer proc, int level) {
    DIR *d = opendir(root);
    if (!d) return s7_f(sc);

    struct dirent *ent;
    s7_pointer result = s7_t(sc);

    while ((ent = readdir(d)) != NULL) {
        const char *name = ent->d_name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;

        /* Build full path */
        size_t root_len = strlen(root);
        size_t name_len = strlen(name);
        char *full = (char *)malloc(root_len + 1 + name_len + 1);
        memcpy(full, root, root_len);
        full[root_len] = '/';
        memcpy(full + root_len + 1, name, name_len);
        full[root_len + 1 + name_len] = '\0';

        /* Determine type */
        s7_pointer type_sym;
        struct stat st;
        int stat_ok = lstat(full, &st);
        if (stat_ok != 0) {
            type_sym = s7_make_symbol(sc, "other");
        } else if (S_ISREG(st.st_mode)) {
            type_sym = s7_make_symbol(sc, "regular");
        } else if (S_ISDIR(st.st_mode)) {
            type_sym = s7_make_symbol(sc, "directory");
        } else if (S_ISLNK(st.st_mode)) {
            type_sym = s7_make_symbol(sc, "symlink");
        } else {
            type_sym = s7_make_symbol(sc, "other");
        }

        /* Call proc */
        s7_pointer cont = s7_call(sc, proc,
                                  s7_list(sc, 3,
                                          s7_make_string(sc, full),
                                          type_sym,
                                          s7_make_integer(sc, level)));

        if (s7_is_boolean(cont) && !s7_boolean(sc, cont)) {
            free(full);
            result = s7_f(sc);
            break;
        }

        /* Recurse into directories */
        if (s7_is_boolean(type_sym) || (s7_is_symbol(type_sym) &&
            strcmp(s7_symbol_name(type_sym), "directory") == 0)) {
            s7_pointer sub = walk_dir_posix(sc, full, proc, level + 1);
            if (s7_is_boolean(sub) && !s7_boolean(sc, sub)) {
                free(full);
                result = s7_f(sc);
                break;
            }
        }
        /* Non-directory: just recurse if it's a directory */
        if (strcmp(s7_symbol_name(type_sym), "directory") == 0) {
            /* already handled above by the check on type_sym name */
        }

        free(full);
    }
    closedir(d);
    return result;
}

/* Cleaner recursion — separate from above */
static s7_pointer do_walk_dir(s7_scheme *sc, const char *root,
                              s7_pointer proc, int level) {
    DIR *d = opendir(root);
    if (!d) return s7_f(sc);

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        const char *name = ent->d_name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;

        size_t root_len = strlen(root);
        size_t name_len = strlen(name);
        char *full = (char *)malloc(root_len + 1 + name_len + 1);
        if (!full) { closedir(d); return s7_f(sc); }
        memcpy(full, root, root_len);
        full[root_len] = '/';
        memcpy(full + root_len + 1, name, name_len + 1);

        s7_pointer type_sym;
        struct stat st;
        int is_dir = 0;
        if (lstat(full, &st) == 0) {
            if      (S_ISREG(st.st_mode))  { type_sym = s7_make_symbol(sc, "regular");   }
            else if (S_ISDIR(st.st_mode))  { type_sym = s7_make_symbol(sc, "directory"); is_dir = 1; }
            else if (S_ISLNK(st.st_mode))  { type_sym = s7_make_symbol(sc, "symlink");   }
            else                            { type_sym = s7_make_symbol(sc, "other");     }
        } else {
            type_sym = s7_make_symbol(sc, "other");
        }

        s7_pointer cont = s7_call(sc, proc,
                                  s7_list(sc, 3,
                                          s7_make_string(sc, full),
                                          type_sym,
                                          s7_make_integer(sc, level)));

        int stop = (s7_is_boolean(cont) && !s7_boolean(sc, cont));
        if (!stop && is_dir) {
            s7_pointer sub = do_walk_dir(sc, full, proc, level + 1);
            stop = (s7_is_boolean(sub) && !s7_boolean(sc, sub));
        }
        free(full);
        if (stop) { closedir(d); return s7_f(sc); }
    }
    closedir(d);
    return s7_t(sc);
}

#endif /* !_WIN32 */

static s7_pointer builtin_walk_dir(s7_scheme *sc, s7_pointer args) {
    s7_pointer root_arg  = s7_car(args);
    s7_pointer proc_arg  = s7_cadr(args);
    s7_pointer level_arg = s7_caddr(args);

    if (!s7_is_string(root_arg))
        return s7_wrong_type_arg_error(sc, "lext-walk-dir", 1, root_arg, "string");
    if (!s7_is_procedure(proc_arg))
        return s7_wrong_type_arg_error(sc, "lext-walk-dir", 2, proc_arg, "procedure");

    const char *root = s7_string(root_arg);
    int level = s7_is_integer(level_arg) ? (int)s7_integer(level_arg) : 0;

#ifndef _WIN32
    return do_walk_dir(sc, root, proc_arg, level);
#else
    (void)level;
    return s7_error(sc, s7_make_symbol(sc, "not-implemented"),
                    s7_list(sc, 1, s7_make_string(sc,
                        "lext-walk-dir: Windows not yet implemented")));
#endif
}

/* ------------------------------------------------------------------ */
/* Registration                                                         */
/* ------------------------------------------------------------------ */

void lext_da_register(s7_scheme *sc) {
    s7_define_function(sc, "lext-da-reserve", builtin_da_reserve, 3, 0, false,
                       "(lext-da-reserve da-ptr expected-capacity element-size) grows da if needed");
    s7_define_function(sc, "lext-da-append-raw", builtin_da_append_raw, 3, 0, false,
                       "(lext-da-append-raw da-ptr element-size val-ptr) appends raw bytes to da");
    s7_define_function(sc, "lext-walk-dir", builtin_walk_dir, 3, 0, false,
                       "(lext-walk-dir root proc level) walks directory tree calling proc");
}
