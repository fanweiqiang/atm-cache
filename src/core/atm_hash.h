#ifndef _ATM_HASH_H_INCLUDED_
#define _ATM_HASH_H_INCLUDED_

#include <atm_config.h>
#include <atm_core.h>

typedef struct {
    void *value;
    atm_str_t *key;
    struct atm_hash_entry_t *next;
} atm_hash_entry_t;


typedef struct {
    atm_hash_entry_t **table;
    atm_ulong_t size;
    atm_uint_t (* hash_func)(atm_str_t *key);
    atm_uint_t (* key_compare)(atm_str_t *key1, atm_str_t *key2);
} atm_hash_t;


void atm_hash_init();
atm_int_t atm_hash_contains(atm_string_t *key);
atm_uint_t atm_hash_key_func(atm_string_t *key);
void *atm_hash_get(atm_hash_t *hash, atm_string_t *key);
void *atm_hash_set(atm_hash_t *hash, atm_string_t *key, atm_string_t *value);
atm_int_t atm_hash_remove(atm_hash_t *hash, atm_string_t *key);


#endif
