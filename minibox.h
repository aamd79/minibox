//
//  minibox.h
//  MiniBox
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

#ifndef minibox_h
#define minibox_h

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    MINIBOX_MEMORY_RETAINT = 0x0,
    MINIBOX_MEMORY_RELEASE = 0x1,
    
    MINIBOX_TYPE_NULL    = 0x2,
    MINIBOX_TYPE_BOOLEAN = 0x4,
    MINIBOX_TYPE_NUMBER  = 0x6,
    MINIBOX_TYPE_STRING  = 0x8,
    MINIBOX_TYPE_STREAM  = 0xA,
    MINIBOX_TYPE_ARRAY   = 0xC,
    MINIBOX_TYPE_OBJECT  = 0xE
};

typedef long box_t;

void free_box(box_t __box);

//***************************************************************
    
box_t new_array(void);
    
void array_remove(box_t __box, long __index);
void* array_get(box_t __box, long __index);
long array_count(box_t __box);

void array_add_null(box_t __box);
void array_add_boolean(box_t __box, long __value);
void array_add_number(box_t __box, double __value);
void array_add_string(box_t __box, int __vmt, const char *__value);
void array_add_box(box_t __box, int __vmt, box_t __value);

void array_insert_null(box_t __box, long __index);
void array_insert_boolean(box_t __box, long __index, long __value);
void array_insert_number(box_t __box, long __index, double __value);
void array_insert_string(box_t __box, long __index, int __vmt, const char *__value);
void array_insert_box(box_t __box, long __index, int __vmt, box_t __value);
    
void array_set_boolean(box_t __box, long __index, long __value);
void array_set_null(box_t __box, long __index);
void array_set_number(box_t __box, long __index, long __value);
void array_set_string(box_t __box, long __index, int __vmt, const char *__value);
void array_set_box(box_t __box, long __index, int __vmt, long __value);

#define array_get_null(x, i) (*((long *) array_get(x, i)))
#define array_get_boolean(x, i) (*((long *) array_get(x, i)))
#define array_get_number(x, i) (*((double *) array_get(x, i)))
#define array_get_string(x, i) (*((char **) array_get(x, i)))
#define array_get_box(x, i) (*((box_t *) array_get(x, i)))

//***************************************************************
    
box_t new_object(void);

void object_remove(box_t __box, const char *__key);
void* object_get( box_t __box, const char *__key);
long object_attributes(box_t __box);

void object_put_null   (box_t __b,   int __kmt, const char *__key                                );
void object_put_boolean(box_t __b,   int __kmt, const char *__key,            long        __value);
void object_put_number (box_t __box, int __kmt, const char *__key,            double      __value);
void object_put_string (box_t __box, int __kmt, const char *__key, int __vmt, const char *__value);
void object_put_box    (box_t __box, int __kmt, const char *__key, int __vmt, box_t       __value);

void object_set_null   (box_t __b, const char *__k                            );
void object_set_boolean(box_t __b, const char *__k,            long        __v);
void object_set_number (box_t __b, const char *__k,            double      __v);
void object_set_string (box_t __b, const char *__k, int __vmt, const char *__v);
void object_set_box    (box_t __b, const char *__k, int __vmt, box_t       __v);

#define object_get_null(x, k) (*((long *) object_get(x, k)))
#define object_get_boolean(x, k) (*((long *) object_get(x, k)))
#define object_get_number(x, k) (*((double *) object_get(x, k)))
#define object_get_string(x, k) (*((char **) object_get(x, k)))
#define object_get_box(x, k) (*((box_t *) object_get(x, k)))

//***************************************************************

box_t new_stream(void);
box_t stream_load(const char *__path);
int stream_save(box_t __box, const char *__path);
void stream_add(box_t __box, const char *__value);
void stream_add_char(box_t __box, char __value);
void stream_add_number(box_t __box, double __value);
void stream_paragraph(box_t __box, int __level);
void stream_open_hierarchy(box_t __box, char __symbol, int __level);
void stream_close_hierarchy(box_t __box, char __symbol, int __level);
void stream_add_between(box_t __box, const char *__str, char __char);
void stream_finalize(box_t __box);
char* stream_get(box_t __box);
void stream_print(box_t __box);

//***************************************************************

box_t object_from_json_string(const char *__string);
box_t object_from_json_file(const char *__path);
box_t json_stream_from_object(box_t __box);
int json_file_from_object(box_t __box, const char *__path);

box_t xml_object_from_string(const char *__string);
box_t xml_object_from_file(const char *__path);
box_t xml_stream_from_object(box_t __box);
int xml_file_from_object(box_t __box, const char *__path);
    
#ifdef __cplusplus
}
#endif

#endif /* minibox_h */
