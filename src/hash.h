#ifndef __HASH_H_
#define __HASH_H_

#include <stdbool.h>

#include "value.h"

typedef struct hash_entry_s
{
    value_t key;
    value_t value;
    struct hash_entry_s *next;
} hash_entry_t;

typedef struct
{
    uint32_t size;
    hash_entry_t **table;

} hashtable_t;

typedef void(*hash_iterator_func)(hash_entry_t *node);

hashtable_t *hashtable_new(uint32_t size);
void hashtable_free(hashtable_t *htable);
void hashtable_dump(hashtable_t *htable);

void hashtable_set(hashtable_t *htable, value_t key, value_t value);
value_t *hashtable_get(hashtable_t *htable, value_t key);

void hashtable_iterate(hashtable_t *htable, hash_iterator_func iterator);

#endif