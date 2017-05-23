#include "hash.h"

#define HASH_SEED 4759

// https://en.wikipedia.org/wiki/MurmurHash
uint32_t murmur3_32(const uint8_t *key, size_t len, uint32_t seed)
{
    uint32_t h = seed;
    if (len > 3)
    {
        const uint32_t *key_x4 = (const uint32_t*)key;
        size_t i = len >> 2;
        do
        {
            uint32_t k = *key_x4++;
            k *= 0xcc9e2d51;
            k = (k << 15) | (k >> 17);
            k *= 0x1b873593;
            h ^= k;
            h = (h << 13) | (h >> 19);
            h += (h << 2) + 0xe6546b64;
        } while (--i);
        key = (const uint8_t*)key_x4;
    }
    if (len & 3)
    {
        size_t i = len & 3;
        uint32_t k = 0;
        key = &key[i - 1];
        do
        {
            k <<= 8;
            k |= *key--;
        } while (--i);
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        h ^= k;
    }
    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

static hash_entry_t *new_entry(value_t key, value_t value)
{
    hash_entry_t *entry = (hash_entry_t*)calloc(1, sizeof(hash_entry_t));
    entry->key = key;
    entry->value = value;
    entry->next = NULL;
    return entry;
}

hashtable_t *hashtable_new(uint32_t size)
{
    hashtable_t *htable = (hashtable_t*)calloc(1, sizeof(hashtable_t));
    htable->table = (hash_entry_t**)calloc(size, sizeof(hash_entry_t*));
    for (uint32_t i = 0; i < size; i++)
    {
        htable->table[i] = NULL;
    }
    htable->size = size;

    return htable;
}

void hashtable_free(hashtable_t *htable)
{
    for (uint32_t i = 0; i < htable->size; i++)
    {
        hash_entry_t *node = htable->table[i];
        while (node)
        {
            hash_entry_t *next = node->next;
            free(node);
            node = next;
        }
    }
    free(htable->table);
    free(htable);
}

void hashtable_dump(hashtable_t *htable)
{
    for (size_t i = 0; i < htable->size; i++)
    {
        hash_entry_t *node = htable->table[i];
        while (node)
        {
            printf("key: "); value_print(node->key);
            printf("value: "); value_print(node->value);
            node = node->next;
        }
    }
}

static uint32_t hash_string(const char *s)
{
    return murmur3_32(s, strlen(s), HASH_SEED);
}

static uint32_t hash_value(value_t v)
{
    if (IS_STR(v))
    {
        return hash_string(AS_STR(v));
    }
}

void hashtable_set(hashtable_t *htable, value_t key, value_t value)
{
    uint32_t bin = hash_value(key) % htable->size;

    hash_entry_t *last = NULL;
    hash_entry_t *newpair = NULL;
    hash_entry_t *next = htable->table[bin];

    while (next != NULL && !value_equals(key, next->key))
    {
        last = next;
        next = next->next;
    }

    if (next != NULL && value_equals(key, next->key))
    {
        next->value = value;
    }
    else
    {
        newpair = new_entry(key, value);
        if (next == htable->table[bin])
        {
            newpair->next = next;
            htable->table[bin] = newpair;
        }
        else if (next == NULL)
        {
            last->next = newpair;
        }
        else
        {
            newpair->next = next;
            last->next = newpair;
        }
    }
}

value_t *hashtable_get(hashtable_t *htable, value_t key)
{
    uint32_t bin = hash_value(key) % htable->size;

    hash_entry_t *node = htable->table[bin];
    while (node != NULL && !value_equals(key, node->key))
    {
        node = node->next;
    }

    if (node == NULL || !value_equals(key, node->key))
    {
        return NULL;
    }
    else
    {
        return &node->value;
    }
}

void hashtable_iterate(hashtable_t *htable, hash_iterator_func iterator)
{
    for (size_t i = 0; i < htable->size; i++)
    {
        hash_entry_t *node = htable->table[i];
        while (node)
        {
            iterator(node);
            node = node->next;
        }
    }
}