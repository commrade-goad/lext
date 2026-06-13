#ifndef LEXT_MODULE_H
#define LEXT_MODULE_H

#include "s7/s7.h"
#include "lext_hash.h"

/* Global table tracking already-loaded modules */
extern MeowHashTable *lext_loaded_modules;

/* Initialize the module loader (call once before registering 'use') */
void lext_module_init(void);

/* Register the 'use' builtin on the given s7 instance */
void lext_module_register(s7_scheme *sc);

#endif /* LEXT_MODULE_H */
