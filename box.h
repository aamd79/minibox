//
//  box.h
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

#ifndef box_h
#define box_h

#include <stdio.h>
#include "minibox.h"

#ifdef __cplusplus
extern "C" {
#endif

#define V1S 0x8
#define V2S 0x10
#define V3S 0x18

#define BOXB ( (void *) ((long *) __box)[0] )
#define BOXS ( ((long *) __box)[1] )
#define BOXT ( ((long *) __box)[2] )
#define BOXM ( ((long *) __box)[3] )

#define BOX_CREATE_ERROR "The box could not be created."
#define BOX_MEMORY_ERROR "Could not reserve memory."
#define ERROR(x, s) printf("\t%s\nError::xml->%s\n", x, s);
#define EIF(c, x, s) if(c){printf("\t%s\nError::xml->%s\n",x,s);return -1;}
#define WARNING(x, s) printf("\t%s\nWarning::xml->%s\n", x, s);

#define MINIBOX_VALUE_NULL "null"
#define MINIBOX_VALUE_FALSE "false"
#define MINIBOX_VALUE_TRUE "true"

enum {
    MINIBOX_TPAR_STRING = MINIBOX_TYPE_STRING + 1,
    MINIBOX_TPAR_ARRAY  = MINIBOX_TYPE_ARRAY  + 1,
    MINIBOX_TPAR_OBJECT = MINIBOX_TYPE_OBJECT + 1,
    MINIBOX_FALSE       = MINIBOX_TYPE_BOOLEAN,
    MINIBOX_TRUE        = MINIBOX_TYPE_BOOLEAN + 1
};

enum {
    MINIBOX_VALUE = 0x0,
    MINIBOX_TYPE = 0x8,
    MINIBOX_KEY = 0x10
};

box_t box_create(long __type);

int box_allocated(box_t __box, long __size);

int box_reallocated(box_t __box, long __size);

int box_finalize(box_t __box);

void box_set(box_t __box, long __position, const void *__value);

void* box_get(box_t __box, long __position);

int box_move(box_t __box, long __src, long __dst, long __size);

void* box_buffer(box_t __box);

long box_size(box_t __box);

long box_type(box_t __box);

void box_free_value(void *__value);

char* box_copy_str(const char *__str);

char * box_number_string(double __value);

void box_put_value(box_t __str, long __type, const void *__value, int *level);
    
#ifdef __cplusplus
}
#endif

#endif /* box_h */
