#pragma once

typedef struct {
        SCM proc;
        void *args;
} dscm_call_data_t;

static inline SCM
dscm_get_variable(const char *name)
{
        return scm_variable_ref(scm_c_lookup(name));
}

static inline SCM
dscm_alist_get(SCM alist, const char* key)
{
        return scm_assoc_ref(alist, scm_from_utf8_string(key));
}

static inline char*
dscm_alist_get_string(SCM alist, const char* key)
{
        SCM value = dscm_alist_get(alist, key);
        if (scm_is_string(value))
                return scm_to_locale_string(value);
        return NULL;
}

static inline int
dscm_alist_get_int(SCM alist, const char* key)
{
        SCM value = dscm_alist_get(alist, key);
        if (scm_is_bool(value))
                return scm_is_true(value) ? 1 : 0;
        return scm_to_int(value);
}

static inline unsigned int
dscm_get_list_length(SCM list)
{
        return scm_to_unsigned_integer(scm_length(list), 0, -1);
}

static inline SCM
dscm_get_list_item(SCM list, unsigned int index)
{
        return scm_list_ref(list, scm_from_unsigned_integer(index));
}

static inline void *
dscm_iterate_list(SCM list, size_t elem_size,
        void (*iterator)(unsigned int, SCM, void*), unsigned int *length_var)
{
        SCM item;
        unsigned int i = 0, length = 0;
        length = dscm_get_list_length(list);
        void *allocated = calloc(length, elem_size);
        for (; i < length; i++) {
                item = dscm_get_list_item(list, i);
                (*iterator)(i, item, allocated);
        }
        if (length_var)
                *length_var = length;
        return allocated;
}

static inline scm_t_bits *
dscm_alist_get_proc_pointer(SCM alist, const char *key)
{
        scm_t_bits *proc = NULL;
        SCM value = dscm_alist_get(alist, key);
        if (scm_is_false(value))
                return proc;
        SCM eval = scm_primitive_eval(value);
        /* SCM_UNPACK_POINTER is only allowed on expressions where SCM_IMP is 0 */
        if (SCM_IMP(eval) == 1)
                BARF("dscm: invalid callback procedure. SCM_IMP(proc) = 1");
        if (scm_procedure_p(eval) == SCM_BOOL_T) {
                proc = SCM_UNPACK_POINTER(eval);
                scm_gc_protect_object(eval);
        }
        return proc;
}

static inline void*
dscm_call_block_callback(void *data)
{
        dscm_call_data_t *wrapper = (dscm_call_data_t*)data;
        SCM state = scm_from_pointer(wrapper->args, NULL);
        return scm_call_1(wrapper->proc, state);
}

// TODO: Replace void *data with the block state type
static inline void
dscm_safe_call(scm_t_bits *ptr, void *data)
{
        if (ptr == NULL)
                BARF("dscm: could not call proc that is NULL");
        dscm_call_data_t wrapper = {.proc = SCM_PACK_POINTER(ptr), .args = data};
        scm_c_with_continuation_barrier(&dscm_call_block_callback, &wrapper);
}
