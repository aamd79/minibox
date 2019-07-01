//
//  box.c
//  minibox
//
//  Created by Antonio Angel Martínez Domínguez on 1/6/19.
//
//  Copyright 2019 Rokit Systems
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "box.h"

#define BOX ((long *) __box)

box_t box_create(long __type)
{
    long *box;
    void *buffer;
    
    if (!(box = malloc(32)))
    {
        printf("<< minibox::box::new_box->malloc() >>\nError al recervar memoria.\n");
        return 0;
    }
    
    if (!(buffer = malloc(32)))
    {
        free(box);
        printf("<< minibox::box::new_box->malloc() >>\nError al recervar memoria.\n");
        return 0;
    }
    
    box[0] = (long)buffer;
    box[1] = 0;
    box[2] = __type;
    box[3] = 32;
    
    return (long) box;
}

int box_allocated(box_t __box, long __size)
{
    void *tmp;
    
    if (!(tmp = malloc(__size)))
        return -1;
    
    BOX[0] = (long) tmp;
    BOX[1] = __size;
    BOX[3] = 0;
    
    return 0;
}

int box_reallocated(box_t __box, long __size)
{
    void *tmp;
    long m = BOXM;
    long s = BOXS + __size;
    
    if (m < s)
    {
        if (m == 0) return -1;
        
        while (m < s) m *= 2;
        
        if (!(tmp = realloc(BOXB, m)))
        {
            printf("<< minibox::box::box_reallocated->realloc() >>\nError al recervar memoria.\n");
            return -1;
        }
        
        BOX[0] = (long) tmp;
        BOX[3] = m;
    }
    
    BOX[1] += __size;
    
    return 0;
}

int box_finalize(box_t __box)
{
    void *tmp;
    long s = BOXS;
    
    if (!(tmp = realloc(BOXB, s)))
    {
        printf("<< minibox::box::box_reallocated->realloc() >>\nError al recervar memoria.\n");
        return -1;
    }
    
    BOX[0] = (long) tmp;
    BOX[3] = 0;
    return 0;
}

void* box_buffer(box_t __box)
{
    return BOXB;
}

long box_size(box_t __box)
{
    return BOXS;
}

long box_type(box_t __box)
{
    return BOXT;
}

void box_set(box_t __box, long __position, const void *__value)
{
    long *v = BOXB + __position;
     *v = *((long *)__value);
}

void * box_get(box_t __box, long __position)
{
    return BOXB + __position;
}

int box_move(box_t __box, long __src, long __dst, long __size)
{
    return !memmove(BOXB + __dst, BOXB + __src, __size);
}

void box_free_value(void *__value)
{
    long type = *((long *) (__value + MINIBOX_TYPE));
    switch (type)
    {
        case MINIBOX_TPAR_STRING:
            free(*(void **)__value);
            break;
            
        case MINIBOX_TPAR_ARRAY:
        case MINIBOX_TPAR_OBJECT:
            free_box(*(box_t *)__value);
            break;
            
        default: break;
    }
}

void free_box(box_t __box)
{
    long i, s = BOXS;
    
    switch (BOXT)
    {
        case MINIBOX_TYPE_ARRAY:
        case MINIBOX_TPAR_ARRAY:
            for (i = 0; i < s; i += V2S)
                box_free_value(BOXB + i);
            break;
            
        case MINIBOX_TYPE_OBJECT:
        case MINIBOX_TPAR_OBJECT:
            for (i = 0; i < s; i += V3S)
            {
                box_free_value(BOXB + i);
                free(*(char **)(BOXB + i + MINIBOX_KEY));
            }
            break;
            
        case MINIBOX_TYPE_STREAM:
            break;
            
        default: return;
    }
    
    free(BOXB);
    free(BOX);
}

char* box_copy_str(const char *__key)
{
    char *str;
    
    if (!(str = malloc(strlen(__key) + 1)))
        return NULL;
    
    strcpy(str, __key);
    
    return str;
}

char * box_number_string(double __value)
{
    static char buf[32];
    /*******************************************************
     AQUI HAY UN BUG.
     LA PARTE DECIMAL NO ES CORRECTA CON ALGUNOS NUMEROS.
     QUITA LOS CEROS AL PRINCIPIO DE LA PARATE DECIMAL.
     
    unsigned long ent = (long)__value;
    unsigned fra = 0;
    
    __value = fabs(__value);
    __value -= ent;
    __value *= 1000000000;
    
    fra = __value;
    
    if(fra < __value) ++fra;
    else if(fra > __value) --fra;
    
    if(fra) while(!(fra % 10)) fra /= 10;
    
    if (fra == 0)
        sprintf(buf, "%ld", ent);
    else
        sprintf(buf, "%ld.%u", ent, fra);
     
    ******************************************************/
    
    
    if (__value - (long)__value == 0.0) {
        sprintf(buf, "%ld", (long) __value);
    } else {
        sprintf(buf, "%f", __value);
        
        long i;
        
        for (i = strlen(buf) -1; i != 0; i--) {
            if (buf[i] == '0') {
                buf[i] = 0;
            } else
                break;
        }
    }
    
    return buf;
}

void box_put_value(box_t __str, long __type, const void *__value, int *level)
{
    switch (__type) {
        case MINIBOX_TYPE_NULL:
            stream_add(__str, MINIBOX_VALUE_NULL);
            break;
        case MINIBOX_TYPE_BOOLEAN:
            if (*(long *) __value == 0) {
                stream_add(__str, MINIBOX_VALUE_FALSE);
            } else {
                stream_add(__str, MINIBOX_VALUE_TRUE);
            }
            break;
        case MINIBOX_TYPE_NUMBER:
            stream_add_number(__str, *(double *) __value);
            break;
            
        default: break;
    }
}
