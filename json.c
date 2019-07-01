//
//  json.c
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
#include "box.h"

typedef struct __json
{
    const char *source;
    const char *pointer;
    int obj_lev;
    int arr_lev;
} json_t;

void json_object(json_t *__json, box_t __obj);
void object_json(box_t __box, box_t __str, int *level);

box_t object_from_json_string(const char *__src)
{
    box_t obj = new_object();
    json_t json = { __src, __src, -1, -1 };
    
    json_object(&json, obj);
    
    return obj;
}

box_t json_stream_from_object(box_t __box)
{
    box_t str = new_stream();
    int level = 0;
    object_json(__box, str, &level);
    stream_finalize(str);
    return str;
}

box_t object_from_json_file(const char *__path)
{
    box_t box;
    box_t str;
    char *src;
    
    if (!(str = stream_load(__path)))
        return 0;
    
    if (!(box = new_object()))
    {
        ERROR(BOX_CREATE_ERROR, "object_from_json_file()")
        free_box(str);
        return 0;
    }
    
    src = box_buffer(str);
    json_t json = { src, src, -1, -1 };
    
    json_object(&json, box);
    
    free_box(str);
    
    return box;
}

int json_file_from_object(box_t __box, const char *__path)
{
    box_t str;
    
    if (!(str = json_stream_from_object(__box)))
        return -1;
    
    if (stream_save(str, __path))
        return -1;
    
    free_box(str);
    
    return 0;
}

static char * json_string(json_t *__json)
{
    char *str;
    size_t len = 0;
    const char *p = __json->pointer;
    
    while (*p != '"') if (*p != '\0') p++; else return NULL;
    
    len = p++ - __json->pointer;
    
    if (!(str = malloc(len + 1)))
        return NULL;
    
    memcpy(str, __json->pointer, len);
    
    str[len] = '\0';
    
    __json->pointer += len +1;
    
    return str;
}

static double json_number(json_t *__json)
{
    return atof(__json->pointer-1);
}

void json_array(json_t *__json, box_t __box)
{
    box_t box;
    long level = __json->arr_lev;
    int nc = 1;
    
    do
    {
        switch (*__json->pointer++)
        {
            // { (OBJECT)
            case 123:
                __json->obj_lev++;
                box = new_object();
                json_object(__json, box);
                array_add_box(__box, MINIBOX_MEMORY_RELEASE, box);
                break;
            //[ (ARRAY)
            case 91:
                __json->arr_lev++;
                box = new_array();
                json_array(__json, box);
                array_add_box(__box, MINIBOX_MEMORY_RELEASE, box);
                break;
            // ] (ARRAY)
            case 93:
                if (level == __json->arr_lev--)
                    return;
                break;
            // " (STRING)
            case 34:
                array_add_string(__box, MINIBOX_MEMORY_RELEASE, json_string(__json));
                break;
            // 0<->9 (NUMBER)
            case 48: case 49: case 50: case 51: case 52:
            case 53: case 54: case 55: case 56: case 57:
                if (nc)
                    array_add_number(__box, json_number(__json));
                nc = 0;
                break;
            // t (BOOLEAN)
            case 116:
                if (nc)
                    array_add_boolean(__box, 1);
                nc = 0;
                break;
            // f (BOOLEAN)
            case 102:
                if (nc)
                    array_add_boolean(__box, 0);
                nc = 0;
                break;
            // n (VNULL)
            case 110:
                if (nc)
                    array_add_boolean(__box, 0);
                nc = 0;
                break;
            // , (SEPARATOR)
            case 44: nc = 1; break;
                
            default: break;
        }
        
    } while (*__json->pointer != '\0');
}

void json_object(json_t *__json, box_t __box)
{
    box_t box;
    char *key = NULL;
    int isKey = 0;
    int level = __json->obj_lev;
    
    do
    {
        switch (*__json->pointer++)
        {
            case 123: // {
    
                if (__json->obj_lev++ == -1)
                {
                    level = __json->obj_lev;
                    break;
                }
                
                box = new_object();
                json_object(__json, box);
                object_put_box(__box,
                               MINIBOX_MEMORY_RELEASE, key,
                               MINIBOX_MEMORY_RELEASE, box);
                key = NULL;
                isKey = 0;
                break;
                
            case 125: // }
                
                if (level == __json->obj_lev--) return;
                break;
                
            // [ (ARRAY)
            case 91:
                
                __json->arr_lev++;
                box = new_array();
                json_array(__json, box);
                object_put_box(__box,
                               MINIBOX_MEMORY_RELEASE, key,
                               MINIBOX_MEMORY_RELEASE, box);
                key = NULL;
                isKey = 0;
                break;
                
            // " (STRING)
            case 34:
            {
                if (!isKey)
                    key = json_string(__json);
                else
                {
                    char *val = json_string(__json);
                    
                    object_put_string(__box,
                                      MINIBOX_MEMORY_RELEASE, key,
                                      MINIBOX_MEMORY_RELEASE, val);
                    
                    key = NULL;
                    isKey = 0;
                }
            }
                break;
            // : (ATTRIBUTE)
            case 58:
                
                isKey = 1;
                break;
                
            // 0<->9 (NUMBER)
            case 48: case 49: case 50: case 51: case 52:
            case 53: case 54: case 55: case 56: case 57:
            // - (NUMBER)
            case 45:
                
                if (isKey)
                {
                    double num = json_number(__json);
                    object_put_number(__box,
                                      MINIBOX_MEMORY_RELEASE, key,
                                      num);
                    key = NULL;
                    isKey = 0;
                }
                
                break;
            // t (BOOLEAN)
            case 116:
                
                if (isKey)
                {
                    long num = 1;
                    object_put_boolean(__box,
                                       MINIBOX_MEMORY_RELEASE, key,
                                       num);
                    key = NULL;
                    isKey = 0;
                }
                break;
            // f (BOOLEAN)
            case 102:
                
                if (isKey)
                {
                    long num = 0;
                    object_put_boolean(__box,
                                       MINIBOX_MEMORY_RELEASE, key,
                                       num);
                    key = NULL;
                    isKey = 0;
                }
                break;
            // n (VNULL)
            case 110:
                
                if (isKey)
                {
                    object_put_null(__box,
                                    MINIBOX_MEMORY_RELEASE, key);
                    key = NULL;
                    isKey = 0;
                }
                break;
                
            // '\t' '\n' ' ' ',' (USELESS)
            default: break;
        }
        
    } while (*__json->pointer != '\0');
}

void array_json(box_t __box, box_t __str, int *level);

static void json_put_value(box_t __str, long __type, void *__value, int *__level)
{
    switch (__type)
    {
        case MINIBOX_TYPE_STRING:
        case MINIBOX_TPAR_STRING:
            stream_add_between(__str, *((char **) __value), '"');
            break;
            
        case MINIBOX_TYPE_ARRAY:
        case MINIBOX_TPAR_ARRAY:
            array_json(*(box_t *)__value, __str, __level);
            break;
        
        case MINIBOX_TYPE_OBJECT:
        case MINIBOX_TPAR_OBJECT:
            object_json(*(box_t *)__value, __str, __level);
            break;
            
        default: box_put_value(__str, __type, __value, __level);
    }
}

void array_json(box_t __box, box_t __str, int *level)
{
    long i;
    
    stream_open_hierarchy(__str, '[', (*level += 1));
    
    for (i = 0; i < BOXS; i += V2S)
    {
        void *value = BOXB + i + MINIBOX_VALUE;
        long type  = *(long*) (BOXB + i + MINIBOX_TYPE );
        
        if (i != 0) stream_add(__str, ", ");
        json_put_value(__str, type, value, level);
    }
    
    stream_close_hierarchy(__str, ']', (*level -= 1));
}

void object_json(box_t __box, box_t __str, int *level)
{
    long i;
    
    stream_open_hierarchy(__str, '{', (*level += 1));
    
    for (i = 0; i < BOXS; i += V3S)
    {
        void *value = (BOXB + i + MINIBOX_VALUE);
        long type  = *(long *) (BOXB + i + MINIBOX_TYPE );
        char *key = *(char **) (BOXB + i + MINIBOX_KEY  );
    
        if (i != 0) stream_open_hierarchy(__str, ',', *level);
        
        stream_add_between(__str, key, '"');
        stream_add(__str, " : ");
        json_put_value(__str, type, value, level);
    }
    
    stream_close_hierarchy(__str, '}', (*level -= 1));
}
