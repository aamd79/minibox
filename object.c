//
//  object.c
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

#include <string.h>
#include "box.h"

box_t new_object(void) {
    return box_create(MINIBOX_TYPE_OBJECT);
}

long object_index(box_t __box, const char *__key)
{
    long i;
    
    for (i = 0; i < BOXS; i += V3S) {
        if (!strcmp(*((char **)(BOXB + i + MINIBOX_KEY)), __key))
            return i / V3S;
    }
    
    return -1;
}

void object_set_at(box_t __box, long __index, const void *__value, long __type)
{
    long p = __index * V3S;
    
    box_free_value(BOXB + p);
    
    box_set(__box, p + MINIBOX_VALUE, __value);
    box_set(__box, p + MINIBOX_TYPE, &__value);
}

void object_set(box_t __box, const char *__key, const void *__value, long __type)
{
    long index = object_index(__box, __key);
    
    if (index < 0) return;
    
    object_set_at(__box, index, __value, __type);
}

void object_put(box_t __box, const char *__key, int _kf, const void *__value, long __type)
{
    long index = object_index(__box, __key);
    
    if (index < 0)
    {
        if (box_reallocated(__box, V3S)) return;
        
        const long pos = BOXS - V3S;
        const char *key = _kf ? __key : box_copy_str(__key);
        
        box_set(__box, pos + MINIBOX_VALUE, __value);
        box_set(__box, pos + MINIBOX_TYPE, &__type);
        box_set(__box, pos + MINIBOX_KEY,  &key);
    }
    else
    {
        object_set_at(__box, index, __value, __type);
    }
}

void* object_get(box_t __box, const char *__key)
{
    long index = object_index(__box, __key);
    
    if (index < 0) return NULL;
    
    return BOXB + (index * V3S);
}

void object_remove(box_t __box, const char *__key)
{
    long p = object_index(__box, __key) * V3S;
    
    if (p < 0) return;
    
    box_free_value(BOXB + p);
    
    if (box_move(__box, p + V3S, p, (BOXS - V3S) - p))
        return;
    
    if (box_reallocated(__box, -V3S)) return;
}

long object_attributes(box_t __box) {
    return BOXS / V3S;
}

#pragma mark - Set

void object_set_null(box_t __b, const char *__k) {
    long v = 0;
    object_set(__b, __k, &v, MINIBOX_TYPE_NULL);
}

void object_set_boolean(box_t __b, const char *__k, long __v) {
    object_set(__b, __k, &__v, MINIBOX_TYPE_BOOLEAN);
}

void object_set_number(box_t __b, const char *__k, double __v) {
    object_set(__b, __k, &__v, MINIBOX_TYPE_NUMBER);
}

void object_set_string(box_t __b, const char *__k, int _vf, const char *__v) {
    object_set(__b, __k, &__v, MINIBOX_TYPE_STRING + _vf);
}

void object_set_box(box_t __b, const char *__k, int _vf, box_t  __v) {
    object_set(__b, __k, &__v, box_type(__v) + _vf);
}

#pragma mark - Put

void object_put_null(box_t __b, int _kf, const char *__k) {
    long v = 0;
    object_put(__b, __k, _kf, &v, MINIBOX_TYPE_NULL);
}

void object_put_boolean(box_t __b, int _kf, const char *__k, long __v) {
    object_put(__b, __k, _kf, &__v, MINIBOX_TYPE_BOOLEAN);
}

void object_put_number(box_t __b, int _kf, const char *__k, double __v) {
    object_put(__b, __k, _kf, &__v, MINIBOX_TYPE_NUMBER);
}

void object_put_string(box_t __b, int _kf, const char *__k, int _vf, const char *__v) {
    object_put(__b, __k, _kf, &__v, MINIBOX_TYPE_STRING + _vf);
}

void object_put_box(box_t __b, int _kf, const char *__k, int _vf, box_t __v) {
    object_put(__b, __k, _kf, &__v, box_type(__v) + _vf);
}

