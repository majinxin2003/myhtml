/*
 Copyright (C) 2015-2016 Alexander Borisov
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 
 Author: lex.borisov@gmail.com (Alexander Borisov)
*/

#include "myhtml/utils/mcobject.h"

mcobject_t * mcobject_create(void)
{
    return myhtml_calloc(1, sizeof(mcobject_t));
}

myhtml_status_t mcobject_init(mcobject_t *mcobject, size_t chunk_size, size_t struct_size)
{
    mcobject->struct_size = struct_size;
    mcobject->chunk_size  = chunk_size;
    
    mcobject->chunk = NULL;
    
    mcobject->cache_length = 0;
    mcobject->cache_size = chunk_size;
    mcobject->cache = (void**)myhtml_malloc(sizeof(void*) * mcobject->cache_size);
    
    if(mcobject->cache == NULL)
        return MyHTML_STATUS_MCOBJECT_ERROR_CACHE_CREATE;
    
    return MyHTML_STATUS_OK;
}

void mcobject_clean(mcobject_t *mcobject)
{
    if(mcobject->chunk == NULL)
        return;
    
    mcobject_chunk_t* chunk = mcobject->chunk;
    
    while(chunk->next)
        chunk = chunk->next;
    
    while(chunk) {
        mcobject_chunk_t* tmp = chunk->prev;
        
        if(chunk->begin) {
            myhtml_free(chunk->begin);
        }
        
        myhtml_free(chunk);
        
        chunk = tmp;
    }
    
    mcobject->chunk = NULL;
    mcobject->cache_length = 0;
}

mcobject_t * mcobject_destroy(mcobject_t *mcobject, bool destroy_self)
{
    if(mcobject == NULL)
        return NULL;
    
    mcobject_clean(mcobject);
    
    if(mcobject->cache) {
        myhtml_free(mcobject->cache);
        mcobject->cache = NULL;
    }
    
    if(destroy_self) {
        myhtml_free(mcobject);
        return NULL;
    }
    
    return mcobject;
}

void mcobject_chunk_malloc(mcobject_t* mcobject, myhtml_status_t* status)
{
    if(status)
        *status = MyHTML_STATUS_OK;
    
    mcobject_chunk_t* chunk;
    
    if(mcobject->chunk && mcobject->chunk->next) {
        mcobject->chunk = mcobject->chunk->next;
        
        mcobject->chunk->length = 0;
        
        return;
    }
    else {
        chunk = myhtml_calloc(1, sizeof(mcobject_chunk_t));
        
        if(chunk == NULL) {
            if(status)
                *status = MyHTML_STATUS_MCOBJECT_ERROR_CHUNK_CREATE;
            
            return;
        }
        
        chunk->begin = myhtml_malloc(mcobject->struct_size * mcobject->chunk_size);
        
        if(chunk->begin == NULL) {
            if(status)
                *status = MyHTML_STATUS_MCOBJECT_ERROR_CHUNK_INIT;
            
            myhtml_free(chunk);
            return;
        }
        
        chunk->size = mcobject->chunk_size;
    }
    
    if(mcobject->chunk == NULL) {
        mcobject->chunk = chunk;
        return;
    }
    
    chunk->prev = mcobject->chunk;
    mcobject->chunk->next = chunk;
    
    mcobject->chunk = chunk;
}

void * mcobject_malloc(mcobject_t *mcobject, myhtml_status_t* status)
{
    if(mcobject->cache_length) {
        if(status)
            *status = MyHTML_STATUS_OK;
        
        mcobject->cache_length--;
        return mcobject->cache[ mcobject->cache_length ];
    }
    
    mcobject_chunk_t* chunk = mcobject->chunk;
    
    if(chunk == NULL || chunk->length >= chunk->size)
    {
        mcobject_chunk_malloc(mcobject, status);
        
        if(status && *status)
            return NULL;
        
        chunk = mcobject->chunk;
    }
    
    if(status)
        *status = MyHTML_STATUS_OK;
    
    chunk->length++;
    return &chunk->begin[((chunk->length - 1) * mcobject->struct_size)];
}

myhtml_status_t mcobject_free(mcobject_t *mcobject, void *entry)
{
    if(mcobject->cache_length >= mcobject->cache_size) {
        size_t new_size = mcobject->cache_size << 1;
        
        void **tmp = (void**)myhtml_realloc(mcobject->cache, sizeof(void*) * new_size);
        
        if(tmp) {
            mcobject->cache = tmp;
            mcobject->cache_size = new_size;
        }
        else
            return MyHTML_STATUS_MCOBJECT_ERROR_CACHE_REALLOC;
    }
    
    mcobject->cache[ mcobject->cache_length ] = entry;
    mcobject->cache_length++;
    
    return MyHTML_STATUS_OK;
}


