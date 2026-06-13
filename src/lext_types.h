#ifndef LEXT_TYPES_H
#define LEXT_TYPES_H

#include <stddef.h>
#include <stdbool.h>
#include <ffi.h>
#include "s7/s7.h"
#include "lext_hash.h"

/* Linked list of heap-allocated ffi_type structs to free after use */
typedef struct TypeAlloc {
    ffi_type        *type;
    struct TypeAlloc *next;
} TypeAlloc;

/* Free every node in a TypeAlloc chain */
void lext_free_allocs(TypeAlloc *allocs);

/* Align an offset to the given power-of-two alignment */
size_t lext_align_to(size_t offset, size_t alignment);

/*
 * Resolve a symbolic/named type descriptor to a concrete type desc.
 * Named types registered via ffi-typedef are expanded recursively.
 */
s7_pointer lext_resolve_type_desc(s7_scheme *sc, s7_pointer type_desc);

/*
 * Returns true if the type descriptor contains at least one integer-sized field.
 * Used for union layout selection.
 */
bool lext_type_has_integer(s7_scheme *sc, s7_pointer type_desc);

/*
 * Recursively translate a Scheme type descriptor to a libffi ffi_type.
 * Any ffi_type structs allocated are prepended to *allocs so they can
 * be freed by the caller via lext_free_allocs().
 *
 * Returns NULL on failure.
 */
ffi_type *lext_parse_ffi_type(s7_scheme *sc, s7_pointer type_desc, TypeAlloc **allocs);

/* Type cache — maps meow_hash(name) -> ffi_type* for registered named types.
 * Populated by ffi-typedef; checked at the top of lext_parse_ffi_type. */
extern MeowHashTable *lext_type_cache;

#endif /* LEXT_TYPES_H */
