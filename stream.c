//
//  stream.c
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

box_t new_stream(void)
{
    return box_create(MINIBOX_TYPE_STREAM);
}

void stream_add(box_t __box, const char *__value)
{
    long len = strlen(__value);
    if (box_reallocated(__box, len)) return;
    
    void *dst = BOXB + BOXS - len;
    
    memcpy(dst, __value, len);
}

void stream_add_char(box_t __box, char __value)
{
    if (box_reallocated(__box, 1)) return;
    
    void *dst = BOXB + BOXS - 1;
    
    memcpy(dst, &__value, 1);
}

char* stream_get(box_t __box)
{
    return BOXB;
}

void stream_add_number(box_t __box, double __value)
{
    stream_add(__box, box_number_string(__value));
}

void stream_open_hierarchy(box_t __box, char __char, int __level)
{
    long size = 2 + __level;
    
    if (box_reallocated(__box, size)) return;
    
    void *dst = BOXB + BOXS - size;
    
    memcpy(dst, &__char, 1);
    memcpy(dst + 1, "\n", 1);
    memset(dst + 2, '\t', __level);
}

void stream_paragraph(box_t __box, int __level)
{
    long size = 1 + __level;
    
    if (box_reallocated(__box, size)) return;
    
    void *dst = BOXB + BOXS - size;
    
    memcpy(dst, "\n", 1);
    memset(dst + 1, '\t', __level);
}

void stream_close_hierarchy(box_t __box, char __char, int __level)
{
    long size = 2 + __level;
    
    if (box_reallocated(__box, size)) return;
    
    void *dst = BOXB + BOXS - size;
    
    memcpy(dst, "\n", 1);
    memset(dst + 1, '\t', __level);
    memcpy(dst + __level + 1, &__char, 1);
}

void stream_add_between(box_t __box, const char *__str, char __char)
{
    long len = strlen(__str);
    long size = 2 + len;
    
    if (box_reallocated(__box, size)) return;
    
    void *dst = BOXB + BOXS - size;
    
    memcpy(dst, &__char, 1);
    memcpy(dst + 1, __str, len);
    memcpy(dst + 1 + len, &__char, 1);
}

void stream_finalize(box_t __box)
{
    if (box_reallocated(__box, 1)) return;
    
    ((char *)BOXB)[BOXS - 1] = 0;
    
    box_finalize(__box);
}

int stream_save(box_t __box, const char *__path)
{

    FILE *file;
    
    if (!(file = fopen(__path, "w")))
        return -1;
    
    long len = BOXS;
    const char *str = BOXB;
    if (fwrite(str, 1, len, file) != len)
    {
        fclose(file);
        return -1;
    }
    
    fclose(file);
    return 0;
}

box_t stream_load(const char *__path)
{
    FILE *file;
    box_t __box;
    fpos_t fs;
    
    if (!(file = fopen(__path, "r")))
        return 0;
    
    if (fseek(file, 0, SEEK_END))
        goto FILE;
        
    fgetpos(file, &fs);
    
    rewind(file);
    
    if(!(__box = new_stream()))
        goto FILE;
   
    if (box_allocated(__box, fs + 1))
        goto OBJ;
    
    if (fread(BOXB, 1, fs, file) != fs)
        goto OBJ;
    
    ((char *)BOXB)[fs] = '\0';
    
    fclose(file);
    
    return __box;
   
OBJ: free_box(__box);
FILE: fclose(file);
    return 0;
}

void stream_print(box_t __box)
{
    printf("%s\n", BOXB);
}
