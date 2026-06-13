#include "lext_io.h"
#include "lext_types.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/* write_val                                                            */
/* ------------------------------------------------------------------ */

int lext_write_val(s7_scheme *sc, void *buf,
                   s7_pointer type_desc, ffi_type *ft,
                   s7_pointer val)
{
    type_desc = lext_resolve_type_desc(sc, type_desc);

    /* --- Integers --- */
    if (ft == &ffi_type_sint   || ft == &ffi_type_uint   ||
        ft == &ffi_type_schar  || ft == &ffi_type_uchar  ||
        ft == &ffi_type_sshort || ft == &ffi_type_ushort ||
        ft == &ffi_type_sint8  || ft == &ffi_type_uint8  ||
        ft == &ffi_type_sint16 || ft == &ffi_type_uint16 ||
        ft == &ffi_type_sint32 || ft == &ffi_type_uint32 ||
        ft == &ffi_type_sint64 || ft == &ffi_type_uint64 ||
        ft == &ffi_type_slong  || ft == &ffi_type_ulong) {

        int64_t val_i;
        if      (s7_is_integer(val))   val_i = s7_integer(val);
        else if (s7_is_boolean(val))   val_i = (s7_boolean(sc, val) ? 1 : 0);
        else if (s7_is_character(val)) val_i = (int)s7_character(val);
        else return -1;

        if      (ft == &ffi_type_sint  || ft == &ffi_type_sint32) *(int32_t  *)buf = (int32_t)val_i;
        else if (ft == &ffi_type_uint  || ft == &ffi_type_uint32) *(uint32_t *)buf = (uint32_t)val_i;
        else if (ft == &ffi_type_schar || ft == &ffi_type_sint8)  *(int8_t   *)buf = (int8_t)val_i;
        else if (ft == &ffi_type_uchar || ft == &ffi_type_uint8)  *(uint8_t  *)buf = (uint8_t)val_i;
        else if (ft == &ffi_type_sshort|| ft == &ffi_type_sint16) *(int16_t  *)buf = (int16_t)val_i;
        else if (ft == &ffi_type_ushort|| ft == &ffi_type_uint16) *(uint16_t *)buf = (uint16_t)val_i;
        else if (ft == &ffi_type_sint64|| ft == &ffi_type_slong)  *(int64_t  *)buf = (int64_t)val_i;
        else if (ft == &ffi_type_uint64|| ft == &ffi_type_ulong)  *(uint64_t *)buf = (uint64_t)val_i;
        else return -1;
        return 0;
    }

    /* --- Floats --- */
    if (ft == &ffi_type_double) {
        if      (s7_is_real(val))    *(double *)buf = s7_real(val);
        else if (s7_is_integer(val)) *(double *)buf = (double)s7_integer(val);
        else return -1;
        return 0;
    }
    if (ft == &ffi_type_float) {
        if      (s7_is_real(val))    *(float *)buf = (float)s7_real(val);
        else if (s7_is_integer(val)) *(float *)buf = (float)s7_integer(val);
        else return -1;
        return 0;
    }

    /* --- Pointer --- */
    if (ft == &ffi_type_pointer) {
        void *p = NULL;
        if      (s7_is_string(val))                                    p = (void *)s7_string(val);
        else if (s7_is_c_pointer(val))                                 p = s7_c_pointer(val);
        else if (s7_is_pair(val) && s7_is_c_pointer(s7_cdr(val)))     p = s7_c_pointer(s7_cdr(val));
        else if (!s7_is_null(sc, val)) return -1;
        *(void **)buf = p;
        return 0;
    }

    /* --- Struct / Array / Union --- */
    if (ft->type == FFI_TYPE_STRUCT) {
        if (!s7_is_pair(type_desc)) return -1;
        s7_pointer head = s7_car(type_desc);
        if (!s7_is_symbol(head)) return -1;
        const char *head_name = s7_symbol_name(head);

        /* array */
        if (strcmp(head_name, "array") == 0) {
            s7_pointer type_arg = s7_cadr(type_desc);
            if (!s7_is_list(sc, val) && !s7_is_vector(val)) return -1;
            int size = 0;
            for (int i = 0; ft->elements[i] != NULL; i++) size++;
            size_t offset = 0;
            s7_pointer curr_val = val;
            for (int i = 0; i < size; i++) {
                ffi_type *field_ft = ft->elements[i];
                offset = lext_align_to(offset, field_ft->alignment);
                s7_pointer item_val;
                if (s7_is_vector(val)) {
                    item_val = s7_vector_ref(sc, val, i);
                } else {
                    if (s7_is_null(sc, curr_val)) return -1;
                    item_val  = s7_car(curr_val);
                    curr_val  = s7_cdr(curr_val);
                }
                if (lext_write_val(sc, (char *)buf + offset, type_arg, field_ft, item_val) < 0)
                    return -1;
                offset += field_ft->size;
            }
            return 0;
        }

        /* union */
        if (strcmp(head_name, "union") == 0) {
            if (!s7_is_pair(val) || !s7_is_pair(s7_cdr(val))) return -1;
            s7_pointer active_field_key = s7_car(val);
            s7_pointer field_val        = s7_cadr(val);
            s7_pointer fields           = s7_cdr(type_desc);

            int field_idx = -1;
            if (s7_is_integer(active_field_key)) {
                field_idx = (int)s7_integer(active_field_key);
            } else if (s7_is_symbol(active_field_key)) {
                s7_pointer curr = fields;
                for (int idx = 0; s7_is_pair(curr); idx++) {
                    s7_pointer f = s7_car(curr);
                    if (s7_is_pair(f) && s7_list_length(sc, f) == 2 &&
                        strcmp(s7_symbol_name(s7_cadr(f)),
                               s7_symbol_name(active_field_key)) == 0) {
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
            if (s7_is_pair(field_desc) && s7_list_length(sc, field_desc) == 2)
                field_type_desc = s7_car(field_desc);

            TypeAlloc *tmp = NULL;
            ffi_type *field_ft = lext_parse_ffi_type(sc, field_type_desc, &tmp);
            if (!field_ft) { lext_free_allocs(tmp); return -1; }
            int res = lext_write_val(sc, buf, field_type_desc, field_ft, field_val);
            lext_free_allocs(tmp);
            return res;
        }

        /* struct */
        if (strcmp(head_name, "struct") == 0) {
            size_t offset    = 0;
            s7_pointer fields    = s7_cdr(type_desc);
            s7_pointer curr_val  = val;
            s7_pointer curr_field= fields;

            bool is_assoc = s7_is_pair(val) && s7_is_pair(s7_car(val)) &&
                            s7_is_symbol(s7_caar(val));

            for (int i = 0; ft->elements[i] != NULL; i++) {
                if (s7_is_null(sc, curr_field)) return -1;
                ffi_type *field_ft = ft->elements[i];
                offset = lext_align_to(offset, field_ft->alignment);

                s7_pointer field_desc      = s7_car(curr_field);
                s7_pointer field_type_desc = field_desc;
                s7_pointer label           = s7_nil(sc);
                if (s7_is_pair(field_desc) && s7_list_length(sc, field_desc) == 2) {
                    field_type_desc = s7_car(field_desc);
                    label           = s7_cadr(field_desc);
                }

                s7_pointer field_val = s7_nil(sc);
                if (is_assoc) {
                    if (s7_is_null(sc, label)) return -1;
                    s7_pointer pair = s7_assoc(sc, label, val);
                    if (!s7_is_pair(pair)) return -1;
                    field_val = s7_cdr(pair);
                } else {
                    if (s7_is_null(sc, curr_val)) return -1;
                    field_val = s7_car(curr_val);
                    curr_val  = s7_cdr(curr_val);
                }

                if (lext_write_val(sc, (char *)buf + offset,
                                   field_type_desc, field_ft, field_val) < 0)
                    return -1;
                offset += field_ft->size;
                curr_field = s7_cdr(curr_field);
            }
            return 0;
        }
    }

    return -1;
}

/* ------------------------------------------------------------------ */
/* read_val                                                             */
/* ------------------------------------------------------------------ */

s7_pointer lext_read_val(s7_scheme *sc, void *buf,
                         s7_pointer type_desc, ffi_type *ft)
{
    type_desc = lext_resolve_type_desc(sc, type_desc);

    if (ft == &ffi_type_sint  || ft == &ffi_type_sint32)  return s7_make_integer(sc, *(int32_t  *)buf);
    if (ft == &ffi_type_uint  || ft == &ffi_type_uint32)  return s7_make_integer(sc, *(uint32_t *)buf);
    if (ft == &ffi_type_schar || ft == &ffi_type_sint8)   return s7_make_integer(sc, *(int8_t   *)buf);
    if (ft == &ffi_type_uchar || ft == &ffi_type_uint8)   return s7_make_integer(sc, *(uint8_t  *)buf);
    if (ft == &ffi_type_sshort|| ft == &ffi_type_sint16)  return s7_make_integer(sc, *(int16_t  *)buf);
    if (ft == &ffi_type_ushort|| ft == &ffi_type_uint16)  return s7_make_integer(sc, *(uint16_t *)buf);
    if (ft == &ffi_type_sint64|| ft == &ffi_type_slong)   return s7_make_integer(sc, *(int64_t  *)buf);
    if (ft == &ffi_type_uint64|| ft == &ffi_type_ulong)   return s7_make_integer(sc, (s7_int)*(uint64_t *)buf);
    if (ft == &ffi_type_double)                            return s7_make_real(sc, *(double *)buf);
    if (ft == &ffi_type_float)                             return s7_make_real(sc, (double)*(float *)buf);

    if (ft == &ffi_type_pointer) {
        void *p = *(void **)buf;
        /* Check if type_desc is a symbol "string" */
        const char *name = s7_is_symbol(type_desc) ? s7_symbol_name(type_desc) : "";
        if (strcmp(name, "string") == 0)
            return p ? s7_make_string(sc, (const char *)p) : s7_nil(sc);

        /* #12: if type_desc is a pair (* SomeName) where SomeName is a registered
         * named type, wrap the pointer as a tptr (SomeName . c-pointer). */
        if (s7_is_pair(type_desc)) {
            s7_pointer head = s7_car(type_desc);
            if (s7_is_symbol(head) && strcmp(s7_symbol_name(head), "*") == 0) {
                s7_pointer named = s7_cadr(type_desc);
                if (s7_is_symbol(named)) {
                    s7_pointer registry = s7_name_to_value(sc, "*ffi-types*");
                    if (s7_is_pair(registry)) {
                        s7_pointer found = s7_assoc(sc, named, registry);
                        if (s7_is_pair(found)) {
                            /* It's a registered named type — wrap as tptr */
                            return s7_cons(sc, named, s7_make_c_pointer(sc, p));
                        }
                    }
                }
            }
        }
        return s7_make_c_pointer(sc, p);
    }

    if (ft->type == FFI_TYPE_STRUCT) {
        if (!s7_is_pair(type_desc)) return s7_nil(sc);
        s7_pointer head = s7_car(type_desc);
        if (!s7_is_symbol(head)) return s7_nil(sc);
        const char *head_name = s7_symbol_name(head);

        /* array */
        if (strcmp(head_name, "array") == 0) {
            s7_pointer type_arg = s7_cadr(type_desc);
            s7_pointer result = s7_nil(sc);
            s7_pointer last   = s7_nil(sc);
            size_t offset = 0;
            for (int i = 0; ft->elements[i] != NULL; i++) {
                ffi_type *field_ft = ft->elements[i];
                offset = lext_align_to(offset, field_ft->alignment);
                s7_pointer item = lext_read_val(sc, (char *)buf + offset, type_arg, field_ft);
                s7_pointer cell = s7_cons(sc, item, s7_nil(sc));
                if (s7_is_null(sc, result)) result = cell;
                else s7_set_cdr(last, cell);
                last = cell;
                offset += field_ft->size;
            }
            return result;
        }

        /* union */
        if (strcmp(head_name, "union") == 0) {
            s7_pointer fields = s7_cdr(type_desc);
            s7_pointer result = s7_nil(sc);
            s7_pointer last   = s7_nil(sc);
            s7_pointer curr_field = fields;

            while (s7_is_pair(curr_field)) {
                s7_pointer field_desc      = s7_car(curr_field);
                s7_pointer field_type_desc = field_desc;
                s7_pointer label           = s7_nil(sc);
                if (s7_is_pair(field_desc) && s7_list_length(sc, field_desc) == 2) {
                    field_type_desc = s7_car(field_desc);
                    label           = s7_cadr(field_desc);
                }

                TypeAlloc *tmp = NULL;
                ffi_type *field_ft = lext_parse_ffi_type(sc, field_type_desc, &tmp);
                s7_pointer item = s7_nil(sc);
                if (field_ft) {
                    item = lext_read_val(sc, buf, field_type_desc, field_ft);
                    lext_free_allocs(tmp);
                }

                s7_pointer cell;
                if (!s7_is_null(sc, label))
                    cell = s7_cons(sc, s7_cons(sc, label, item), s7_nil(sc));
                else
                    cell = s7_cons(sc, item, s7_nil(sc));

                if (s7_is_null(sc, result)) result = cell;
                else s7_set_cdr(last, cell);
                last = cell;
                curr_field = s7_cdr(curr_field);
            }
            return result;
        }

        /* struct */
        if (strcmp(head_name, "struct") == 0) {
            s7_pointer fields     = s7_cdr(type_desc);
            s7_pointer result     = s7_nil(sc);
            s7_pointer last       = s7_nil(sc);
            s7_pointer curr_field = fields;
            size_t offset = 0;

            for (int i = 0; ft->elements[i] != NULL; i++) {
                ffi_type *field_ft = ft->elements[i];
                offset = lext_align_to(offset, field_ft->alignment);

                s7_pointer field_desc      = s7_car(curr_field);
                s7_pointer field_type_desc = field_desc;
                s7_pointer label           = s7_nil(sc);
                if (s7_is_pair(field_desc) && s7_list_length(sc, field_desc) == 2) {
                    field_type_desc = s7_car(field_desc);
                    label           = s7_cadr(field_desc);
                }

                s7_pointer item = lext_read_val(sc, (char *)buf + offset, field_type_desc, field_ft);

                s7_pointer cell;
                if (!s7_is_null(sc, label))
                    cell = s7_cons(sc, s7_cons(sc, label, item), s7_nil(sc));
                else
                    cell = s7_cons(sc, item, s7_nil(sc));

                if (s7_is_null(sc, result)) result = cell;
                else s7_set_cdr(last, cell);
                last = cell;

                offset += field_ft->size;
                curr_field = s7_cdr(curr_field);
            }
            return result;
        }
    }

    return s7_nil(sc);
}
