 //
//  array.c
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

#include "box.h"

#define arrset(p) \
    box_set(__box, p + MINIBOX_VALUE, __value); \
    box_set(__box, p + MINIBOX_TYPE , &__type)

box_t new_array()
{
    return box_create(MINIBOX_TYPE_ARRAY);
}

void* array_get(box_t __box, long __index)
{
    return BOXB + (__index * V2S);
}

void array_remove(box_t __box, long __index)
{
    long p = __index * V2S;
    
    box_free_value(BOXB + p);
    
    if (box_move(__box, p + V2S, p, (BOXS - V2S) - p))
        return;
    
    if (box_reallocated(__box, -V2S)) return;
}

long array_count(box_t __box)
{
    return BOXS / V2S;
}

#pragma mark - Set

void array_set(box_t __box, long __position, const void *__value, long __type)
{
    box_free_value(box_get(__box, __position));
    
    arrset(__position);
}

void array_set_boolean(box_t __box, long __index, long __value)
{
    array_set(__box, __index * V2S, &__value, MINIBOX_TYPE_BOOLEAN);
}

void array_set_null(box_t __box, long __index)
{
    long v =0;
    array_set(__box, __index * V2S, &v, MINIBOX_TYPE_NULL);
}

void array_set_number(box_t __box, long __index, long __value)
{
    array_set(__box, __index * V2S, &__value, MINIBOX_TYPE_NUMBER);
}

void array_set_string(box_t __box, long __index, int __vmt, const char *__value)
{
    array_set(__box, __index * V2S, &__value, MINIBOX_TYPE_STRING + __vmt);
}

void array_set_box(box_t __box, long __index, int __vmt, long __value)
{
    array_set(__box, __index * V2S, &__value, box_type(__value) + __vmt);
}

#pragma mark - Add

void array_add(box_t __box, const void *__value, long __type)
{
    if (box_reallocated(__box, V2S)) return;
    
    arrset(BOXS - V2S);
}

void array_add_boolean(box_t __box, long __value)
{
    array_add(__box, &__value, MINIBOX_TYPE_BOOLEAN);
}

void array_add_null(box_t __box)
{
    long value = 0;
    array_add(__box, &value, MINIBOX_TYPE_NULL);
}

void array_add_number(box_t __box, double __value)
{
    array_add(__box, &__value, MINIBOX_TYPE_NUMBER);
}

void array_add_string(box_t __box, int __vmt, const char *__value)
{
    array_add(__box, &__value, MINIBOX_TYPE_STRING + __vmt);
}

void array_add_box(box_t __box, int __vmt, box_t __value)
{
    array_add(__box, &__value, box_type(__value) + __vmt);
}

#pragma mark - Insert

void array_insert(box_t __box, long __index, const void *__value, long __type)
{
    long p = __index * V2S;
    
    if (box_reallocated(__box, V2S)) return;
    
    if (box_move(__box, p, p + V2S, (BOXS - V2S) - p))
        return;
    
    arrset(p);
}

void array_insert_boolean(box_t __box, long __index, long __value)
{
    array_insert(__box, __index, &__value, MINIBOX_TYPE_BOOLEAN);
}

void array_insert_null(box_t __box, long __index)
{
    long value = 0;
    array_insert(__box, __index, &value, MINIBOX_TYPE_NULL);
}

void array_insert_number(box_t __box, long __index, double __value)
{
    array_insert(__box, __index, &__value, MINIBOX_TYPE_NUMBER);
}

void array_insert_string(box_t __box, long __index, int __vmt, const char *__value)
{
    array_insert(__box, __index, &__value, MINIBOX_TYPE_STRING + __vmt);
}

void array_insert_box(box_t __box, long __index, int __vmt, box_t __value)
{
    array_insert(__box, __index, &__value, box_type(__value) + __vmt);
}
