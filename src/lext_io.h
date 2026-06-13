#ifndef LEXT_IO_H
#define LEXT_IO_H

#include <stddef.h>
#include "s7/s7.h"
#include "lext_types.h"

/*
 * Write a Scheme value into a raw memory buffer at `buf`.
 *
 * `type_desc` — Scheme-side type descriptor (e.g. 'int, '(struct ...))
 * `ft`        — Corresponding libffi ffi_type
 * `val`       — The Scheme value to marshal
 *
 * Returns  0 on success, -1 on type mismatch.
 */
int lext_write_val(s7_scheme *sc, void *buf,
                   s7_pointer type_desc, ffi_type *ft,
                   s7_pointer val);

/*
 * Read raw memory at `buf` into a Scheme value.
 *
 * `type_desc` — Scheme-side type descriptor
 * `ft`        — Corresponding libffi ffi_type
 *
 * Returns the corresponding Scheme object.
 */
s7_pointer lext_read_val(s7_scheme *sc, void *buf,
                         s7_pointer type_desc, ffi_type *ft);

#endif /* LEXT_IO_H */
