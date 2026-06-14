#ifndef LEXT_BUILTINS_H
#define LEXT_BUILTINS_H

#include "s7/s7.h"

void lext_builtins_register(s7_scheme *sc);
s7_pointer lext_make_libc_constants(s7_scheme *sc, s7_pointer args);

#endif /* LEXT_BUILTINS_H */

