#include "lext_types.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* #11: Type cache — maps meow_hash(name) -> ffi_type* for registered named types */
MeowHashTable *lext_type_cache = NULL;

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */

void lext_free_allocs(TypeAlloc *allocs) {
    while (allocs) {
        TypeAlloc *next = allocs->next;
        free(allocs->type->elements);
        free(allocs->type);
        free(allocs);
        allocs = next;
    }
}

size_t lext_align_to(size_t offset, size_t alignment) {
    if (alignment == 0) return offset;
    return (offset + alignment - 1) & ~(alignment - 1);
}

/* ------------------------------------------------------------------ */
/* Type resolution                                                      */
/* ------------------------------------------------------------------ */

static s7_pointer lookup_named_type(s7_scheme *sc, s7_pointer sym) {
    s7_pointer registry = s7_name_to_value(sc, "*ffi-types*");
    if (s7_is_pair(registry)) {
        s7_pointer pair = s7_assoc(sc, sym, registry);
        if (s7_is_pair(pair))
            return s7_cdr(pair);
    }
    return s7_nil(sc);
}

s7_pointer lext_resolve_type_desc(s7_scheme *sc, s7_pointer type_desc) {
    if (s7_is_symbol(type_desc)) {
        const char *name = s7_symbol_name(type_desc);
        /* Primitive types — stop resolving */
        if (strcmp(name, "void")    == 0 || strcmp(name, "int")    == 0 ||
            strcmp(name, "enum")    == 0 || strcmp(name, "double")  == 0 ||
            strcmp(name, "float")   == 0 || strcmp(name, "string")  == 0 ||
            strcmp(name, "pointer") == 0 ||
            strcmp(name, "char")    == 0 || strcmp(name, "schar")   == 0 ||
            strcmp(name, "uchar")   == 0 ||
            strcmp(name, "int8")    == 0 || strcmp(name, "uint8")   == 0 ||
            strcmp(name, "short")   == 0 || strcmp(name, "ushort")  == 0 ||
            strcmp(name, "int16")   == 0 || strcmp(name, "uint16")  == 0 ||
            strcmp(name, "int32")   == 0 || strcmp(name, "uint32")  == 0 ||
            strcmp(name, "long")    == 0 || strcmp(name, "ulong")   == 0 ||
            strcmp(name, "int64")   == 0 || strcmp(name, "uint64")  == 0 ||
            strcmp(name, "size_t")  == 0) {
            return type_desc;
        }
        s7_pointer resolved = lookup_named_type(sc, type_desc);
        if (!s7_is_null(sc, resolved))
            return lext_resolve_type_desc(sc, resolved);
    }
    return type_desc;
}

/* ------------------------------------------------------------------ */
/* Integer presence check (union layout helper)                        */
/* ------------------------------------------------------------------ */

bool lext_type_has_integer(s7_scheme *sc, s7_pointer type_desc) {
    type_desc = lext_resolve_type_desc(sc, type_desc);
    if (s7_is_symbol(type_desc)) {
        const char *name = s7_symbol_name(type_desc);
        if (strcmp(name, "int")    == 0 || strcmp(name, "enum")   == 0 ||
            strcmp(name, "pointer")== 0 || strcmp(name, "string") == 0 ||
            strcmp(name, "char")   == 0 || strcmp(name, "schar")  == 0 ||
            strcmp(name, "uchar")  == 0 ||
            strcmp(name, "int8")   == 0 || strcmp(name, "uint8")  == 0 ||
            strcmp(name, "short")  == 0 || strcmp(name, "ushort") == 0 ||
            strcmp(name, "int16")  == 0 || strcmp(name, "uint16") == 0 ||
            strcmp(name, "int32")  == 0 || strcmp(name, "uint32") == 0 ||
            strcmp(name, "long")   == 0 || strcmp(name, "ulong")  == 0 ||
            strcmp(name, "int64")  == 0 || strcmp(name, "uint64") == 0 ||
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
                s7_pointer curr = s7_cdr(type_desc);
                while (s7_is_pair(curr)) {
                    s7_pointer field_desc = s7_car(curr);
                    s7_pointer f_type = field_desc;
                    if (s7_is_pair(field_desc) && s7_list_length(sc, field_desc) == 2)
                        f_type = s7_car(field_desc);
                    if (lext_type_has_integer(sc, f_type)) return true;
                    curr = s7_cdr(curr);
                }
            }
            if (strcmp(head_name, "*")     == 0) return true;
            if (strcmp(head_name, "array") == 0)
                return lext_type_has_integer(sc, s7_cadr(type_desc));
        }
    }
    return false;
}

/* ------------------------------------------------------------------ */
/* Type parser                                                          */
/* ------------------------------------------------------------------ */

ffi_type *lext_parse_ffi_type(s7_scheme *sc, s7_pointer type_desc, TypeAlloc **allocs) {
    /* #11: If it's a symbol and NOT a primitive, check the type cache first.
     * We check before lext_resolve_type_desc so we hit the cache on the original name. */
    if (s7_is_symbol(type_desc) && lext_type_cache) {
        const char *sym_name = s7_symbol_name(type_desc);
        /* Quickly skip obvious primitives without a hash lookup */
        if (sym_name[0] != 'v' && sym_name[0] != 'i' && sym_name[0] != 'u' &&
            sym_name[0] != 'd' && sym_name[0] != 'f' && sym_name[0] != 's' &&
            sym_name[0] != 'p' && sym_name[0] != 'c' && sym_name[0] != 'l' &&
            sym_name[0] != 'e') {
            /* Likely a user-defined type name — try cache */
            MeowHash key = meow_hash_string(sym_name);
            ffi_type *cached = (ffi_type *)meow_hash_table_get(lext_type_cache, key);
            if (cached) return cached;
        }
    }

    type_desc = lext_resolve_type_desc(sc, type_desc);

    if (s7_is_symbol(type_desc)) {
        const char *name = s7_symbol_name(type_desc);
        if (strcmp(name, "void")   == 0) return &ffi_type_void;
        if (strcmp(name, "int")    == 0) return &ffi_type_sint;
        if (strcmp(name, "enum")   == 0) return &ffi_type_sint;
        if (strcmp(name, "double") == 0) return &ffi_type_double;
        if (strcmp(name, "float")  == 0) return &ffi_type_float;
        if (strcmp(name, "string") == 0) return &ffi_type_pointer;
        if (strcmp(name, "pointer")== 0) return &ffi_type_pointer;
        if (strcmp(name, "char")   == 0 || strcmp(name, "schar") == 0)
            return &ffi_type_schar;
        if (strcmp(name, "uchar")  == 0) return &ffi_type_uchar;
        if (strcmp(name, "int8")   == 0) return &ffi_type_sint8;
        if (strcmp(name, "uint8")  == 0) return &ffi_type_uint8;
        if (strcmp(name, "short")  == 0) return &ffi_type_sshort;
        if (strcmp(name, "ushort") == 0) return &ffi_type_ushort;
        if (strcmp(name, "int16")  == 0) return &ffi_type_sint16;
        if (strcmp(name, "uint16") == 0) return &ffi_type_uint16;
        if (strcmp(name, "int32")  == 0) return &ffi_type_sint32;
        if (strcmp(name, "uint32") == 0) return &ffi_type_uint32;
        if (strcmp(name, "long")   == 0) return &ffi_type_slong;
        if (strcmp(name, "ulong")  == 0) return &ffi_type_ulong;
        if (strcmp(name, "int64")  == 0) return &ffi_type_sint64;
        if (strcmp(name, "uint64") == 0) return &ffi_type_uint64;
        if (strcmp(name, "size_t") == 0)
            return sizeof(size_t) == 8 ? &ffi_type_uint64 : &ffi_type_uint32;
        return NULL;
    }

    if (!s7_is_pair(type_desc)) return NULL;

    s7_pointer head = s7_car(type_desc);
    if (!s7_is_symbol(head)) return NULL;
    const char *head_name = s7_symbol_name(head);

    /* ---- pointer ---- */
    if (strcmp(head_name, "*") == 0) return &ffi_type_pointer;

    /* ---- array ---- */
    if (strcmp(head_name, "array") == 0) {
        s7_pointer type_arg = s7_cadr(type_desc);
        s7_pointer size_arg = s7_caddr(type_desc);
        if (!s7_is_integer(size_arg)) return NULL;
        int size = (int)s7_integer(size_arg);
        if (size <= 0) return NULL;

        ffi_type *elem_type = lext_parse_ffi_type(sc, type_arg, allocs);
        if (!elem_type) return NULL;

        ffi_type *stype = malloc(sizeof(ffi_type));
        stype->size      = 0;
        stype->alignment = 0;
        stype->type      = FFI_TYPE_STRUCT;
        stype->elements  = malloc((size + 1) * sizeof(ffi_type *));
        for (int i = 0; i < size; i++)
            stype->elements[i] = elem_type;
        stype->elements[size] = NULL;

        TypeAlloc *node = malloc(sizeof(TypeAlloc));
        node->type = stype;
        node->next = *allocs;
        *allocs = node;
        return stype;
    }

    /* ---- struct ---- */
    if (strcmp(head_name, "struct") == 0) {
        s7_pointer fields = s7_cdr(type_desc);
        int nfields = s7_list_length(sc, fields);
        if (nfields <= 0) return NULL;

        ffi_type *stype = malloc(sizeof(ffi_type));
        stype->size      = 0;
        stype->alignment = 0;
        stype->type      = FFI_TYPE_STRUCT;
        stype->elements  = malloc((nfields + 1) * sizeof(ffi_type *));

        s7_pointer curr = fields;
        for (int i = 0; i < nfields; i++) {
            s7_pointer field_desc = s7_car(curr);
            s7_pointer f_type = field_desc;
            if (s7_is_pair(field_desc) && s7_list_length(sc, field_desc) == 2)
                f_type = s7_car(field_desc);
            ffi_type *ft = lext_parse_ffi_type(sc, f_type, allocs);
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

    /* ---- union ---- */
    if (strcmp(head_name, "union") == 0) {
        s7_pointer fields = s7_cdr(type_desc);
        int nfields = s7_list_length(sc, fields);
        if (nfields <= 0) return NULL;

        size_t max_size  = 0;
        size_t max_align = 0;
        bool   has_int   = false;

        s7_pointer curr = fields;
        for (int i = 0; i < nfields; i++) {
            s7_pointer field_desc = s7_car(curr);
            s7_pointer field_type = field_desc;
            if (s7_is_pair(field_desc) && s7_list_length(sc, field_desc) == 2)
                field_type = s7_car(field_desc);
            if (lext_type_has_integer(sc, field_type))
                has_int = true;

            TypeAlloc *tmp = NULL;
            ffi_type *ft = lext_parse_ffi_type(sc, field_type, &tmp);
            if (!ft) { lext_free_allocs(tmp); return NULL; }

            if (ft->type == FFI_TYPE_STRUCT && ft->size == 0) {
                ffi_cif dummy; ffi_prep_cif(&dummy, FFI_DEFAULT_ABI, 0, ft, NULL);
            }
            if (ft->size  > max_size)  max_size  = ft->size;
            if (ft->alignment > max_align) max_align = ft->alignment;
            lext_free_allocs(tmp);
            curr = s7_cdr(curr);
        }

        size_t union_size  = lext_align_to(max_size, max_align);
        ffi_type *align_type = &ffi_type_schar;
        if      (max_align == 8) align_type = has_int ? &ffi_type_sint64  : &ffi_type_double;
        else if (max_align == 4) align_type = has_int ? &ffi_type_sint    : &ffi_type_float;
        else if (max_align == 2) align_type = &ffi_type_sshort;

        size_t num_elements = union_size / align_type->size;

        ffi_type *stype = malloc(sizeof(ffi_type));
        stype->size      = 0;
        stype->alignment = 0;
        stype->type      = FFI_TYPE_STRUCT;
        stype->elements  = malloc((num_elements + 1) * sizeof(ffi_type *));
        for (size_t i = 0; i < num_elements; i++)
            stype->elements[i] = align_type;
        stype->elements[num_elements] = NULL;

        TypeAlloc *node = malloc(sizeof(TypeAlloc));
        node->type = stype;
        node->next = *allocs;
        *allocs = node;
        return stype;
    }

    return NULL;
}
