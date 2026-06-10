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

static size_t align_to(size_t offset, size_t alignment) {
    if (alignment == 0) return offset;
    return (offset + alignment - 1) & ~(alignment - 1);
}

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
            strcmp(name, "pointer") == 0) {
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
            strcmp(name, "pointer") == 0 || strcmp(name, "string") == 0) {
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
                    if (type_has_integer(sc, s7_car(curr))) return true;
                    curr = s7_cdr(curr);
                }
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
        return NULL;
    }
    
    if (s7_is_pair(type_desc)) {
        s7_pointer head = s7_car(type_desc);
        if (s7_is_symbol(head)) {
            const char *head_name = s7_symbol_name(head);
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
                    ffi_type *ft = parse_ffi_type_rec(sc, s7_car(curr), allocs);
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
                    s7_pointer field_type = s7_car(curr);
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
    if (ft == &ffi_type_sint) {
        if (s7_is_integer(val)) {
            *(int *)buf = (int)s7_integer(val);
        } else if (s7_is_boolean(val)) {
            *(int *)buf = (s7_boolean(sc, val) ? 1 : 0);
        } else if (s7_is_character(val)) {
            *(int *)buf = (int)s7_character(val);
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
            if (s7_is_symbol(head) && strcmp(s7_symbol_name(head), "union") == 0) {
                if (!s7_is_pair(val) || !s7_is_pair(s7_cdr(val))) return -1;
                int field_idx = (int)s7_integer(s7_car(val));
                s7_pointer field_val = s7_cadr(val);
                s7_pointer fields = s7_cdr(type_desc);
                s7_pointer field_type_desc = s7_list_ref(sc, fields, field_idx);
                if (s7_is_null(sc, field_type_desc)) return -1;
                
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
                if (!s7_is_list(sc, val)) return -1;
                size_t offset = 0;
                s7_pointer fields = s7_cdr(type_desc);
                s7_pointer curr_val = val;
                s7_pointer curr_field = fields;
                
                for (int i = 0; ft->elements[i] != NULL; i++) {
                    if (s7_is_null(sc, curr_val)) return -1;
                    ffi_type *field_ft = ft->elements[i];
                    offset = align_to(offset, field_ft->alignment);
                    
                    s7_pointer field_type_desc = s7_car(curr_field);
                    if (write_val(sc, (char *)buf + offset, field_type_desc, field_ft, s7_car(curr_val)) < 0) {
                        return -1;
                    }
                    offset += field_ft->size;
                    curr_val = s7_cdr(curr_val);
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
    if (ft == &ffi_type_sint) {
        return s7_make_integer(sc, *(int *)buf);
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
            if (s7_is_symbol(head) && strcmp(s7_symbol_name(head), "union") == 0) {
                s7_pointer fields = s7_cdr(type_desc);
                s7_pointer result = s7_nil(sc);
                s7_pointer last = s7_nil(sc);
                s7_pointer curr_field = fields;
                
                while (s7_is_pair(curr_field)) {
                    s7_pointer field_type_desc = s7_car(curr_field);
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
                    
                    s7_pointer cell = s7_cons(sc, val, s7_nil(sc));
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
                    
                    s7_pointer field_type_desc = s7_car(curr_field);
                    s7_pointer val = read_val(sc, (char *)buf + offset, field_type_desc, field_ft);
                    
                    s7_pointer cell = s7_cons(sc, val, s7_nil(sc));
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

// Lexer states for stripping or capturing delimiters
enum State {
    STATE_TEXT,
    STATE_AT1,
    STATE_AT2
};

int main(int argc, char **argv) {
    bool escape_enabled = false;
    bool no_template = false;
    char *input_file = NULL;
    char *output_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [options] <input_file> [<output_file>]\n", argv[0]);
            printf("Options:\n");
            printf("  -h, --help         Show this help message\n");
            printf("  -v, --version      Show version information\n");
            printf("  -e, --escape       Enable auto-escaping of backslashes inside string literals in @@(...)\n");
            printf("  -s, --no-template  Run as a pure Scheme script runner without output\n");
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("lext version " VERSTRING "-" HASHVER "\n");
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--escape") == 0) {
            escape_enabled = true;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--no-template") == 0) {
            no_template = true;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            fprintf(stderr, "Usage: %s [options] <input_file> [<output_file>]\n", argv[0]);
            return EXIT_FAILURE;
        } else {
            if (!input_file) {
                input_file = argv[i];
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

    // Register FFI bindings
    s7_define_variable(s7, "*ffi-types*", s7_nil(s7));
    s7_eval_c_string(s7, "(define (ffi-typedef name type-desc) (set! *ffi-types* (cons (cons name type-desc) *ffi-types*)))");
    s7_define_function(s7, "ffi-open", ffi_open, 1, 0, false, "(ffi-open path) loads dynamic library");
    s7_define_function(s7, "ffi-sym", ffi_sym, 2, 0, false, "(ffi-sym handle name) finds symbol in library");
    s7_define_function(s7, "ffi-close", ffi_close, 1, 0, false, "(ffi-close handle) closes dynamic library");
    s7_define_function(s7, "ffi-call", s7_ffi_call, 4, 1, false, "(ffi-call func ret-type arg-types arg-vals [nfixed]) invokes foreign function");

    if (no_template) {
        s7_load(s7, input_file);
        return EXIT_SUCCESS;
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
