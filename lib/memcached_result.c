/* 
  memcached_result_st are used to internally represent the return values from
  memcached. We use a structure so that long term as identifiers are added 
  to memcached we will be able to absorb new attributes without having 
  to addjust the entire API.
*/
#include "common.h"

memcached_result_st *memcached_result_create(memcached_st *memc, 
                                             memcached_result_st *ptr)
{
  /* Saving malloc calls :) */
  if (ptr)
  {
    memset(ptr, 0, sizeof(memcached_result_st));
    ptr->is_allocated= MEMCACHED_NOT_ALLOCATED;
  }
  else
  {
    ptr= (memcached_result_st *)malloc(sizeof(memcached_result_st));
    if (!ptr)
      return NULL;
    memset(ptr, 0, sizeof(memcached_result_st));
    ptr->is_allocated= MEMCACHED_ALLOCATED;
  }

  ptr->root= memc;
  memcached_string_create(memc, &ptr->value, 0);
  WATCHPOINT_ASSERT(ptr->value.string == NULL);
  WATCHPOINT_ASSERT(ptr->value.is_allocated == MEMCACHED_NOT_ALLOCATED);

  return ptr;
}

void memcached_result_free(memcached_result_st *ptr)
{
  memcached_string_free(&ptr->value);

  if (ptr->is_allocated == MEMCACHED_ALLOCATED)
    free(ptr);
}