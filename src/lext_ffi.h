#ifndef LEXT_FFI_H
#define LEXT_FFI_H

#include "s7/s7.h"

/*
 * Register all FFI-related Scheme builtins on the given s7 instance.
 *
 *  ffi-open, ffi-sym, ffi-close
 *  ffi-call, ffi-deref, ffi-set!, ffi-size, ffi-align
 *  ffi-callback
 *  c-pointer->integer, integer->c-pointer
 */
void lext_ffi_register(s7_scheme *sc);

#endif /* LEXT_FFI_H */
