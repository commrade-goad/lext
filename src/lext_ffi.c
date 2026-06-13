#include "lext_ffi.h"
#include "lext_types.h"
#include "lext_io.h"
#include "lext_hash.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>
#include <ffi.h>

/* ------------------------------------------------------------------ */
/* Bounds-check registry (#7)                                          */
/* ------------------------------------------------------------------ */

static MeowHashTable *ptr_bounds_table = NULL;

static s7_pointer builtin_malloc_tracked(s7_scheme *sc, s7_pointer args) {
    s7_pointer sz_arg = s7_car(args);
    if (!s7_is_integer(sz_arg))
        return s7_wrong_type_arg_error(sc, "lext-malloc-tracked", 1, sz_arg, "integer");
    size_t sz = (size_t)s7_integer(sz_arg);
    void *p = malloc(sz);
    if (!p && sz != 0)
        return s7_error(sc, s7_make_symbol(sc, "memory-error"),
                        s7_list(sc, 1, s7_make_string(sc, "lext-malloc-tracked: malloc failed")));
    if (ptr_bounds_table && p) {
        MeowHash key = meow_hash_string(NULL);
        /* Key by address — convert address to hex string */
        char addr_buf[32];
        snprintf(addr_buf, sizeof(addr_buf), "%p", p);
        key = meow_hash_string(addr_buf);
        meow_hash_table_set(ptr_bounds_table, key, (void *)(uintptr_t)sz);
    }
    return s7_make_c_pointer(sc, p);
}

static s7_pointer builtin_free_tracked(s7_scheme *sc, s7_pointer args) {
    s7_pointer ptr_arg = s7_car(args);
    void *p = NULL;
    if (s7_is_c_pointer(ptr_arg)) p = s7_c_pointer(ptr_arg);
    else if (s7_is_null(sc, ptr_arg)) return s7_unspecified(sc);
    else return s7_wrong_type_arg_error(sc, "lext-free-tracked", 1, ptr_arg, "c-pointer");
    if (ptr_bounds_table && p) {
        char addr_buf[32];
        snprintf(addr_buf, sizeof(addr_buf), "%p", p);
        /* Remove from table by setting to NULL (entry stays, size becomes 0) */
        MeowHash key = meow_hash_string(addr_buf);
        meow_hash_table_set(ptr_bounds_table, key, NULL);
    }
    free(p);
    return s7_unspecified(sc);
}

static s7_pointer builtin_bounds_check(s7_scheme *sc, s7_pointer args) {
    s7_pointer ptr_arg = s7_car(args);
    s7_pointer sz_arg  = s7_cadr(args);
    if (!s7_is_c_pointer(ptr_arg)) return s7_f(sc);
    if (!s7_is_integer(sz_arg))    return s7_f(sc);
    void *p = s7_c_pointer(ptr_arg);
    if (!ptr_bounds_table || !p) return s7_f(sc);
    char addr_buf[32];
    snprintf(addr_buf, sizeof(addr_buf), "%p", p);
    MeowHash key = meow_hash_string(addr_buf);
    void *stored = meow_hash_table_get(ptr_bounds_table, key);
    if (!stored) return s7_f(sc);
    size_t reg_sz = (size_t)(uintptr_t)stored;
    size_t req_sz = (size_t)s7_integer(sz_arg);
    return (req_sz <= reg_sz) ? s7_t(sc) : s7_f(sc);
}

static s7_pointer builtin_register_bounds(s7_scheme *sc, s7_pointer args) {
    s7_pointer ptr_arg = s7_car(args);
    s7_pointer sz_arg  = s7_cadr(args);
    if (!s7_is_c_pointer(ptr_arg))
        return s7_wrong_type_arg_error(sc, "lext-register-bounds", 1, ptr_arg, "c-pointer");
    if (!s7_is_integer(sz_arg))
        return s7_wrong_type_arg_error(sc, "lext-register-bounds", 2, sz_arg, "integer");
    void *p = s7_c_pointer(ptr_arg);
    size_t sz = (size_t)s7_integer(sz_arg);
    if (ptr_bounds_table && p) {
        char addr_buf[32];
        snprintf(addr_buf, sizeof(addr_buf), "%p", p);
        MeowHash key = meow_hash_string(addr_buf);
        meow_hash_table_set(ptr_bounds_table, key, (void *)(uintptr_t)sz);
    }
    return s7_unspecified(sc);
}

static s7_pointer builtin_unregister_bounds(s7_scheme *sc, s7_pointer args) {
    s7_pointer ptr_arg = s7_car(args);
    void *p = NULL;
    if (s7_is_c_pointer(ptr_arg)) p = s7_c_pointer(ptr_arg);
    else if (s7_is_null(sc, ptr_arg)) return s7_unspecified(sc);
    else return s7_wrong_type_arg_error(sc, "lext-unregister-bounds", 1, ptr_arg, "c-pointer");
    if (ptr_bounds_table && p) {
        char addr_buf[32];
        snprintf(addr_buf, sizeof(addr_buf), "%p", p);
        MeowHash key = meow_hash_string(addr_buf);
        meow_hash_table_set(ptr_bounds_table, key, NULL);
    }
    return s7_unspecified(sc);
}

static s7_pointer builtin_registered_size(s7_scheme *sc, s7_pointer args) {
    s7_pointer ptr_arg = s7_car(args);
    void *p = NULL;
    if (s7_is_c_pointer(ptr_arg)) p = s7_c_pointer(ptr_arg);
    else if (s7_is_null(sc, ptr_arg)) return s7_make_integer(sc, -1);
    else return s7_wrong_type_arg_error(sc, "lext-registered-size", 1, ptr_arg, "c-pointer");

    if (!ptr_bounds_table || !p) return s7_make_integer(sc, -1);
    char addr_buf[32];
    snprintf(addr_buf, sizeof(addr_buf), "%p", p);
    MeowHash key = meow_hash_string(addr_buf);
    void *stored = meow_hash_table_get(ptr_bounds_table, key);
    if (!stored) return s7_make_integer(sc, -1);
    return s7_make_integer(sc, (s7_int)(uintptr_t)stored);
}



/* ------------------------------------------------------------------ */
/* Dynamic library helpers                                              */
/* ------------------------------------------------------------------ */

static s7_pointer builtin_ffi_open(s7_scheme *sc, s7_pointer args) {
    s7_pointer path_arg = s7_car(args);
    const char *path = s7_is_string(path_arg) ? s7_string(path_arg) : NULL;
    void *handle = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
    if (!handle)
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2,
                                s7_make_string(sc, "ffi-open failed: ~A"),
                                s7_make_string(sc, dlerror())));
    return s7_make_c_pointer(sc, handle);
}

/* ------------------------------------------------------------------ */
/* #8: ffi-open-lib — cross-platform logical library name resolver      */
/* ------------------------------------------------------------------ */

static s7_pointer builtin_ffi_open_lib(s7_scheme *sc, s7_pointer args) {
    s7_pointer name_arg = s7_car(args);
    if (!s7_is_string(name_arg))
        return s7_wrong_type_arg_error(sc, "ffi-open-lib", 1, name_arg, "string");
    const char *name = s7_string(name_arg);

#if defined(_WIN32) || defined(_WIN64)
    /* Windows: NAME.dll */
    static const char *suffixes[] = { ".dll", NULL };
    static const char *prefixes[] = { "", NULL };
#elif defined(__APPLE__) || defined(__MACH__)
    /* macOS: libNAME.dylib */
    static const char *suffixes[] = { ".dylib", ".so", NULL };
    static const char *prefixes[] = { "lib", "", NULL };
#else
    /* Linux / BSD: libNAME.so, libNAME.so.0 ... */
    static const char *suffixes[] = { ".so", ".so.0", ".so.1", ".so.2",
                                      ".so.3", ".so.4", ".so.5", ".so.6", NULL };
    static const char *prefixes[] = { "lib", "", NULL };
#endif

    char path[1024];
    for (int pi = 0; prefixes[pi] != NULL; pi++) {
        for (int si = 0; suffixes[si] != NULL; si++) {
            snprintf(path, sizeof(path), "%s%s%s", prefixes[pi], name, suffixes[si]);
            void *handle = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
            if (handle) return s7_make_c_pointer(sc, handle);
        }
    }
    /* Last attempt: the name exactly as given */
    void *handle = dlopen(name, RTLD_LAZY | RTLD_GLOBAL);
    if (handle) return s7_make_c_pointer(sc, handle);

    return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                    s7_list(sc, 2,
                            s7_make_string(sc, "ffi-open-lib: could not open library ~S"),
                            name_arg));
}

static s7_pointer builtin_ffi_sym(s7_scheme *sc, s7_pointer args) {
    s7_pointer handle_arg = s7_car(args);
    s7_pointer name_arg   = s7_cadr(args);
    if (!s7_is_c_pointer(handle_arg))
        return s7_wrong_type_arg_error(sc, "ffi-sym", 1, handle_arg, "c-pointer");
    if (!s7_is_string(name_arg))
        return s7_wrong_type_arg_error(sc, "ffi-sym", 2, name_arg, "string");
    void *handle = s7_c_pointer(handle_arg);
    const char *name = s7_string(name_arg);
    void *sym = dlsym(handle, name);
    if (!sym)
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2,
                                s7_make_string(sc, "ffi-sym failed: ~A"),
                                s7_make_string(sc, dlerror())));
    return s7_make_c_pointer(sc, sym);
}

static s7_pointer builtin_ffi_close(s7_scheme *sc, s7_pointer args) {
    s7_pointer handle_arg = s7_car(args);
    if (!s7_is_c_pointer(handle_arg))
        return s7_wrong_type_arg_error(sc, "ffi-close", 1, handle_arg, "c-pointer");
    return s7_make_integer(sc, dlclose(s7_c_pointer(handle_arg)));
}

/* ------------------------------------------------------------------ */
/* ffi-call                                                             */
/* ------------------------------------------------------------------ */

/* Convenience union used for scalar argument buffers */
typedef union { int i; float f; double d; void *p; } FfiVal;

static s7_pointer builtin_ffi_call(s7_scheme *sc, s7_pointer args) {
    s7_pointer func_arg       = s7_car(args);
    s7_pointer ret_type_arg   = s7_cadr(args);
    s7_pointer arg_types_list = s7_caddr(args);
    s7_pointer arg_vals_list  = s7_cadddr(args);

    /* Optional 5th arg: nfixed (for variadic calls) */
    s7_pointer nfixed_arg = s7_nil(sc);
    {
        s7_pointer tmp = args;
        for (int i = 0; i < 4 && s7_is_pair(tmp); i++) tmp = s7_cdr(tmp);
        if (s7_is_pair(tmp)) nfixed_arg = s7_car(tmp);
    }

    if (!s7_is_c_pointer(func_arg))
        return s7_wrong_type_arg_error(sc, "ffi-call", 1, func_arg, "c-pointer");
    void *func = s7_c_pointer(func_arg);

    TypeAlloc *allocs = NULL;
    ffi_type *ret_type = lext_parse_ffi_type(sc, ret_type_arg, &allocs);
    if (!ret_type) {
        lext_free_allocs(allocs);
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2,
                                s7_make_string(sc, "invalid return type: ~S"),
                                ret_type_arg));
    }

    int nargs  = s7_list_length(sc, arg_vals_list);
    int ntypes = s7_list_length(sc, arg_types_list);
    int nfixed = s7_is_integer(nfixed_arg) ? (int)s7_integer(nfixed_arg) : -1;

    if (nfixed > 0) {
        if (ntypes < nfixed) {
            lext_free_allocs(allocs);
            return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                            s7_list(sc, 1, s7_make_string(sc,
                                "types list must cover at least the fixed arguments")));
        }
        if (nfixed > nargs) {
            lext_free_allocs(allocs);
            return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                            s7_list(sc, 1, s7_make_string(sc,
                                "nfixed cannot be greater than total arguments count")));
        }
    } else {
        if (ntypes != nargs) {
            lext_free_allocs(allocs);
            return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                            s7_list(sc, 1, s7_make_string(sc,
                                "argument types and values count mismatch")));
        }
    }

    ffi_type  **arg_types          = malloc(nargs * sizeof(ffi_type *));
    s7_pointer *resolved_type_descs = malloc(nargs * sizeof(s7_pointer));
    s7_pointer t_curr = arg_types_list;

    for (int i = 0; i < nargs; i++) {
        if (i < ntypes) {
            s7_pointer t_sym = s7_car(t_curr);
            ffi_type *t = lext_parse_ffi_type(sc, t_sym, &allocs);
            if (!t) {
                free(arg_types); free(resolved_type_descs);
                lext_free_allocs(allocs);
                return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                                s7_list(sc, 2,
                                        s7_make_string(sc, "invalid argument type: ~S"),
                                        t_sym));
            }
            arg_types[i]           = t;
            resolved_type_descs[i] = t_sym;
            t_curr = s7_cdr(t_curr);
        } else {
            /* Variadic: infer type from value */
            s7_pointer val = s7_list_ref(sc, arg_vals_list, i);
            if      (s7_is_integer(val) || s7_is_boolean(val) || s7_is_character(val)) {
                arg_types[i]           = &ffi_type_sint;
                resolved_type_descs[i] = s7_make_symbol(sc, "int");
            } else if (s7_is_real(val)) {
                arg_types[i]           = &ffi_type_double;
                resolved_type_descs[i] = s7_make_symbol(sc, "double");
            } else if (s7_is_string(val)) {
                arg_types[i]           = &ffi_type_pointer;
                resolved_type_descs[i] = s7_make_symbol(sc, "string");
            } else if (s7_is_c_pointer(val) || s7_is_null(sc, val) ||
                       (s7_is_pair(val) && s7_is_c_pointer(s7_cdr(val)))) {
                arg_types[i]           = &ffi_type_pointer;
                resolved_type_descs[i] = s7_make_symbol(sc, "pointer");
            } else {
                free(arg_types); free(resolved_type_descs);
                lext_free_allocs(allocs);
                return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                                s7_list(sc, 2,
                                        s7_make_string(sc,
                                            "could not infer type for variadic argument: ~S"),
                                        val));
            }
        }
    }

    ffi_cif cif;
    ffi_status prep_status = (nfixed > 0)
        ? ffi_prep_cif_var(&cif, FFI_DEFAULT_ABI, nfixed, nargs, ret_type, arg_types)
        : ffi_prep_cif    (&cif, FFI_DEFAULT_ABI,          nargs, ret_type, arg_types);

    if (prep_status != FFI_OK) {
        free(arg_types); free(resolved_type_descs);
        lext_free_allocs(allocs);
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 1, s7_make_string(sc,
                            nfixed > 0 ? "ffi_prep_cif_var failed" : "ffi_prep_cif failed")));
    }

    void **arg_values = malloc(nargs * sizeof(void *));
    void **arg_data   = malloc(nargs * sizeof(void *));
    s7_pointer v_curr = arg_vals_list;

    for (int i = 0; i < nargs; i++) {
        ffi_type *t   = arg_types[i];
        s7_pointer val   = s7_car(v_curr);
        s7_pointer t_sym = resolved_type_descs[i];

        if (t->type == FFI_TYPE_STRUCT) {
            void *struct_buf = malloc(t->size);
            memset(struct_buf, 0, t->size);
            if (lext_write_val(sc, struct_buf, t_sym, t, val) < 0) {
                for (int j = 0; j < i; j++) free(arg_data[j]);
                free(arg_data); free(arg_values); free(arg_types); free(resolved_type_descs);
                lext_free_allocs(allocs); free(struct_buf);
                return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                                s7_list(sc, 2,
                                        s7_make_string(sc, "failed to write argument: ~S"), val));
            }
            arg_data[i]   = struct_buf;
            arg_values[i] = struct_buf;
        } else {
            FfiVal *p_val = malloc(sizeof(FfiVal));
            memset(p_val, 0, sizeof(FfiVal));
            if (lext_write_val(sc, p_val, t_sym, t, val) < 0) {
                for (int j = 0; j < i; j++) free(arg_data[j]);
                free(arg_data); free(arg_values); free(arg_types); free(resolved_type_descs);
                lext_free_allocs(allocs); free(p_val);
                return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                                s7_list(sc, 2,
                                        s7_make_string(sc, "failed to write argument: ~S"), val));
            }
            arg_data[i]   = p_val;
            arg_values[i] = p_val;
        }
        v_curr = s7_cdr(v_curr);
    }

    size_t ret_buf_size = ret_type->size > sizeof(ffi_arg) ? ret_type->size : sizeof(ffi_arg);
    void  *ret_buf      = malloc(ret_buf_size);
    memset(ret_buf, 0, ret_buf_size);

    ffi_call(&cif, FFI_FN(func), ret_buf, arg_values);

    s7_pointer result = (ret_type == &ffi_type_void)
        ? s7_unspecified(sc)
        : lext_read_val(sc, ret_buf, ret_type_arg, ret_type);

    for (int i = 0; i < nargs; i++) free(arg_data[i]);
    free(arg_data); free(arg_values); free(arg_types); free(resolved_type_descs); free(ret_buf);
    lext_free_allocs(allocs);
    return result;
}

/* ------------------------------------------------------------------ */
/* ffi-deref, ffi-set!, ffi-size, ffi-align                           */
/* ------------------------------------------------------------------ */

static s7_pointer builtin_ffi_deref(s7_scheme *sc, s7_pointer args) {
    s7_pointer ptr_arg  = s7_car(args);
    s7_pointer type_arg = s7_cadr(args);
    if (!s7_is_c_pointer(ptr_arg))
        return s7_wrong_type_arg_error(sc, "ffi-deref", 1, ptr_arg, "c-pointer");
    void *ptr = s7_c_pointer(ptr_arg);
    if (!ptr)
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 1, s7_make_string(sc, "dereferencing NULL pointer")));

    TypeAlloc *allocs = NULL;
    ffi_type  *ft     = lext_parse_ffi_type(sc, type_arg, &allocs);
    if (!ft) {
        lext_free_allocs(allocs);
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2,
                                s7_make_string(sc, "invalid type descriptor: ~S"),
                                type_arg));
    }
    if (ft->type == FFI_TYPE_STRUCT && ft->size == 0) {
        ffi_cif dummy; ffi_prep_cif(&dummy, FFI_DEFAULT_ABI, 0, ft, NULL);
    }
    s7_pointer result = lext_read_val(sc, ptr, type_arg, ft);
    lext_free_allocs(allocs);
    return result;
}

static s7_pointer builtin_ffi_set_bang(s7_scheme *sc, s7_pointer args) {
    s7_pointer ptr_arg  = s7_car(args);
    s7_pointer type_arg = s7_cadr(args);
    s7_pointer val_arg  = s7_caddr(args);
    if (!s7_is_c_pointer(ptr_arg))
        return s7_wrong_type_arg_error(sc, "ffi-set!", 1, ptr_arg, "c-pointer");
    void *ptr = s7_c_pointer(ptr_arg);
    if (!ptr)
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 1, s7_make_string(sc, "writing to NULL pointer")));

    TypeAlloc *allocs = NULL;
    ffi_type  *ft     = lext_parse_ffi_type(sc, type_arg, &allocs);
    if (!ft) {
        lext_free_allocs(allocs);
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2,
                                s7_make_string(sc, "invalid type descriptor: ~S"),
                                type_arg));
    }
    if (ft->type == FFI_TYPE_STRUCT && ft->size == 0) {
        ffi_cif dummy; ffi_prep_cif(&dummy, FFI_DEFAULT_ABI, 0, ft, NULL);
    }
    int res = lext_write_val(sc, ptr, type_arg, ft, val_arg);
    lext_free_allocs(allocs);
    if (res < 0)
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2,
                                s7_make_string(sc, "failed to write value: ~S"),
                                val_arg));
    return s7_unspecified(sc);
}

static s7_pointer builtin_ffi_size(s7_scheme *sc, s7_pointer args) {
    s7_pointer type_arg = s7_car(args);
    TypeAlloc *allocs   = NULL;
    ffi_type  *ft       = lext_parse_ffi_type(sc, type_arg, &allocs);
    if (!ft) {
        lext_free_allocs(allocs);
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2,
                                s7_make_string(sc, "invalid type descriptor: ~S"),
                                type_arg));
    }
    if (ft->type == FFI_TYPE_STRUCT && ft->size == 0) {
        ffi_cif dummy; ffi_prep_cif(&dummy, FFI_DEFAULT_ABI, 0, ft, NULL);
    }
    s7_int size = ft->size;
    lext_free_allocs(allocs);
    return s7_make_integer(sc, size);
}

static s7_pointer builtin_ffi_align(s7_scheme *sc, s7_pointer args) {
    s7_pointer type_arg = s7_car(args);
    TypeAlloc *allocs   = NULL;
    ffi_type  *ft       = lext_parse_ffi_type(sc, type_arg, &allocs);
    if (!ft) {
        lext_free_allocs(allocs);
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2,
                                s7_make_string(sc, "invalid type descriptor: ~S"),
                                type_arg));
    }
    if (ft->type == FFI_TYPE_STRUCT && ft->size == 0) {
        ffi_cif dummy; ffi_prep_cif(&dummy, FFI_DEFAULT_ABI, 0, ft, NULL);
    }
    s7_int align = ft->alignment;
    lext_free_allocs(allocs);
    return s7_make_integer(sc, align);
}

/* ------------------------------------------------------------------ */
/* Pointer <-> integer                                                  */
/* ------------------------------------------------------------------ */

static s7_pointer builtin_c_ptr_to_int(s7_scheme *sc, s7_pointer args) {
    s7_pointer p = s7_car(args);
    if (!s7_is_c_pointer(p))
        return s7_wrong_type_arg_error(sc, "c-pointer->integer", 1, p, "c-pointer");
    return s7_make_integer(sc, (s7_int)s7_c_pointer(p));
}

static s7_pointer builtin_int_to_c_ptr(s7_scheme *sc, s7_pointer args) {
    s7_pointer val = s7_car(args);
    if (!s7_is_integer(val))
        return s7_wrong_type_arg_error(sc, "integer->c-pointer", 1, val, "integer");
    return s7_make_c_pointer(sc, (void *)s7_integer(val));
}

/* ------------------------------------------------------------------ */
/* ffi-callback                                                         */
/* ------------------------------------------------------------------ */

typedef struct {
    s7_scheme  *sc;
    s7_int      proc_loc;
    s7_int      ret_type_loc;
    s7_int      arg_types_loc;
    ffi_cif     cif;
    ffi_type  **arg_types;
    TypeAlloc  *allocs;
} CallbackCtx;

static void closure_trampoline(ffi_cif *cif, void *ret, void **args, void *user_data) {
    CallbackCtx *ctx = (CallbackCtx *)user_data;
    s7_scheme   *sc  = ctx->sc;
    int nargs = cif->nargs;

    s7_pointer scheme_args  = s7_nil(sc);
    s7_pointer arg_types_list = s7_gc_protected_at(sc, ctx->arg_types_loc);
    s7_pointer curr_type      = arg_types_list;

    for (int i = 0; i < nargs; i++) {
        s7_pointer type_desc = s7_car(curr_type);
        s7_pointer val       = lext_read_val(sc, args[i], type_desc, cif->arg_types[i]);
        scheme_args = s7_cons(sc, val, scheme_args);
        curr_type   = s7_cdr(curr_type);
    }
    scheme_args = s7_reverse(sc, scheme_args);

    s7_pointer proc          = s7_gc_protected_at(sc, ctx->proc_loc);
    s7_pointer result        = s7_call(sc, proc, scheme_args);
    s7_pointer ret_type_desc = s7_gc_protected_at(sc, ctx->ret_type_loc);
    lext_write_val(sc, ret, ret_type_desc, cif->rtype, result);
}

static s7_pointer builtin_ffi_callback(s7_scheme *sc, s7_pointer args) {
    s7_pointer proc           = s7_car(args);
    s7_pointer ret_type_arg   = s7_cadr(args);
    s7_pointer arg_types_list = s7_caddr(args);

    if (!s7_is_procedure(proc))
        return s7_wrong_type_arg_error(sc, "ffi-callback", 1, proc, "procedure");

    TypeAlloc *allocs   = NULL;
    ffi_type  *ret_type = lext_parse_ffi_type(sc, ret_type_arg, &allocs);
    if (!ret_type) {
        lext_free_allocs(allocs);
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2,
                                s7_make_string(sc, "invalid callback return type: ~S"),
                                ret_type_arg));
    }

    int         nargs     = s7_list_length(sc, arg_types_list);
    ffi_type  **arg_types = malloc(nargs * sizeof(ffi_type *));
    s7_pointer  t_curr    = arg_types_list;

    for (int i = 0; i < nargs; i++) {
        s7_pointer t_sym = s7_car(t_curr);
        ffi_type  *t     = lext_parse_ffi_type(sc, t_sym, &allocs);
        if (!t) {
            free(arg_types);
            lext_free_allocs(allocs);
            return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                            s7_list(sc, 2,
                                    s7_make_string(sc, "invalid callback argument type: ~S"),
                                    t_sym));
        }
        arg_types[i] = t;
        t_curr = s7_cdr(t_curr);
    }

    CallbackCtx *ctx = malloc(sizeof(CallbackCtx));
    ctx->sc             = sc;
    ctx->proc_loc       = s7_gc_protect(sc, proc);
    ctx->ret_type_loc   = s7_gc_protect(sc, ret_type_arg);
    ctx->arg_types_loc  = s7_gc_protect(sc, arg_types_list);
    ctx->arg_types      = arg_types;
    ctx->allocs         = allocs;

    ffi_status prep_status = ffi_prep_cif(&ctx->cif, FFI_DEFAULT_ABI, nargs, ret_type, arg_types);
    if (prep_status != FFI_OK) {
        s7_gc_unprotect_at(sc, ctx->proc_loc);
        s7_gc_unprotect_at(sc, ctx->ret_type_loc);
        s7_gc_unprotect_at(sc, ctx->arg_types_loc);
        free(arg_types); free(ctx);
        lext_free_allocs(allocs);
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 1, s7_make_string(sc, "ffi_prep_cif failed for callback")));
    }

    void         *code_ptr = NULL;
    ffi_closure  *closure  = ffi_closure_alloc(sizeof(ffi_closure), &code_ptr);
    if (!closure) {
        s7_gc_unprotect_at(sc, ctx->proc_loc);
        s7_gc_unprotect_at(sc, ctx->ret_type_loc);
        s7_gc_unprotect_at(sc, ctx->arg_types_loc);
        free(arg_types); free(ctx);
        lext_free_allocs(allocs);
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 1, s7_make_string(sc, "ffi_closure_alloc failed")));
    }

    prep_status = ffi_prep_closure_loc(closure, &ctx->cif, closure_trampoline, ctx, code_ptr);
    if (prep_status != FFI_OK) {
        ffi_closure_free(closure);
        s7_gc_unprotect_at(sc, ctx->proc_loc);
        s7_gc_unprotect_at(sc, ctx->ret_type_loc);
        s7_gc_unprotect_at(sc, ctx->arg_types_loc);
        free(arg_types); free(ctx);
        lext_free_allocs(allocs);
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 1, s7_make_string(sc, "ffi_prep_closure_loc failed")));
    }

    return s7_make_c_pointer(sc, code_ptr);
}

/* ------------------------------------------------------------------ */
/* Registration                                                         */
/* ------------------------------------------------------------------ */

void lext_ffi_register(s7_scheme *sc) {
    /* Initialize bounds table */
    ptr_bounds_table = meow_hash_table_new();

    s7_define_function(sc, "ffi-open",    builtin_ffi_open,    1, 0, false,
                       "(ffi-open path) loads dynamic library");
    s7_define_function(sc, "ffi-open-lib", builtin_ffi_open_lib, 1, 0, false,
                       "(ffi-open-lib name) opens library by logical name (tries lib<name>.so etc.)");
    s7_define_function(sc, "ffi-sym",     builtin_ffi_sym,     2, 0, false,
                       "(ffi-sym handle name) finds symbol in library");
    s7_define_function(sc, "ffi-close",   builtin_ffi_close,   1, 0, false,
                       "(ffi-close handle) closes dynamic library");
    s7_define_function(sc, "ffi-call",    builtin_ffi_call,    4, 1, false,
                       "(ffi-call func ret-type arg-types arg-vals [nfixed]) invokes foreign function");
    s7_define_function(sc, "ffi-deref",   builtin_ffi_deref,   2, 0, false,
                       "(ffi-deref ptr type-desc) dereferences pointer");
    s7_define_function(sc, "ffi-set!",    builtin_ffi_set_bang,3, 0, false,
                       "(ffi-set! ptr type-desc value) writes value to pointer");
    s7_define_function(sc, "ffi-size",    builtin_ffi_size,    1, 0, false,
                       "(ffi-size type) returns size of FFI type in bytes");
    s7_define_function(sc, "ffi-align",   builtin_ffi_align,   1, 0, false,
                       "(ffi-align type) returns alignment of FFI type");
    s7_define_function(sc, "ffi-callback",builtin_ffi_callback,3, 0, false,
                       "(ffi-callback proc ret-type arg-types) creates C function pointer from Scheme proc");
    s7_define_function(sc, "c-pointer->integer", builtin_c_ptr_to_int, 1, 0, false,
                       "(c-pointer->integer ptr) converts c-pointer to integer");
    s7_define_function(sc, "integer->c-pointer", builtin_int_to_c_ptr, 1, 0, false,
                       "(integer->c-pointer addr) converts integer to c-pointer");
    /* Bounds-checking (#7) */
    s7_define_function(sc, "lext-malloc-tracked", builtin_malloc_tracked, 1, 0, false,
                       "(lext-malloc-tracked size) malloc with bounds registration");
    s7_define_function(sc, "lext-free-tracked", builtin_free_tracked, 1, 0, false,
                       "(lext-free-tracked ptr) free with bounds deregistration");
    s7_define_function(sc, "lext-bounds-check", builtin_bounds_check, 2, 0, false,
                       "(lext-bounds-check ptr size) returns #t if allocation covers size bytes");
    s7_define_function(sc, "lext-register-bounds", builtin_register_bounds, 2, 0, false,
                       "(lext-register-bounds ptr size) registers bounds for dynamic pointer");
    s7_define_function(sc, "lext-unregister-bounds", builtin_unregister_bounds, 1, 0, false,
                       "(lext-unregister-bounds ptr) removes bounds entry for pointer");
    s7_define_function(sc, "lext-registered-size", builtin_registered_size, 1, 0, false,
                       "(lext-registered-size ptr) returns size of tracked pointer or -1 if not tracked");
}
