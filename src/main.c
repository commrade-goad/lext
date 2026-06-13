#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include "s7/s7.h"
#include "helpa.h"

#define MeowHash meow_hash_impl
#include "meow_hash_x64_aesni.h"
#undef MeowHash

#define VERSTRING  "1.1.0"
#ifndef HASHVER
#define HASHVER "unknown"
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define PLATFORM_NAME "windows"
#elif defined(__APPLE__) || defined(__MACH__)
#define PLATFORM_NAME "macos"
#elif defined(__linux__)
#define PLATFORM_NAME "linux"
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#define PLATFORM_NAME "freebsd"
#else
#define PLATFORM_NAME "unknown"
#endif

#include <dlfcn.h>
#include <ffi.h>

typedef meow_u128 MeowHash;

typedef struct MeowHashNode {
    MeowHash key;
    void *val;
    struct MeowHashNode *next;
} MeowHashNode;

typedef struct {
    MeowHashNode *buckets[1024];
} MeowHashTable;

static MeowHashTable *loaded_modules;

static MeowHashTable *meow_hash_table_new(void) {
    return (MeowHashTable *)calloc(1, sizeof(MeowHashTable));
}

static void *meow_hash_table_get(MeowHashTable *t, MeowHash key) {
    if (!t) return NULL;
    uint64_t hash64 = MeowU64From(key, 0);
    size_t idx = hash64 % 1024;
    MeowHashNode *node = t->buckets[idx];
    while (node) {
        if (MeowHashesAreEqual(node->key, key)) {
            return node->val;
        }
        node = node->next;
    }
    return NULL;
}

static void meow_hash_table_set(MeowHashTable *t, MeowHash key, void *val) {
    if (!t) return;
    uint64_t hash64 = MeowU64From(key, 0);
    size_t idx = hash64 % 1024;
    MeowHashNode *node = t->buckets[idx];
    while (node) {
        if (MeowHashesAreEqual(node->key, key)) {
            node->val = val;
            return;
        }
        node = node->next;
    }
    node = (MeowHashNode *)malloc(sizeof(MeowHashNode));
    node->key = key;
    node->val = val;
    node->next = t->buckets[idx];
    t->buckets[idx] = node;
}

static MeowHash meow_hash_string(const char *str) {
    if (!str) str = "";
    return meow_hash_impl(MeowDefaultSeed, strlen(str), (void *)str);
}

typedef struct {
    HStrView *dt;
    size_t   sz;
    size_t   cp;
} ArrHsv;

void split_string_to_arrhstr(ArrHsv *a, const char *str, char c) {
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

static s7_pointer builtin_use_lib(s7_scheme *sc, s7_pointer args)
{
    const char *env = getenv("LEXT_HOME");
    if (!env)
        return s7_error(sc,
                        s7_make_symbol(sc, "env-error"),
                        s7_make_string(sc, "LEXT_HOME is not set"));

    s7_pointer curr = args;
    while (s7_is_pair(curr)) {
        s7_pointer lib = s7_car(curr);
        const char *lib_str = NULL;
        if (s7_is_string(lib)) {
            lib_str = s7_string(lib);
        } else if (s7_is_symbol(lib)) {
            lib_str = s7_symbol_name(lib);
        } else {
            return s7_error(sc,
                            s7_make_symbol(sc, "use-error"),
                            s7_make_string(sc, "module name must be a string or symbol"));
        }

        ArrHsv paths = {0};
        split_string_to_arrhstr(&paths, env, ':');
        HStr path = {0};
        bool loaded = false;

        for (size_t i = 0; i < paths.sz; i++) {
            HStrView current = paths.dt[i];
            hstr_clear(&path);
            hstr_printf(&path, "%.*s/%s/lib.scm", (int)current.sz, (const char *)current.dt, lib_str);
            int exist =
        #if _WIN32
                GetFileAttributesA((const char *)path.dt) != INVALID_FILE_ATTRIBUTES;
        #else
                access((const char *)path.dt, F_OK) == 0;
        #endif

            if (exist) {
                MeowHash key = meow_hash_string(lib_str);
                if (meow_hash_table_get(loaded_modules, key)) {
                    loaded = true;
                    break;
                } else {
                    meow_hash_table_set(loaded_modules, key, (void*)1);
                }

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

static size_t align_to(size_t offset, size_t alignment) {
    if (alignment == 0) return offset;
    return (offset + alignment - 1) & ~(alignment - 1);
}

typedef struct Loc {
    size_t line;
    size_t col;
} Loc;

typedef struct Locs {
    Loc *dt;
    size_t sz;
    size_t cp;
} Locs;

typedef struct TypeAlloc {
    ffi_type *type;
    struct TypeAlloc *next;
} TypeAlloc;

static s7_pointer lookup_named_type(s7_scheme *sc, s7_pointer sym) {
    s7_pointer registry = s7_name_to_value(sc, "*ffi-types*");
    if (s7_is_pair(registry)) {
        s7_pointer pair = s7_assoc(sc, sym, registry);
        if (s7_is_pair(pair)) {
            return s7_cdr(pair);
        }
    }
    return s7_nil(sc);
}

static s7_pointer resolve_type_desc(s7_scheme *sc, s7_pointer type_desc) {
    if (s7_is_symbol(type_desc)) {
        const char *name = s7_symbol_name(type_desc);
        if (strcmp(name, "void") == 0 || strcmp(name, "int") == 0 ||
            strcmp(name, "enum") == 0 || strcmp(name, "double") == 0 ||
            strcmp(name, "float") == 0 || strcmp(name, "string") == 0 ||
            strcmp(name, "pointer") == 0 ||
            strcmp(name, "char") == 0 || strcmp(name, "schar") == 0 || strcmp(name, "uchar") == 0 ||
            strcmp(name, "int8") == 0 || strcmp(name, "uint8") == 0 ||
            strcmp(name, "short") == 0 || strcmp(name, "ushort") == 0 ||
            strcmp(name, "int16") == 0 || strcmp(name, "uint16") == 0 ||
            strcmp(name, "int32") == 0 || strcmp(name, "uint32") == 0 ||
            strcmp(name, "long") == 0 || strcmp(name, "ulong") == 0 ||
            strcmp(name, "int64") == 0 || strcmp(name, "uint64") == 0 ||
            strcmp(name, "size_t") == 0) {
            return type_desc;
        }
        s7_pointer resolved = lookup_named_type(sc, type_desc);
        if (!s7_is_null(sc, resolved)) {
            return resolve_type_desc(sc, resolved);
        }
    }
    return type_desc;
}

static ffi_type *parse_ffi_type_rec(s7_scheme *sc, s7_pointer type_desc, TypeAlloc **allocs);

static bool type_has_integer(s7_scheme *sc, s7_pointer type_desc) {
    type_desc = resolve_type_desc(sc, type_desc);
    if (s7_is_symbol(type_desc)) {
        const char *name = s7_symbol_name(type_desc);
        if (strcmp(name, "int") == 0 || strcmp(name, "enum") == 0 ||
            strcmp(name, "pointer") == 0 || strcmp(name, "string") == 0 ||
            strcmp(name, "char") == 0 || strcmp(name, "schar") == 0 || strcmp(name, "uchar") == 0 ||
            strcmp(name, "int8") == 0 || strcmp(name, "uint8") == 0 ||
            strcmp(name, "short") == 0 || strcmp(name, "ushort") == 0 ||
            strcmp(name, "int16") == 0 || strcmp(name, "uint16") == 0 ||
            strcmp(name, "int32") == 0 || strcmp(name, "uint32") == 0 ||
            strcmp(name, "long") == 0 || strcmp(name, "ulong") == 0 ||
            strcmp(name, "int64") == 0 || strcmp(name, "uint64") == 0 ||
            strcmp(name, "size_t") == 0) {
            return true;
        }
        return false;
    }
    if (s7_is_pair(type_desc)) {
        s7_pointer head = s7_car(type_desc);
        if (s7_is_symbol(head)) {
            const char *head_name = s7_symbol_name(head);
            if (strcmp(head_name, "struct") == 0 || strcmp(head_name, "union") == 0) {
                s7_pointer fields = s7_cdr(type_desc);
                s7_pointer curr = fields;
                while (s7_is_pair(curr)) {
                    s7_pointer field_desc = s7_car(curr);
                    s7_pointer f_type = field_desc;
                    if (s7_is_pair(field_desc) && s7_list_length(sc, field_desc) == 2) {
                        f_type = s7_car(field_desc);
                    }
                    if (type_has_integer(sc, f_type)) return true;
                    curr = s7_cdr(curr);
                }
            }
            if (strcmp(head_name, "*") == 0) {
                return true;
            }
            if (strcmp(head_name, "array") == 0) {
                return type_has_integer(sc, s7_cadr(type_desc));
            }
        }
    }
    return false;
}

static ffi_type *parse_ffi_type_rec(s7_scheme *sc, s7_pointer type_desc, TypeAlloc **allocs) {
    type_desc = resolve_type_desc(sc, type_desc);
    if (s7_is_symbol(type_desc)) {
        const char *name = s7_symbol_name(type_desc);
        if (strcmp(name, "void") == 0) return &ffi_type_void;
        if (strcmp(name, "int") == 0) return &ffi_type_sint;
        if (strcmp(name, "enum") == 0) return &ffi_type_sint;
        if (strcmp(name, "double") == 0) return &ffi_type_double;
        if (strcmp(name, "float") == 0) return &ffi_type_float;
        if (strcmp(name, "string") == 0) return &ffi_type_pointer;
        if (strcmp(name, "pointer") == 0) return &ffi_type_pointer;

        if (strcmp(name, "char") == 0 || strcmp(name, "schar") == 0) return &ffi_type_schar;
        if (strcmp(name, "uchar") == 0) return &ffi_type_uchar;
        if (strcmp(name, "int8") == 0) return &ffi_type_sint8;
        if (strcmp(name, "uint8") == 0) return &ffi_type_uint8;

        if (strcmp(name, "short") == 0) return &ffi_type_sshort;
        if (strcmp(name, "ushort") == 0) return &ffi_type_ushort;
        if (strcmp(name, "int16") == 0) return &ffi_type_sint16;
        if (strcmp(name, "uint16") == 0) return &ffi_type_uint16;

        if (strcmp(name, "int32") == 0) return &ffi_type_sint32;
        if (strcmp(name, "uint32") == 0) return &ffi_type_uint32;

        if (strcmp(name, "long") == 0) return &ffi_type_slong;
        if (strcmp(name, "ulong") == 0) return &ffi_type_ulong;
        if (strcmp(name, "int64") == 0) return &ffi_type_sint64;
        if (strcmp(name, "uint64") == 0) return &ffi_type_uint64;
        if (strcmp(name, "size_t") == 0) {
            if (sizeof(size_t) == 8) return &ffi_type_uint64;
            else return &ffi_type_uint32;
        }

        return NULL;
    }

    if (s7_is_pair(type_desc)) {
        s7_pointer head = s7_car(type_desc);
        if (s7_is_symbol(head)) {
            const char *head_name = s7_symbol_name(head);
            if (strcmp(head_name, "array") == 0) {
                s7_pointer type_arg = s7_cadr(type_desc);
                s7_pointer size_arg = s7_caddr(type_desc);
                if (!s7_is_integer(size_arg)) return NULL;
                int size = (int)s7_integer(size_arg);
                if (size <= 0) return NULL;

                ffi_type *elem_type = parse_ffi_type_rec(sc, type_arg, allocs);
                if (!elem_type) return NULL;

                ffi_type *stype = malloc(sizeof(ffi_type));
                stype->size = 0;
                stype->alignment = 0;
                stype->type = FFI_TYPE_STRUCT;
                stype->elements = malloc((size + 1) * sizeof(ffi_type *));
                for (int i = 0; i < size; i++) {
                    stype->elements[i] = elem_type;
                }
                stype->elements[size] = NULL;

                TypeAlloc *node = malloc(sizeof(TypeAlloc));
                node->type = stype;
                node->next = *allocs;
                *allocs = node;

                return stype;
            }
            if (strcmp(head_name, "*") == 0) {
                return &ffi_type_pointer;
            }
            if (strcmp(head_name, "struct") == 0) {
                s7_pointer fields = s7_cdr(type_desc);
                int nfields = s7_list_length(sc, fields);
                if (nfields <= 0) return NULL;

                ffi_type *stype = malloc(sizeof(ffi_type));
                stype->size = 0;
                stype->alignment = 0;
                stype->type = FFI_TYPE_STRUCT;
                stype->elements = malloc((nfields + 1) * sizeof(ffi_type *));

                s7_pointer curr = fields;
                for (int i = 0; i < nfields; i++) {
                    s7_pointer field_desc = s7_car(curr);
                    s7_pointer f_type = field_desc;
                    if (s7_is_pair(field_desc) && s7_list_length(sc, field_desc) == 2) {
                        f_type = s7_car(field_desc);
                    }
                    ffi_type *ft = parse_ffi_type_rec(sc, f_type, allocs);
                    if (!ft) {
                        free(stype->elements);
                        free(stype);
                        return NULL;
                    }
                    stype->elements[i] = ft;
                    curr = s7_cdr(curr);
                }
                stype->elements[nfields] = NULL;

                TypeAlloc *node = malloc(sizeof(TypeAlloc));
                node->type = stype;
                node->next = *allocs;
                *allocs = node;

                return stype;
            }
            if (strcmp(head_name, "union") == 0) {
                s7_pointer fields = s7_cdr(type_desc);
                int nfields = s7_list_length(sc, fields);
                if (nfields <= 0) return NULL;

                size_t max_size = 0;
                size_t max_align = 0;
                bool has_int = false;

                s7_pointer curr = fields;
                for (int i = 0; i < nfields; i++) {
                    s7_pointer field_desc = s7_car(curr);
                    s7_pointer field_type = field_desc;
                    if (s7_is_pair(field_desc) && s7_list_length(sc, field_desc) == 2) {
                        field_type = s7_car(field_desc);
                    }
                    if (type_has_integer(sc, field_type)) {
                        has_int = true;
                    }

                    TypeAlloc *temp_allocs = NULL;
                    ffi_type *ft = parse_ffi_type_rec(sc, field_type, &temp_allocs);
                    if (!ft) {
                        TypeAlloc *curr_alloc = temp_allocs;
                        while (curr_alloc) {
                            TypeAlloc *next = curr_alloc->next;
                            free(curr_alloc->type->elements);
                            free(curr_alloc->type);
                            free(curr_alloc);
                            curr_alloc = next;
                        }
                        return NULL;
                    }

                    if (ft->type == FFI_TYPE_STRUCT && ft->size == 0) {
                        ffi_cif dummy_cif;
                        ffi_prep_cif(&dummy_cif, FFI_DEFAULT_ABI, 0, ft, NULL);
                    }

                    if (ft->size > max_size) max_size = ft->size;
                    if (ft->alignment > max_align) max_align = ft->alignment;

                    TypeAlloc *curr_alloc = temp_allocs;
                    while (curr_alloc) {
                        TypeAlloc *next = curr_alloc->next;
                        free(curr_alloc->type->elements);
                        free(curr_alloc->type);
                        free(curr_alloc);
                        curr_alloc = next;
                    }

                    curr = s7_cdr(curr);
                }

                size_t union_size = align_to(max_size, max_align);

                ffi_type *align_type = &ffi_type_schar;
                if (max_align == 8) {
                    if (has_int) align_type = &ffi_type_sint64;
                    else align_type = &ffi_type_double;
                } else if (max_align == 4) {
                    if (has_int) align_type = &ffi_type_sint;
                    else align_type = &ffi_type_float;
                } else if (max_align == 2) {
                    align_type = &ffi_type_sshort;
                }

                size_t num_elements = union_size / align_type->size;

                ffi_type *stype = malloc(sizeof(ffi_type));
                stype->size = 0;
                stype->alignment = 0;
                stype->type = FFI_TYPE_STRUCT;
                stype->elements = malloc((num_elements + 1) * sizeof(ffi_type *));
                for (size_t i = 0; i < num_elements; i++) {
                    stype->elements[i] = align_type;
                }
                stype->elements[num_elements] = NULL;

                TypeAlloc *node = malloc(sizeof(TypeAlloc));
                node->type = stype;
                node->next = *allocs;
                *allocs = node;

                return stype;
            }
        }
    }
    return NULL;
}

typedef union {
    int i;
    float f;
    double d;
    void *p;
} ffi_val;

static int write_val(s7_scheme *sc, void *buf, s7_pointer type_desc, ffi_type *ft, s7_pointer val) {
    type_desc = resolve_type_desc(sc, type_desc);

    if (ft == &ffi_type_sint || ft == &ffi_type_uint ||
        ft == &ffi_type_schar || ft == &ffi_type_uchar ||
        ft == &ffi_type_sshort || ft == &ffi_type_ushort ||
        ft == &ffi_type_sint8 || ft == &ffi_type_uint8 ||
        ft == &ffi_type_sint16 || ft == &ffi_type_uint16 ||
        ft == &ffi_type_sint32 || ft == &ffi_type_uint32 ||
        ft == &ffi_type_sint64 || ft == &ffi_type_uint64 ||
        ft == &ffi_type_slong || ft == &ffi_type_ulong) {

        int64_t val_i;
        if (s7_is_integer(val)) {
            val_i = s7_integer(val);
        } else if (s7_is_boolean(val)) {
            val_i = (s7_boolean(sc, val) ? 1 : 0);
        } else if (s7_is_character(val)) {
            val_i = (int)s7_character(val);
        } else {
            return -1;
        }

        if (ft == &ffi_type_sint || ft == &ffi_type_sint32) {
            *(int32_t *)buf = (int32_t)val_i;
        } else if (ft == &ffi_type_uint || ft == &ffi_type_uint32) {
            *(uint32_t *)buf = (uint32_t)val_i;
        } else if (ft == &ffi_type_schar || ft == &ffi_type_sint8) {
            *(int8_t *)buf = (int8_t)val_i;
        } else if (ft == &ffi_type_uchar || ft == &ffi_type_uint8) {
            *(uint8_t *)buf = (uint8_t)val_i;
        } else if (ft == &ffi_type_sshort || ft == &ffi_type_sint16) {
            *(int16_t *)buf = (int16_t)val_i;
        } else if (ft == &ffi_type_ushort || ft == &ffi_type_uint16) {
            *(uint16_t *)buf = (uint16_t)val_i;
        } else if (ft == &ffi_type_sint64 || ft == &ffi_type_slong) {
            *(int64_t *)buf = (int64_t)val_i;
        } else if (ft == &ffi_type_uint64 || ft == &ffi_type_ulong) {
            *(uint64_t *)buf = (uint64_t)val_i;
        } else {
            return -1;
        }
        return 0;
    }
    if (ft == &ffi_type_double) {
        if (s7_is_real(val)) {
            *(double *)buf = s7_real(val);
        } else if (s7_is_integer(val)) {
            *(double *)buf = (double)s7_integer(val);
        } else {
            return -1;
        }
        return 0;
    }
    if (ft == &ffi_type_float) {
        if (s7_is_real(val)) {
            *(float *)buf = (float)s7_real(val);
        } else if (s7_is_integer(val)) {
            *(float *)buf = (float)s7_integer(val);
        } else {
            return -1;
        }
        return 0;
    }
    if (ft == &ffi_type_pointer) {
        void *p = NULL;
        if (s7_is_string(val)) {
            p = (void *)s7_string(val);
        } else if (s7_is_c_pointer(val)) {
            p = s7_c_pointer(val);
        } else if (!s7_is_null(sc, val)) {
            return -1;
        }
        *(void **)buf = p;
        return 0;
    }
    if (ft->type == FFI_TYPE_STRUCT) {
        if (s7_is_pair(type_desc)) {
            s7_pointer head = s7_car(type_desc);
            if (s7_is_symbol(head) && strcmp(s7_symbol_name(head), "array") == 0) {
                s7_pointer type_arg = s7_cadr(type_desc);
                if (!s7_is_list(sc, val) && !s7_is_vector(val)) return -1;
                size_t offset = 0;
                int size = 0;
                for (int i = 0; ft->elements[i] != NULL; i++) size++;
                s7_pointer curr_val = val;
                for (int i = 0; i < size; i++) {
                    ffi_type *field_ft = ft->elements[i];
                    offset = align_to(offset, field_ft->alignment);
                    s7_pointer item_val;
                    if (s7_is_vector(val)) {
                        item_val = s7_vector_ref(sc, val, i);
                    } else {
                        if (s7_is_null(sc, curr_val)) return -1;
                        item_val = s7_car(curr_val);
                        curr_val = s7_cdr(curr_val);
                    }
                    if (write_val(sc, (char *)buf + offset, type_arg, field_ft, item_val) < 0) {
                        return -1;
                    }
                    offset += field_ft->size;
                }
                return 0;
            }
            if (s7_is_symbol(head) && strcmp(s7_symbol_name(head), "union") == 0) {
                if (!s7_is_pair(val) || !s7_is_pair(s7_cdr(val))) return -1;
                s7_pointer active_field_key = s7_car(val);
                s7_pointer field_val = s7_cadr(val);
                s7_pointer fields = s7_cdr(type_desc);

                int field_idx = -1;
                if (s7_is_integer(active_field_key)) {
                    field_idx = (int)s7_integer(active_field_key);
                } else if (s7_is_symbol(active_field_key)) {
                    s7_pointer curr = fields;
                    for (int idx = 0; s7_is_pair(curr); idx++) {
                        s7_pointer f = s7_car(curr);
                        if (s7_is_pair(f) && s7_list_length(sc, f) == 2 && strcmp(s7_symbol_name(s7_cadr(f)), s7_symbol_name(active_field_key)) == 0) {
                            field_idx = idx;
                            break;
                        }
                        curr = s7_cdr(curr);
                    }
                }

                if (field_idx < 0) return -1;

                s7_pointer field_desc = s7_list_ref(sc, fields, field_idx);
                if (s7_is_null(sc, field_desc)) return -1;

                s7_pointer field_type_desc = field_desc;
                if (s7_is_pair(field_desc) && s7_list_length(sc, field_desc) == 2) {
                    field_type_desc = s7_car(field_desc);
                }

                TypeAlloc *temp_allocs = NULL;
                ffi_type *field_ft = parse_ffi_type_rec(sc, field_type_desc, &temp_allocs);
                if (!field_ft) return -1;

                int res = write_val(sc, buf, field_type_desc, field_ft, field_val);

                TypeAlloc *curr_alloc = temp_allocs;
                while (curr_alloc) {
                    TypeAlloc *next = curr_alloc->next;
                    free(curr_alloc->type->elements);
                    free(curr_alloc->type);
                    free(curr_alloc);
                    curr_alloc = next;
                }
                return res;
            } else if (s7_is_symbol(head) && strcmp(s7_symbol_name(head), "struct") == 0) {
                size_t offset = 0;
                s7_pointer fields = s7_cdr(type_desc);
                s7_pointer curr_val = val;
                s7_pointer curr_field = fields;

                bool is_assoc = false;
                if (s7_is_pair(val) && s7_is_pair(s7_car(val)) && s7_is_symbol(s7_caar(val))) {
                    is_assoc = true;
                }

                for (int i = 0; ft->elements[i] != NULL; i++) {
                    if (s7_is_null(sc, curr_field)) return -1;
                    ffi_type *field_ft = ft->elements[i];
                    offset = align_to(offset, field_ft->alignment);

                    s7_pointer field_desc = s7_car(curr_field);
                    s7_pointer field_type_desc = field_desc;
                    s7_pointer label = s7_nil(sc);

                    if (s7_is_pair(field_desc) && s7_list_length(sc, field_desc) == 2) {
                        field_type_desc = s7_car(field_desc);
                        label = s7_cadr(field_desc);
                    }

                    s7_pointer field_val = s7_nil(sc);
                    if (is_assoc) {
                        if (!s7_is_null(sc, label)) {
                            s7_pointer pair = s7_assoc(sc, label, val);
                            if (s7_is_pair(pair)) {
                                field_val = s7_cdr(pair);
                            } else {
                                return -1;
                            }
                        } else {
                            return -1;
                        }
                    } else {
                        if (s7_is_null(sc, curr_val)) return -1;
                        field_val = s7_car(curr_val);
                        curr_val = s7_cdr(curr_val);
                    }

                    if (write_val(sc, (char *)buf + offset, field_type_desc, field_ft, field_val) < 0) {
                        return -1;
                    }
                    offset += field_ft->size;
                    curr_field = s7_cdr(curr_field);
                }
                return 0;
            }
        }
    }
    return -1;
}

static s7_pointer read_val(s7_scheme *sc, void *buf, s7_pointer type_desc, ffi_type *ft) {
    type_desc = resolve_type_desc(sc, type_desc);
    if (ft == &ffi_type_sint || ft == &ffi_type_sint32) {
        return s7_make_integer(sc, *(int32_t *)buf);
    }
    if (ft == &ffi_type_uint || ft == &ffi_type_uint32) {
        return s7_make_integer(sc, *(uint32_t *)buf);
    }
    if (ft == &ffi_type_schar || ft == &ffi_type_sint8) {
        return s7_make_integer(sc, *(int8_t *)buf);
    }
    if (ft == &ffi_type_uchar || ft == &ffi_type_uint8) {
        return s7_make_integer(sc, *(uint8_t *)buf);
    }
    if (ft == &ffi_type_sshort || ft == &ffi_type_sint16) {
        return s7_make_integer(sc, *(int16_t *)buf);
    }
    if (ft == &ffi_type_ushort || ft == &ffi_type_uint16) {
        return s7_make_integer(sc, *(uint16_t *)buf);
    }
    if (ft == &ffi_type_sint64 || ft == &ffi_type_slong) {
        return s7_make_integer(sc, *(int64_t *)buf);
    }
    if (ft == &ffi_type_uint64 || ft == &ffi_type_ulong) {
        return s7_make_integer(sc, (s7_int)*(uint64_t *)buf);
    }
    if (ft == &ffi_type_double) {
        return s7_make_real(sc, *(double *)buf);
    }
    if (ft == &ffi_type_float) {
        return s7_make_real(sc, (double)*(float *)buf);
    }
    if (ft == &ffi_type_pointer) {
        void *p = *(void **)buf;
        const char *name = s7_is_symbol(type_desc) ? s7_symbol_name(type_desc) : "";
        if (strcmp(name, "string") == 0) {
            return p ? s7_make_string(sc, (const char *)p) : s7_nil(sc);
        }
        return s7_make_c_pointer(sc, p);
    }
    if (ft->type == FFI_TYPE_STRUCT) {
        if (s7_is_pair(type_desc)) {
            s7_pointer head = s7_car(type_desc);
            if (s7_is_symbol(head) && strcmp(s7_symbol_name(head), "array") == 0) {
                s7_pointer type_arg = s7_cadr(type_desc);
                s7_pointer result = s7_nil(sc);
                s7_pointer last = s7_nil(sc);
                size_t offset = 0;
                for (int i = 0; ft->elements[i] != NULL; i++) {
                    ffi_type *field_ft = ft->elements[i];
                    offset = align_to(offset, field_ft->alignment);
                    s7_pointer val = read_val(sc, (char *)buf + offset, type_arg, field_ft);
                    s7_pointer cell = s7_cons(sc, val, s7_nil(sc));
                    if (s7_is_null(sc, result)) {
                        result = cell;
                    } else {
                        s7_set_cdr(last, cell);
                    }
                    last = cell;
                    offset += field_ft->size;
                }
                return result;
            }
            if (s7_is_symbol(head) && strcmp(s7_symbol_name(head), "union") == 0) {
                s7_pointer fields = s7_cdr(type_desc);
                s7_pointer result = s7_nil(sc);
                s7_pointer last = s7_nil(sc);
                s7_pointer curr_field = fields;

                while (s7_is_pair(curr_field)) {
                    s7_pointer field_desc = s7_car(curr_field);
                    s7_pointer field_type_desc = field_desc;
                    s7_pointer label = s7_nil(sc);

                    if (s7_is_pair(field_desc) && s7_list_length(sc, field_desc) == 2) {
                        field_type_desc = s7_car(field_desc);
                        label = s7_cadr(field_desc);
                    }

                    TypeAlloc *temp_allocs = NULL;
                    ffi_type *field_ft = parse_ffi_type_rec(sc, field_type_desc, &temp_allocs);

                    s7_pointer val = s7_nil(sc);
                    if (field_ft) {
                        val = read_val(sc, buf, field_type_desc, field_ft);
                        TypeAlloc *curr_alloc = temp_allocs;
                        while (curr_alloc) {
                            TypeAlloc *next = curr_alloc->next;
                            free(curr_alloc->type->elements);
                            free(curr_alloc->type);
                            free(curr_alloc);
                            curr_alloc = next;
                        }
                    }

                    s7_pointer cell;
                    if (!s7_is_null(sc, label)) {
                        cell = s7_cons(sc, s7_cons(sc, label, val), s7_nil(sc));
                    } else {
                        cell = s7_cons(sc, val, s7_nil(sc));
                    }

                    if (s7_is_null(sc, result)) {
                        result = cell;
                    } else {
                        s7_set_cdr(last, cell);
                    }
                    last = cell;

                    curr_field = s7_cdr(curr_field);
                }
                return result;
            } else if (s7_is_symbol(head) && strcmp(s7_symbol_name(head), "struct") == 0) {
                s7_pointer fields = s7_cdr(type_desc);
                s7_pointer result = s7_nil(sc);
                s7_pointer last = s7_nil(sc);
                s7_pointer curr_field = fields;
                size_t offset = 0;

                for (int i = 0; ft->elements[i] != NULL; i++) {
                    ffi_type *field_ft = ft->elements[i];
                    offset = align_to(offset, field_ft->alignment);

                    s7_pointer field_desc = s7_car(curr_field);
                    s7_pointer field_type_desc = field_desc;
                    s7_pointer label = s7_nil(sc);

                    if (s7_is_pair(field_desc) && s7_list_length(sc, field_desc) == 2) {
                        field_type_desc = s7_car(field_desc);
                        label = s7_cadr(field_desc);
                    }

                    s7_pointer val = read_val(sc, (char *)buf + offset, field_type_desc, field_ft);

                    s7_pointer cell;
                    if (!s7_is_null(sc, label)) {
                        cell = s7_cons(sc, s7_cons(sc, label, val), s7_nil(sc));
                    } else {
                        cell = s7_cons(sc, val, s7_nil(sc));
                    }

                    if (s7_is_null(sc, result)) {
                        result = cell;
                    } else {
                        s7_set_cdr(last, cell);
                    }
                    last = cell;

                    offset += field_ft->size;
                    curr_field = s7_cdr(curr_field);
                }
                return result;
            }
        }
    }
    return s7_nil(sc);
}

static s7_pointer s7_ffi_call(s7_scheme *sc, s7_pointer args) {
    s7_pointer func_arg = s7_car(args);
    s7_pointer ret_type_arg = s7_cadr(args);
    s7_pointer arg_types_list = s7_caddr(args);
    s7_pointer arg_vals_list = s7_cadddr(args);

    s7_pointer nfixed_arg = s7_nil(sc);
    s7_pointer temp_args = args;
    for (int i = 0; i < 4 && s7_is_pair(temp_args); i++) {
        temp_args = s7_cdr(temp_args);
    }
    if (s7_is_pair(temp_args)) {
        nfixed_arg = s7_car(temp_args);
    }

    if (!s7_is_c_pointer(func_arg)) {
        return s7_wrong_type_arg_error(sc, "ffi-call", 1, func_arg, "c-pointer");
    }
    void *func = s7_c_pointer(func_arg);

    TypeAlloc *allocs = NULL;

    ffi_type *ret_type = parse_ffi_type_rec(sc, ret_type_arg, &allocs);
    if (!ret_type) {
        TypeAlloc *curr_alloc = allocs;
        while (curr_alloc) {
            TypeAlloc *next = curr_alloc->next;
            free(curr_alloc->type->elements);
            free(curr_alloc->type);
            free(curr_alloc);
            curr_alloc = next;
        }
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2, s7_make_string(sc, "invalid return type: ~S"), ret_type_arg));
    }

    int nargs = s7_list_length(sc, arg_vals_list);
    int ntypes = s7_list_length(sc, arg_types_list);

    int nfixed = -1;
    if (s7_is_integer(nfixed_arg)) {
        nfixed = (int)s7_integer(nfixed_arg);
    }

    if (nfixed > 0) {
        if (ntypes < nfixed) {
            TypeAlloc *curr_alloc = allocs;
            while (curr_alloc) {
                TypeAlloc *next = curr_alloc->next;
                free(curr_alloc->type->elements);
                free(curr_alloc->type);
                free(curr_alloc);
                curr_alloc = next;
            }
            return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                            s7_list(sc, 1, s7_make_string(sc, "types list must cover at least the fixed arguments")));
        }
        if (nfixed > nargs) {
            TypeAlloc *curr_alloc = allocs;
            while (curr_alloc) {
                TypeAlloc *next = curr_alloc->next;
                free(curr_alloc->type->elements);
                free(curr_alloc->type);
                free(curr_alloc);
                curr_alloc = next;
            }
            return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                            s7_list(sc, 1, s7_make_string(sc, "nfixed cannot be greater than total arguments count")));
        }
    } else {
        if (ntypes != nargs) {
            TypeAlloc *curr_alloc = allocs;
            while (curr_alloc) {
                TypeAlloc *next = curr_alloc->next;
                free(curr_alloc->type->elements);
                free(curr_alloc->type);
                free(curr_alloc);
                curr_alloc = next;
            }
            return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                            s7_list(sc, 1, s7_make_string(sc, "argument types and values count mismatch")));
        }
    }

    ffi_type **arg_types = malloc(nargs * sizeof(ffi_type *));
    s7_pointer *resolved_type_descs = malloc(nargs * sizeof(s7_pointer));
    s7_pointer t_curr = arg_types_list;

    for (int i = 0; i < nargs; i++) {
        if (i < ntypes) {
            s7_pointer t_sym = s7_car(t_curr);
            ffi_type *t = parse_ffi_type_rec(sc, t_sym, &allocs);
            if (!t) {
                TypeAlloc *curr_alloc = allocs;
                while (curr_alloc) {
                    TypeAlloc *next = curr_alloc->next;
                    free(curr_alloc->type->elements);
                    free(curr_alloc->type);
                    free(curr_alloc);
                    curr_alloc = next;
                }
                free(arg_types);
                free(resolved_type_descs);
                return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                                s7_list(sc, 2, s7_make_string(sc, "invalid argument type: ~S"), t_sym));
            }
            arg_types[i] = t;
            resolved_type_descs[i] = t_sym;
            t_curr = s7_cdr(t_curr);
        } else {
            // Infer type from Scheme value
            s7_pointer val = s7_list_ref(sc, arg_vals_list, i);
            if (s7_is_integer(val) || s7_is_boolean(val) || s7_is_character(val)) {
                arg_types[i] = &ffi_type_sint;
                resolved_type_descs[i] = s7_make_symbol(sc, "int");
            } else if (s7_is_real(val)) {
                arg_types[i] = &ffi_type_double;
                resolved_type_descs[i] = s7_make_symbol(sc, "double");
            } else if (s7_is_string(val)) {
                arg_types[i] = &ffi_type_pointer;
                resolved_type_descs[i] = s7_make_symbol(sc, "string");
            } else if (s7_is_c_pointer(val) || s7_is_null(sc, val)) {
                arg_types[i] = &ffi_type_pointer;
                resolved_type_descs[i] = s7_make_symbol(sc, "pointer");
            } else {
                TypeAlloc *curr_alloc = allocs;
                while (curr_alloc) {
                    TypeAlloc *next = curr_alloc->next;
                    free(curr_alloc->type->elements);
                    free(curr_alloc->type);
                    free(curr_alloc);
                    curr_alloc = next;
                }
                free(arg_types);
                free(resolved_type_descs);
                return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                                s7_list(sc, 2, s7_make_string(sc, "could not infer type for variadic argument: ~S"), val));
            }
        }
    }

    ffi_cif cif;
    ffi_status prep_status;
    if (nfixed > 0) {
        prep_status = ffi_prep_cif_var(&cif, FFI_DEFAULT_ABI, nfixed, nargs, ret_type, arg_types);
    } else {
        prep_status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, nargs, ret_type, arg_types);
    }

    if (prep_status != FFI_OK) {
        TypeAlloc *curr_alloc = allocs;
        while (curr_alloc) {
            TypeAlloc *next = curr_alloc->next;
            free(curr_alloc->type->elements);
            free(curr_alloc->type);
            free(curr_alloc);
            curr_alloc = next;
        }
        free(arg_types);
        free(resolved_type_descs);
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 1, s7_make_string(sc, nfixed > 0 ? "ffi_prep_cif_var failed" : "ffi_prep_cif failed")));
    }

    void **arg_values = malloc(nargs * sizeof(void *));
    void **arg_data = malloc(nargs * sizeof(void *));
    s7_pointer v_curr = arg_vals_list;

    for (int i = 0; i < nargs; i++) {
        ffi_type *t = arg_types[i];
        s7_pointer val = s7_car(v_curr);
        s7_pointer t_sym = resolved_type_descs[i];

        if (t->type == FFI_TYPE_STRUCT) {
            void *struct_buf = malloc(t->size);
            memset(struct_buf, 0, t->size);
            if (write_val(sc, struct_buf, t_sym, t, val) < 0) {
                for (int j = 0; j < i; j++) {
                    free(arg_data[j]);
                }
                free(arg_data);
                free(arg_values);
                free(arg_types);
                free(resolved_type_descs);
                TypeAlloc *curr_alloc = allocs;
                while (curr_alloc) {
                    TypeAlloc *next = curr_alloc->next;
                    free(curr_alloc->type->elements);
                    free(curr_alloc->type);
                    free(curr_alloc);
                    curr_alloc = next;
                }
                return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                                s7_list(sc, 2, s7_make_string(sc, "failed to write argument: ~S"), val));
            }
            arg_data[i] = struct_buf;
            arg_values[i] = struct_buf;
        } else {
            ffi_val *p_val = malloc(sizeof(ffi_val));
            memset(p_val, 0, sizeof(ffi_val));
            if (write_val(sc, p_val, t_sym, t, val) < 0) {
                for (int j = 0; j < i; j++) {
                    free(arg_data[j]);
                }
                free(arg_data);
                free(arg_values);
                free(arg_types);
                free(resolved_type_descs);
                free(p_val);
                TypeAlloc *curr_alloc = allocs;
                while (curr_alloc) {
                    TypeAlloc *next = curr_alloc->next;
                    free(curr_alloc->type->elements);
                    free(curr_alloc->type);
                    free(curr_alloc);
                    curr_alloc = next;
                }
                return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                                s7_list(sc, 2, s7_make_string(sc, "failed to write argument: ~S"), val));
            }
            arg_data[i] = p_val;
            arg_values[i] = p_val;
        }
        v_curr = s7_cdr(v_curr);
    }

    void *ret_buf = malloc(ret_type->size > sizeof(ffi_arg) ? ret_type->size : sizeof(ffi_arg));
    memset(ret_buf, 0, ret_type->size > sizeof(ffi_arg) ? ret_type->size : sizeof(ffi_arg));

    ffi_call(&cif, FFI_FN(func), ret_buf, arg_values);

    s7_pointer result = s7_nil(sc);
    if (ret_type == &ffi_type_void) {
        result = s7_unspecified(sc);
    } else {
        result = read_val(sc, ret_buf, ret_type_arg, ret_type);
    }

    for (int i = 0; i < nargs; i++) {
        free(arg_data[i]);
    }
    free(arg_data);
    free(arg_values);
    free(arg_types);
    free(resolved_type_descs);
    free(ret_buf);

    TypeAlloc *curr_alloc = allocs;
    while (curr_alloc) {
        TypeAlloc *next = curr_alloc->next;
        free(curr_alloc->type->elements);
        free(curr_alloc->type);
        free(curr_alloc);
        curr_alloc = next;
    }

    return result;
}

static s7_pointer ffi_deref(s7_scheme *sc, s7_pointer args) {
    s7_pointer ptr_arg = s7_car(args);
    s7_pointer type_arg = s7_cadr(args);
    if (!s7_is_c_pointer(ptr_arg)) {
        return s7_wrong_type_arg_error(sc, "ffi-deref", 1, ptr_arg, "c-pointer");
    }
    void *ptr = s7_c_pointer(ptr_arg);
    if (!ptr) {
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 1, s7_make_string(sc, "dereferencing NULL pointer")));
    }
    TypeAlloc *allocs = NULL;
    ffi_type *ft = parse_ffi_type_rec(sc, type_arg, &allocs);
    if (!ft) {
        TypeAlloc *curr_alloc = allocs;
        while (curr_alloc) {
            TypeAlloc *next = curr_alloc->next;
            free(curr_alloc->type->elements);
            free(curr_alloc->type);
            free(curr_alloc);
            curr_alloc = next;
        }
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2, s7_make_string(sc, "invalid type descriptor: ~S"), type_arg));
    }
    if (ft->type == FFI_TYPE_STRUCT && ft->size == 0) {
        ffi_cif dummy_cif;
        ffi_prep_cif(&dummy_cif, FFI_DEFAULT_ABI, 0, ft, NULL);
    }
    s7_pointer result = read_val(sc, ptr, type_arg, ft);
    TypeAlloc *curr_alloc = allocs;
    while (curr_alloc) {
        TypeAlloc *next = curr_alloc->next;
        free(curr_alloc->type->elements);
        free(curr_alloc->type);
        free(curr_alloc);
        curr_alloc = next;
    }
    return result;
}

static s7_pointer ffi_set_bang(s7_scheme *sc, s7_pointer args) {
    s7_pointer ptr_arg = s7_car(args);
    s7_pointer type_arg = s7_cadr(args);
    s7_pointer val_arg = s7_caddr(args);
    if (!s7_is_c_pointer(ptr_arg)) {
        return s7_wrong_type_arg_error(sc, "ffi-set!", 1, ptr_arg, "c-pointer");
    }
    void *ptr = s7_c_pointer(ptr_arg);
    if (!ptr) {
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 1, s7_make_string(sc, "writing to NULL pointer")));
    }
    TypeAlloc *allocs = NULL;
    ffi_type *ft = parse_ffi_type_rec(sc, type_arg, &allocs);
    if (!ft) {
        TypeAlloc *curr_alloc = allocs;
        while (curr_alloc) {
            TypeAlloc *next = curr_alloc->next;
            free(curr_alloc->type->elements);
            free(curr_alloc->type);
            free(curr_alloc);
            curr_alloc = next;
        }
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2, s7_make_string(sc, "invalid type descriptor: ~S"), type_arg));
    }
    if (ft->type == FFI_TYPE_STRUCT && ft->size == 0) {
        ffi_cif dummy_cif;
        ffi_prep_cif(&dummy_cif, FFI_DEFAULT_ABI, 0, ft, NULL);
    }
    int res = write_val(sc, ptr, type_arg, ft, val_arg);
    TypeAlloc *curr_alloc = allocs;
    while (curr_alloc) {
        TypeAlloc *next = curr_alloc->next;
        free(curr_alloc->type->elements);
        free(curr_alloc->type);
        free(curr_alloc);
        curr_alloc = next;
    }
    if (res < 0) {
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2, s7_make_string(sc, "failed to write value: ~S"), val_arg));
    }
    return s7_unspecified(sc);
}

static s7_pointer ffi_size(s7_scheme *sc, s7_pointer args) {
    s7_pointer type_arg = s7_car(args);
    TypeAlloc *allocs = NULL;
    ffi_type *ft = parse_ffi_type_rec(sc, type_arg, &allocs);
    if (!ft) {
        TypeAlloc *curr_alloc = allocs;
        while (curr_alloc) {
            TypeAlloc *next = curr_alloc->next;
            free(curr_alloc->type->elements);
            free(curr_alloc->type);
            free(curr_alloc);
            curr_alloc = next;
        }
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2, s7_make_string(sc, "invalid type descriptor: ~S"), type_arg));
    }
    if (ft->type == FFI_TYPE_STRUCT && ft->size == 0) {
        ffi_cif dummy_cif;
        ffi_prep_cif(&dummy_cif, FFI_DEFAULT_ABI, 0, ft, NULL);
    }
    s7_int size = ft->size;
    TypeAlloc *curr_alloc = allocs;
    while (curr_alloc) {
        TypeAlloc *next = curr_alloc->next;
        free(curr_alloc->type->elements);
        free(curr_alloc->type);
        free(curr_alloc);
        curr_alloc = next;
    }
    return s7_make_integer(sc, size);
}

static s7_pointer ffi_align(s7_scheme *sc, s7_pointer args) {
    s7_pointer type_arg = s7_car(args);
    TypeAlloc *allocs = NULL;
    ffi_type *ft = parse_ffi_type_rec(sc, type_arg, &allocs);
    if (!ft) {
        TypeAlloc *curr_alloc = allocs;
        while (curr_alloc) {
            TypeAlloc *next = curr_alloc->next;
            free(curr_alloc->type->elements);
            free(curr_alloc->type);
            free(curr_alloc);
            curr_alloc = next;
        }
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2, s7_make_string(sc, "invalid type descriptor: ~S"), type_arg));
    }
    if (ft->type == FFI_TYPE_STRUCT && ft->size == 0) {
        ffi_cif dummy_cif;
        ffi_prep_cif(&dummy_cif, FFI_DEFAULT_ABI, 0, ft, NULL);
    }
    s7_int align = ft->alignment;
    TypeAlloc *curr_alloc = allocs;
    while (curr_alloc) {
        TypeAlloc *next = curr_alloc->next;
        free(curr_alloc->type->elements);
        free(curr_alloc->type);
        free(curr_alloc);
        curr_alloc = next;
    }
    return s7_make_integer(sc, align);
}

static s7_pointer s7_c_pointer_to_integer(s7_scheme *sc, s7_pointer args) {
    s7_pointer p = s7_car(args);
    if (!s7_is_c_pointer(p)) {
        return s7_wrong_type_arg_error(sc, "c-pointer->integer", 1, p, "c-pointer");
    }
    return s7_make_integer(sc, (s7_int)s7_c_pointer(p));
}

static s7_pointer s7_integer_to_c_pointer(s7_scheme *sc, s7_pointer args) {
    s7_pointer val = s7_car(args);
    if (!s7_is_integer(val)) {
        return s7_wrong_type_arg_error(sc, "integer->c-pointer", 1, val, "integer");
    }
    return s7_make_c_pointer(sc, (void *)s7_integer(val));
}

typedef struct {
    s7_scheme *sc;
    s7_int proc_loc;
    s7_int ret_type_loc;
    s7_int arg_types_loc;
    ffi_cif cif;
    ffi_type **arg_types;
    TypeAlloc *allocs;
} CallbackCtx;

static void closure_binding_func(ffi_cif *cif, void *ret, void **args, void *user_data) {
    CallbackCtx *ctx = (CallbackCtx *)user_data;
    s7_scheme *sc = ctx->sc;
    int nargs = cif->nargs;
    s7_pointer scheme_args = s7_nil(sc);
    s7_pointer arg_types_list = s7_gc_protected_at(sc, ctx->arg_types_loc);
    s7_pointer curr_type = arg_types_list;
    for (int i = 0; i < nargs; i++) {
        s7_pointer type_desc = s7_car(curr_type);
        s7_pointer val = read_val(sc, args[i], type_desc, cif->arg_types[i]);
        scheme_args = s7_cons(sc, val, scheme_args);
        curr_type = s7_cdr(curr_type);
    }
    scheme_args = s7_reverse(sc, scheme_args);
    s7_pointer proc = s7_gc_protected_at(sc, ctx->proc_loc);
    s7_pointer result = s7_call(sc, proc, scheme_args);
    s7_pointer ret_type_desc = s7_gc_protected_at(sc, ctx->ret_type_loc);
    write_val(sc, ret, ret_type_desc, cif->rtype, result);
}

static s7_pointer s7_ffi_callback(s7_scheme *sc, s7_pointer args) {
    s7_pointer proc = s7_car(args);
    s7_pointer ret_type_arg = s7_cadr(args);
    s7_pointer arg_types_list = s7_caddr(args);

    if (!s7_is_procedure(proc)) {
        return s7_wrong_type_arg_error(sc, "ffi-callback", 1, proc, "procedure");
    }

    TypeAlloc *allocs = NULL;
    ffi_type *ret_type = parse_ffi_type_rec(sc, ret_type_arg, &allocs);
    if (!ret_type) {
        TypeAlloc *curr_alloc = allocs;
        while (curr_alloc) {
            TypeAlloc *next = curr_alloc->next;
            free(curr_alloc->type->elements);
            free(curr_alloc->type);
            free(curr_alloc);
            curr_alloc = next;
        }
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 2, s7_make_string(sc, "invalid callback return type: ~S"), ret_type_arg));
    }

    int nargs = s7_list_length(sc, arg_types_list);
    ffi_type **arg_types = malloc(nargs * sizeof(ffi_type *));
    s7_pointer t_curr = arg_types_list;

    for (int i = 0; i < nargs; i++) {
        s7_pointer t_sym = s7_car(t_curr);
        ffi_type *t = parse_ffi_type_rec(sc, t_sym, &allocs);
        if (!t) {
            TypeAlloc *curr_alloc = allocs;
            while (curr_alloc) {
                TypeAlloc *next = curr_alloc->next;
                free(curr_alloc->type->elements);
                free(curr_alloc->type);
                free(curr_alloc);
                curr_alloc = next;
            }
            free(arg_types);
            return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                            s7_list(sc, 2, s7_make_string(sc, "invalid callback argument type: ~S"), t_sym));
        }
        arg_types[i] = t;
        t_curr = s7_cdr(t_curr);
    }

    CallbackCtx *ctx = malloc(sizeof(CallbackCtx));
    ctx->sc = sc;
    ctx->proc_loc = s7_gc_protect(sc, proc);
    ctx->ret_type_loc = s7_gc_protect(sc, ret_type_arg);
    ctx->arg_types_loc = s7_gc_protect(sc, arg_types_list);
    ctx->arg_types = arg_types;
    ctx->allocs = allocs;

    ffi_status prep_status = ffi_prep_cif(&(ctx->cif), FFI_DEFAULT_ABI, nargs, ret_type, arg_types);
    if (prep_status != FFI_OK) {
        s7_gc_unprotect_at(sc, ctx->proc_loc);
        s7_gc_unprotect_at(sc, ctx->ret_type_loc);
        s7_gc_unprotect_at(sc, ctx->arg_types_loc);
        free(arg_types);
        free(ctx);
        TypeAlloc *curr_alloc = allocs;
        while (curr_alloc) {
            TypeAlloc *next = curr_alloc->next;
            free(curr_alloc->type->elements);
            free(curr_alloc->type);
            free(curr_alloc);
            curr_alloc = next;
        }
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 1, s7_make_string(sc, "ffi_prep_cif failed for callback")));
    }

    void *code_ptr = NULL;
    ffi_closure *closure = ffi_closure_alloc(sizeof(ffi_closure), &code_ptr);
    if (!closure) {
        s7_gc_unprotect_at(sc, ctx->proc_loc);
        s7_gc_unprotect_at(sc, ctx->ret_type_loc);
        s7_gc_unprotect_at(sc, ctx->arg_types_loc);
        free(arg_types);
        free(ctx);
        TypeAlloc *curr_alloc = allocs;
        while (curr_alloc) {
            TypeAlloc *next = curr_alloc->next;
            free(curr_alloc->type->elements);
            free(curr_alloc->type);
            free(curr_alloc);
            curr_alloc = next;
        }
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 1, s7_make_string(sc, "ffi_closure_alloc failed")));
    }

    prep_status = ffi_prep_closure_loc(closure, &(ctx->cif), closure_binding_func, ctx, code_ptr);
    if (prep_status != FFI_OK) {
        ffi_closure_free(closure);
        s7_gc_unprotect_at(sc, ctx->proc_loc);
        s7_gc_unprotect_at(sc, ctx->ret_type_loc);
        s7_gc_unprotect_at(sc, ctx->arg_types_loc);
        free(arg_types);
        free(ctx);
        TypeAlloc *curr_alloc = allocs;
        while (curr_alloc) {
            TypeAlloc *next = curr_alloc->next;
            free(curr_alloc->type->elements);
            free(curr_alloc->type);
            free(curr_alloc);
            curr_alloc = next;
        }
        return s7_error(sc, s7_make_symbol(sc, "ffi-error"),
                        s7_list(sc, 1, s7_make_string(sc, "ffi_prep_closure_loc failed")));
    }

    return s7_make_c_pointer(sc, code_ptr);
}


// Lexer states for stripping or capturing delimiters
enum State {
    STATE_TEXT,
    STATE_AT1,
    STATE_AT2
};

int main(int argc, char **argv) {
    bool no_template = false;
    char *input_file = NULL;
    char *output_file = NULL;

    int scheme_argc = 0;
    char **scheme_argv = NULL;
    char *script_name = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--") == 0) {
            scheme_argc = argc - (i + 1);
            if (scheme_argc > 0) {
                scheme_argv = &argv[i + 1];
            }
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
            if (!input_file) {
                input_file = argv[i];
                script_name = argv[i];
            } else if (!output_file) {
                output_file = argv[i];
            } else {
                fprintf(stderr, "Too many arguments.\n");
                fprintf(stderr, "Usage: %s [options] <input_file> [<output_file>]\n", argv[0]);
                return EXIT_FAILURE;
            }
        }
    }

    if (!input_file) {
        fprintf(stderr, "Error: Input file is required.\n");
        if (no_template) {
            fprintf(stderr, "Usage: %s [options] <input_file>\n", argv[0]);
        } else {
            fprintf(stderr, "Usage: %s [options] <input.texm> <output.tex>\n", argv[0]);
        }
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

    FILE *out = NULL;
    if (!no_template) {
        out = fopen(output_file, "w");
        if (!out) {
            fprintf(stderr, "ERR: Could not create output file: %s\n", output_file);
            fclose(in);
            return EXIT_FAILURE;
        }
    } else {
        fclose(in);
        in = NULL;
    }

    // Initialize s7 VM state
    s7_scheme *s7 = s7_init();
    if (!s7) {
        fprintf(stderr, "ERR: Failed to initialize s7 Scheme engine.\n");
        if (in) fclose(in);
        if (out) fclose(out);
        return EXIT_FAILURE;
    }

    loaded_modules = meow_hash_table_new();

    // Register FFI bindings
    s7_define_variable(s7, "*ffi-types*", s7_nil(s7));
    s7_eval_c_string(s7, "(define (ffi-typedef name type-desc) (set! *ffi-types* (cons (cons name type-desc) *ffi-types*)))");

    s7_define_function(s7, "use", builtin_use_lib, 1, 0, true, "(use) loads lib.scm file from LEXT_HOME env");

    s7_define_function(s7, "ffi-open", ffi_open, 1, 0, false, "(ffi-open path) loads dynamic library");
    s7_define_function(s7, "ffi-sym", ffi_sym, 2, 0, false, "(ffi-sym handle name) finds symbol in library");
    s7_define_function(s7, "ffi-close", ffi_close, 1, 0, false, "(ffi-close handle) closes dynamic library");
    s7_define_function(s7, "ffi-call", s7_ffi_call, 4, 1, false, "(ffi-call func ret-type arg-types arg-vals [nfixed]) invokes foreign function");
    s7_define_function(s7, "ffi-deref", ffi_deref, 2, 0, false, "(ffi-deref ptr type-desc) dereferences pointer using type description");
    s7_define_function(s7, "ffi-set!", ffi_set_bang, 3, 0, false, "(ffi-set! ptr type-desc value) writes value to pointer using type description");
    s7_define_function(s7, "ffi-size", ffi_size, 1, 0, false, "(ffi-size type) returns size of FFI type in bytes");
    s7_define_function(s7, "ffi-align", ffi_align, 1, 0, false, "(ffi-align type) returns alignment of FFI type");
    s7_define_function(s7, "c-pointer->integer", s7_c_pointer_to_integer, 1, 0, false, "(c-pointer->integer ptr) converts c-pointer to integer");
    s7_define_function(s7, "integer->c-pointer", s7_integer_to_c_pointer, 1, 0, false, "(integer->c-pointer addr) converts integer to c-pointer");
    s7_define_function(s7, "ffi-callback", s7_ffi_callback, 3, 0, false, "(ffi-callback proc ret-type arg-types) creates a C function pointer from a Scheme procedure");

    // Initialize command line arguments list
    s7_pointer scheme_script_input = s7_make_string(s7, script_name);
    s7_pointer argv_list = s7_nil(s7);
    for (int i = scheme_argc - 1; i >= 0; i--) {
        argv_list = s7_cons(s7, s7_make_string(s7, scheme_argv[i]), argv_list);
    }
    s7_define_variable(s7, "*script-name*", scheme_script_input);
    s7_define_variable(s7, "*argv*", argv_list);
    s7_define_variable(s7, "argv", argv_list);
    s7_define_variable(s7, "*platform*", s7_make_string(s7, PLATFORM_NAME));


    if (no_template) {
        s7_load(s7, input_file);
        return EXIT_SUCCESS;
    }

    Loc loc = {1, 0};
    enum State state = STATE_TEXT;
    int c;

    while ((c = fgetc(in)) != EOF) {

        if (c == '\n') {
            loc.line++;
            loc.col = 0;
        } else loc.col++;

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
                    HStr lisp_buf;
                    hstr_init(&lisp_buf);
                    hstr_push(&lisp_buf, '('); // Capture the opening parenthesis

                    Locs locs = {0};
                    helpa_da_append(locs, loc);

                    int depth = 1;
                    int lc;
                    bool in_string = false;
                    bool in_comment = false;
                    bool escape = false;
                    bool in_raw_string = false;

                    // Safely track internal token blocks to compute exact tree balance
                    while (depth > 0 && (lc = fgetc(in)) != EOF) {

                        if (lc == '\n') {
                            loc.line++;
                            loc.col = 0;
                        } else loc.col++;

                        if (in_raw_string) {
                            if (lc == '\\') {
                                int next1 = fgetc(in);
                                if (next1 == '\\') {
                                    hstr_append_cstr(&lisp_buf, "\\\\\\\\");
                                } else if (next1 == '"') {
                                    hstr_append_cstr(&lisp_buf, "\\\"");
                                } else {
                                    if (next1 != EOF) ungetc(next1, in);
                                    hstr_append_cstr(&lisp_buf, "\\\\");
                                }
                                continue;
                            } else if (lc == '"') {
                                in_raw_string = false;
                                hstr_push(&lisp_buf, lc);
                            } else {
                                hstr_push(&lisp_buf, lc);
                            }
                            continue;
                        }

                        if (escape) {
                            hstr_push(&lisp_buf, lc);
                            escape = false;
                            continue;
                        }

                        if (lc == '\\') {
                            hstr_push(&lisp_buf, '\\');
                            escape = true;
                            continue;
                        }

                        if (in_comment) {
                            hstr_push(&lisp_buf, lc);
                            if (lc == '\n') in_comment = false;
                            continue;
                        }

                        if (!in_string && lc == 'r') {
                            int next1 = fgetc(in);
                            if (next1 == '"') {
                                in_raw_string = true;
                                hstr_push(&lisp_buf, '"');
                                continue;
                            } else {
                                hstr_push(&lisp_buf, 'r');
                                if (next1 != EOF) {
                                    ungetc(next1, in);
                                }
                                continue;
                            }
                        }

                        hstr_push(&lisp_buf, lc);
                        if (lc == '"') {
                            in_string = !in_string;
                            continue;
                        }

                        if (!in_string) {
                            if (lc == ';') {
                                in_comment = true;
                            } else if (lc == '(') {
                                helpa_da_append(locs, loc);
                                depth++;
                            } else if (lc == ')') {
                                helpa_da_pop(locs);
                                depth--;
                            }
                        }
                    }

                    if (depth > 0) {
                        Loc last = helpa_da_last(locs);
                        fprintf(stderr, "%s:%zu:%zu: ERR: Unmatched parenthesis inside Lisp block (remaining depth: %d).\n", script_name, last.line, last.col, depth);
                        hstr_free(&lisp_buf);
                        helpa_da_free(locs);
                        fclose(in);
                        fclose(out);
                        return EXIT_FAILURE;
                    }

                    // Evaluate expression immediately inside s7 environment
                    s7_pointer res = s7_eval_c_string(s7, (char *)lisp_buf.dt);

                    // Streaming design rule: Evaluate definitions silently.
                    // Only dump explicitly computed string constants into target file.
                    if (s7_is_string(res)) {
                        fputs(s7_string(res), out);
                    }

                    helpa_da_free(locs);
                    hstr_free(&lisp_buf);
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
