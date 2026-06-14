#ifndef LEXT_HASH_H
#define LEXT_HASH_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MeowHash meow_hash_impl
#include "meow_hash_x64_aesni.h"
#undef MeowHash

typedef meow_u128 MeowHash;

typedef struct MeowHashNode {
    MeowHash key;
    void    *val;
    struct MeowHashNode *next;
} MeowHashNode;

typedef struct {
    MeowHashNode *buckets[1024];
} MeowHashTable;

static inline MeowHashTable *meow_hash_table_new(void) {
    return (MeowHashTable *)calloc(1, sizeof(MeowHashTable));
}

static inline void *meow_hash_table_get(MeowHashTable *t, MeowHash key) {
    if (!t) return NULL;
    uint64_t hash64 = MeowU64From(key, 0);
    size_t idx = hash64 % 1024;
    MeowHashNode *node = t->buckets[idx];
    while (node) {
        if (MeowHashesAreEqual(node->key, key))
            return node->val;
        node = node->next;
    }
    return NULL;
}

static inline void meow_hash_table_set(MeowHashTable *t, MeowHash key, void *val) {
    if (!t) return;
    uint64_t hash64 = MeowU64From(key, 0);
    size_t idx = hash64 % 1024;
    MeowHashNode *node = t->buckets[idx];
    MeowHashNode *prev = NULL;
    while (node) {
        if (MeowHashesAreEqual(node->key, key)) {
            if (val == NULL) {
                if (prev) {
                    prev->next = node->next;
                } else {
                    t->buckets[idx] = node->next;
                }
                free(node);
            } else {
                node->val = val;
            }
            return;
        }
        prev = node;
        node = node->next;
    }
    if (val != NULL) {
        node = (MeowHashNode *)malloc(sizeof(MeowHashNode));
        node->key  = key;
        node->val  = val;
        node->next = t->buckets[idx];
        t->buckets[idx] = node;
    }
}

static inline MeowHash meow_hash_string(const char *str) {
    if (!str) str = "";
    return meow_hash_impl(MeowDefaultSeed, strlen(str), (void *)str);
}

#endif /* LEXT_HASH_H */
