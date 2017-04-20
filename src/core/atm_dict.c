#include <atm_core.h>


/*
 * Private
 * */


/* entry type lifecycle */
static atm_dict_entry_t *
atm_dict_entry_new(void *key, void *val);

static void *
atm_dict_entry_spec(void *entry);

static atm_bool_t 
atm_dict_entry_match(void *entry, void *hint); 

static uint64_t 
atm_dict_entry_hash(void *entry);

static atm_int_t 
atm_dict_entry_cmp(void *entry1, void *entry2);

static atm_str_t *
atm_dict_entry_str(void *entry);

static void 
atm_dict_entry_free(void *entry);


static atm_T_t ATM_DICT_ENTRY_T = {
    atm_dict_entry_spec,
    atm_dict_entry_match,
    atm_dict_entry_hash,
    atm_dict_entry_cmp,
    atm_dict_entry_str,
    atm_dict_entry_free,
};


/* funcs */
static atm_uint_t 
atm_dict_hkey(atm_dict_t *dict, void *key);

static atm_dict_entry_t *
atm_dict_entry(atm_dict_t *dict, void *key); 


/*
 * Public
 * */


atm_T_t ATM_DICT_T = {
    atm_dict_spec,
    atm_dict_match,
    atm_dict_hash,
    atm_dict_cmp,
    atm_dict_str,
    atm_dict_free,
};


/* ---------------------IMPLEMENTATIONS--------------------------- */


/*
 * Private
 * */


static atm_dict_entry_t *
atm_dict_entry_new(void *key, void *val) 
{
    atm_dict_entry_t *res = NULL;

    res = (atm_dict_entry_t *)
        atm_alloc(sizeof(*res));

    res->dict = NULL;
    res->key = key; 
    res->val = val; 
    return res;
}


static void *
atm_dict_entry_spec(void *entry)
{
    void *res = NULL;
    return res;
}


static atm_bool_t 
atm_dict_entry_match(void *entry, void *hint)
{
    atm_bool_t res = ATM_FALSE;
    atm_T_t *k_t = NULL;
    atm_dict_entry_t *e = NULL;
    atm_dict_entry_t *h = NULL;

    e = (atm_dict_entry_t *)entry;
    h = (atm_dict_entry_t *)hint;
    if (e == NULL || hint == NULL) {
        atm_log_routine(ATM_LOG_ERROR, 
                "atm_dict_entry_match:"
                "entry carrupted should not NULL");
        exit(ATM_ERROR);
    } else {
        k_t = e->dict->k_type;
        res = k_t->match(e->key, h->key);
    }
    return res;
}


static uint64_t 
atm_dict_entry_hash(void *entry)
{
    uint64_t res = 0;
    atm_dict_entry_t *e = NULL; 
    atm_str_t *e_str = NULL;

    e = (atm_dict_entry_t *) entry;
    e_str = atm_str_ptr_tostr(e); 
    res = atm_siphash(e_str->val, e_str->len);

    atm_str_free(e_str);
    return res;
}


static atm_int_t 
atm_dict_entry_cmp(void *entry1, void *entry2)
{
    return ATM_CMP_EQ;
}


static atm_str_t *
atm_dict_entry_str(void *entry)
{
    atm_str_t *res = NULL;
    atm_str_t *ks = NULL;
    atm_str_t *vs = NULL;
    atm_T_t *k_t = NULL;
    atm_T_t *v_t = NULL;
    atm_dict_entry_t *e = NULL;

    e = (atm_dict_entry_t *) entry;
    if (e != NULL) {
        if (e->dict != NULL) {
            k_t = e->dict->k_type;
            if (k_t != NULL && k_t->str != NULL) {
                ks = k_t->str(e->key);
            } else {
                ks = atm_str_ptr_tostr(e->key);
            }
            if (v_t != NULL && v_t->str != NULL) {
                vs = v_t->str(e->val);
            } else {
                vs = atm_str_ptr_tostr(e->val);
            }
        }
    }
    res = atm_str_fmt("key[%s];val[%s];",ks,vs);

    atm_str_free(ks);
    atm_str_free(vs);
    return res;
}


static void 
atm_dict_entry_free(void *entry) 
{
    atm_dict_entry_t *e = NULL;
    atm_dict_t *dict = NULL;

    if (entry != NULL) {
        e = (atm_dict_entry_t *)entry;
        dict = e->dict; 
        if (dict != NULL) {
            if (dict->deep_free) {
                dict->k_type->free(e->key);
                dict->v_type->free(e->val);
            }
        }
        atm_free(e);
    }
}


static atm_uint_t 
atm_dict_hkey(atm_dict_t *dict, void *k) 
{
    atm_uint_t res = 0;
    uint64_t hash_key = 0;
    atm_T_t *k_t = NULL;

    k_t = dict->k_type;
    hash_key = k_t->hash(k);
    res = hash_key % dict->bucket_size;
    return res;
}


static atm_dict_entry_t *
atm_dict_entry(atm_dict_t *dict, void *key) 
{
    atm_dict_entry_t *res = NULL;
    atm_dict_entry_t *hint = NULL;
    atm_uint_t hash_key = 0;
    atm_list_t *lptr = NULL;

    hash_key = atm_dict_hkey(dict, key);
    lptr = dict->bucket[hash_key];
    /* create a tmp hint to test */
    hint = atm_dict_entry_new(key, NULL);
    res = (atm_dict_entry_t *)
        atm_list_find(lptr, hint);

    /* dont forget free hint */
    atm_dict_entry_free(hint);
    return res;
}


/*
 * Public
 * */


void 
atm_dict_init()
{
    /* TODO */
}


atm_dict_t *
atm_dict_new(atm_T_t *k_type, atm_T_t *v_type) 
{
    atm_uint_t  bsz = 0;
    atm_dict_t *dict = NULL;

    bsz = ATM_DICT_INITIAL_BUCKET_SIZE;
    dict = (atm_dict_t *) 
        atm_alloc(sizeof(atm_dict_t));

    dict->deep_free = ATM_TRUE;
    dict->bucket = (atm_list_t **) 
        atm_alloc(sizeof(atm_list_t *) * bsz);

    dict->bucket_size = bsz;
    dict->size = 0;
    dict->k_type = k_type;
    dict->v_type = v_type;

    return dict;
}


void *
atm_dict_spec(void *dict)
{
    void *res = NULL;
    return res;
}


atm_bool_t
atm_dict_match(void *dict1, void *dict2)
{
    atm_bool_t res = ATM_FALSE;
    if (dict1 == dict2) {
        res = ATM_TRUE;
    }
    return res;
}


uint64_t
atm_dict_hash(void *dict)
{
    uint64_t res = 0;
    atm_dict_t *d = NULL; 
    atm_str_t *d_str = NULL;

    d = (atm_dict_t *) dict;
    d_str = atm_str_ptr_tostr(d); 
    res = atm_siphash(d_str->val,d_str->len);

    atm_str_free(d_str);
    return res;
}


atm_int_t
atm_dict_cmp(void *dict1, void *dict2)
{
    return ATM_CMP_EQ;
}


atm_str_t *
atm_dict_str(void *dict)
{
    atm_str_t *res = NULL;
    atm_dict_t *d = NULL; 

    d = (atm_dict_t *) dict;
    if (d != NULL) {
        res = atm_str_fmt(
            "deep_free[%d];bucket_size[%ld];size[%ld]",
            d->deep_free,
            d->bucket_size,
            d->size);
    } else {
        res = atm_str_new("NULL");
    }
    return res;
}


void
atm_dict_free(void *dict) 
{
    atm_dict_t *d = NULL;
    atm_uint_t  i = 0;
    atm_list_t *lptr = NULL;

    d = (atm_dict_t *) dict;
    for (;i<d->bucket_size; ++i) {
        lptr = d->bucket[i];
        if (lptr->deep_free) {
            /*
             * containers inner container must be free_deep
             * or else the top container's free can not triger
             * the basic val's free action
             */
            atm_list_free(lptr);
        } else {
            atm_log_routine(ATM_LOG_ERROR, 
                "dict inner list curraput");
            exit(ATM_ERROR);
        }
    }

    atm_free(d->bucket);
    atm_free(d);
}


atm_bool_t
atm_dict_contains(atm_dict_t *dict, void *key)
{
    atm_dict_entry_t *entry;
    entry = atm_dict_entry(dict, key);
    return entry==NULL?ATM_FALSE:ATM_TRUE;
}


void *
atm_dict_get(atm_dict_t *dict, void *key) 
{
    void * res = NULL;
    atm_dict_entry_t *entry = NULL;

    entry = atm_dict_entry(dict, key);
    if (entry != NULL) {
        res = entry->val;
    }
    return res;
}


void
atm_dict_set(atm_dict_t *dict, void *key, void *val)
{
    atm_uint_t hash_key = 0;
    atm_list_t *lptr = NULL; 
    atm_dict_entry_t *new_entry = NULL;
    atm_dict_entry_t *entry = NULL;
    
    entry = atm_dict_entry(dict, key);
    if (entry != NULL) {
        entry->val = val;
    } else {
        hash_key = atm_dict_hkey(dict, key);
        lptr = dict->bucket[hash_key];
        if (lptr == NULL) {
            lptr = atm_list_new(&ATM_DICT_ENTRY_T);
            /* lptr is part of data structure 
             * so need to set deep free */
            lptr->deep_free = ATM_TRUE;
            dict->bucket[hash_key] = lptr;
        }
        new_entry = atm_dict_entry_new(key,val);   
        new_entry->dict = dict;
        new_entry->list = lptr;
        atm_list_push(lptr, new_entry);
    }
}


void
atm_dict_del(atm_dict_t *dict, void *key) 
{
    atm_list_t *lptr = NULL;
    atm_dict_entry_t *entry = NULL;

    entry = atm_dict_entry(dict, key);
    if (entry != NULL) {
        lptr = entry->list;
        atm_list_del(lptr, entry);
    }
}