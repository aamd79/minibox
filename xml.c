//
//  xml.c
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

#define XML_FORMAT_ERROR "Invalid xml format."
#define STRING_TRIM(s) while (*s > 0x0 && *s < 0x21) ++s
#define case_0(x) case 0x0: ERROR(XML_FORMAT_ERROR, x)
#define case_trim case 0x8: case 0x9: case 0xA: \
case 0xB: case 0xC: case 0xD: case 0x20:


enum {
    XML_INVALID,
    XML_CLOSE,
    XML_VALUE,
    XML_LABEL,
    XML_OBJECT,
    XML_ATTRIBUTE_OPEN,
    XML_ATTRIBUTE_CLOSE
    
};

typedef struct
{
    const char *src;
    const char *key;
    unsigned size;
    signed type;
} tok_t;

typedef struct
{
    const char *xml;
    const char *ptr;
    signed level;
    tok_t token;
} xml_t;

void object_xml(box_t __box, box_t __str, int *level);
void array_xml(box_t __box, box_t __str, int *level);

const char* xml_parse_start(box_t __box, const char *__src);
const char* xml_parse_header(const char *__src);
const char* xml_parse_doctype(const char *__src);

box_t xml_object_from_string(const char *__src)
{
    box_t box = new_object();
    const char *ptr = __src;
    
    do
    {
        switch (*ptr++)
        {
            case '<':
                switch (*ptr)
            {
                case '?': ptr = xml_parse_header(ptr); break;
                case '!': ptr = xml_parse_doctype(ptr); break;
                default : ptr = xml_parse_start(box, --ptr); break;
            }
                break;
            case_trim
                break;
                
            default:
                ERROR(XML_FORMAT_ERROR, "xml_object_from_string()")
                return 0;
        }
        
    } while (*ptr != 0);
    
    return box;
}

box_t xml_object_from_file(const char *__path)
{
    box_t str;
    
    if (!(str = stream_load(__path)))
    {
        ERROR(BOX_CREATE_ERROR, "xml_object_from_file()")
        return 0;
    }
    
    return xml_object_from_string(box_buffer(str));
}

box_t xml_stream_from_object(box_t __box)
{
    box_t str = new_stream();
    int level = 0;
    
    stream_add(str, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    stream_add(str, "<!-- XML document created with MiniBox API -->\n");
    stream_add(str, "<object>");
    
    object_xml(__box, str, &level);
    stream_add(str, "</object>");
    
    stream_finalize(str);
    
    return str;
}

int xml_file_from_object(box_t __box, const char *__path)
{
    box_t str = 0;
    
    EIF(!(str = xml_stream_from_object(__box)),
        XML_FORMAT_ERROR, "xml_file_from_object()")
    
    EIF(stream_save(str, __path),
        BOX_CREATE_ERROR, "xml_file_from_object()")
    
    free_box(str);
    
    return 0;
}

const char* xml_attr_end(const char *src, int *type, int *level)
{
    while (1)
    {
        switch (*src++)
        {
            
            case '<':
            case_0("xml_attr_end")
                return NULL;
                
            case '/':
                
                STRING_TRIM(src);
                
                if (*src == '>')
                {
                    *type = XML_ATTRIBUTE_CLOSE;
                    
                    return ++src;
                }
                break;
                
            case '>':
                
                *type = XML_ATTRIBUTE_OPEN;
                *level = (*level) + 1;
                
                return src;
                
            default: break;
        }
    }
}

static int xml_next(xml_t *__xml)
{
    const char *ptr = __xml->ptr;
    const char *src = NULL;
    unsigned size = 0;
    signed type = XML_INVALID;
    signed level = __xml->level;
    
    STRING_TRIM(ptr);
    
    do
    {
        switch (*ptr++)
        {
            case_0("xml_next()")
                return -1;
                
            case '\n': case '\t': case '\r':
                break;
                
            case '<':
                
                if (size)
                {
                    type = XML_VALUE;
                    src = __xml->ptr;
                    ptr--;
                } else if (*ptr == '/')
                {
                    type = XML_CLOSE;
                    level--;
                    src = NULL;
                    
                    EIF(!(ptr = strchr(ptr, '>')),
                        XML_FORMAT_ERROR, "xml_next()")
                    
                    ptr++;
                }
                else src = ptr;
                break;
                
            case '>':
                
                STRING_TRIM(ptr);
                
                if (*ptr == '<')
                    type = XML_OBJECT;
                else
                    type = XML_LABEL;
                
                level++;
                break;
                
            case ' ':
                
                if (src)
                    ptr = xml_attr_end(ptr, &type, &level);
                else
                    size++;
                break;
                
            default:
                size++;
                break;
        }
    } while (!type);
    
    __xml->token.src = src;
    __xml->token.size = size;
    __xml->token.type = type;
    __xml->ptr = ptr;
    __xml->level = level;
    
    return 0;
}

static int xml_is_array(xml_t *__xml)
{
    switch (__xml->token.type)
    {
        case XML_OBJECT:
        case XML_ATTRIBUTE_OPEN:
        case XML_ATTRIBUTE_CLOSE:
            break;
        default:
            WARNING(XML_FORMAT_ERROR, "xml_is_array()")
            return 0;
    }

    xml_t xml = *__xml;
    const char *key = NULL;
    signed level = xml.level;
    int levOk = 0;
    
    do
    {
        if (xml_next(&xml)) return -1;
        
        switch (xml.token.type)
        {
            case XML_ATTRIBUTE_CLOSE:
                levOk = xml.level == level;
                break;
            case XML_ATTRIBUTE_OPEN:
            case XML_OBJECT:
            case XML_LABEL:
                levOk = xml.level == level +1;
                break;
            default: levOk = 0; break;
        }
        
        if (levOk) {
            if (key)
                return !memcmp(key, xml.token.src, xml.token.size);
            
            __xml->token.key = key = xml.token.src;
        }
    } while (level <= xml.level);
    
    return 0;
}

static char* xml_copy_key(tok_t *__tkn)
{
    char *key;
    
    if (!(key = malloc(__tkn->size + 1)))
    {
        ERROR(BOX_MEMORY_ERROR, "xml_copy_key()")
        return NULL;
    }
    memcpy(key, __tkn->src, __tkn->size);
    key[__tkn->size] = 0;
    
    return key;
}

static char * xml_array_key(xml_t *__xml)
{
    char *key;
    tok_t *t = &__xml->token;
    const char *ptr = t->key;
    long l = 0, s = 0;
    
    while (*ptr != ' ' && *ptr != '>') ptr++;
    
    l = ptr - t->key;
    
    if ((*t->key + l) == 's')
        s = l + 1;
    else
        s = l + 2;
    
    if (!(key = malloc(s)))
    {
        ERROR(BOX_MEMORY_ERROR, "xml_array_key()")
        return NULL;
    }
    
    memcpy(key, t->key, l);
    
    key[s-2] = 's';
    key[s-1] = 0;
    
    return key;
}

static void xml_trim_value(tok_t *__tkn)
{
    if (!__tkn->size) return;
    unsigned s = __tkn->size;
    const char *b = __tkn->src;
    const char *e = b + s - 1;
    
    while (*b == ' ') { b++; s--; }
    while (*e == ' ') { e--; s--; }
    
    __tkn->src = b;
    __tkn->size = s;
}

static int xml_value_type(tok_t *__tkn)
{
    int i = 0, p = 0;
    
    xml_trim_value(__tkn);
    
    for (i = 0; i < __tkn->size; i++)
    {
        switch (*(__tkn->src + i))
        {
            case_0("xml_value_type")
                return -1;
                
            case 48: case 49: case 50: case 51: case 52: /* 0<->9 */
            case 53: case 54: case 55: case 56: case 57:
                break;
                
            case 45: /* - */
                
                if (!i) break;
                else return MINIBOX_TYPE_STRING;
                
            case 46: /* . */
                
                if (!p++ && i) break;
                else return MINIBOX_TYPE_STRING;
                break;
                
            default:
                
                if (i) return MINIBOX_TYPE_STRING;
                
                if (__tkn->size >= 5)
                {
                    if (!memcmp(__tkn->src, MINIBOX_VALUE_FALSE, 5))
                        return MINIBOX_FALSE;
                }
                else if (__tkn->size >= 4)
                {
                    if (!memcmp(__tkn->src, MINIBOX_VALUE_TRUE, 4))
                        return MINIBOX_TRUE;
                    
                    if (!memcmp(__tkn->src, MINIBOX_VALUE_NULL, 4))
                        return MINIBOX_TYPE_NULL;
                }
                
                return MINIBOX_TYPE_STRING;
        }
    }
    return MINIBOX_TYPE_NUMBER;
}

static int xml_attributes(box_t __box, xml_t *__xml)
{
    const char *src = NULL;
    const char  *ptr = __xml->token.src + __xml->token.size;
    
    while (*ptr++ == 0x20) /* espace */
    {
        char *key = NULL, *val = NULL;
        long len = 0;
        int ok = 0;
        
        do { switch (*ptr++)
            {
                
                case_0("xml_attributes()") /* end */
                    return - 1;
                case_trim
                    break;
                
                case 0x3D: /* = */
                    ok = 0x1;
                    break;
                    
                default :
                if (len++ == 0x0)
                    src = ptr -0x1;
                break;
            }
            
        } while (!ok);
        
        EIF(!(key = malloc(len + 1)),
            BOX_MEMORY_ERROR, "xml_attributes()")

        memcpy(key, src, len);
        
        len = ok = key[len] = 0;
        
        STRING_TRIM(ptr);
        
        do { switch (*ptr++) {
                
            case_0("xml_attributes()") /* end */
                return - 1;
                
            case 0x27: /* ' */
            case 0x22: /* " */
                ok++;
                break;
            default :
                if (ok && !len++)
                    src = ptr -1;
                break;
        } } while (ok != 2);
        
        EIF(!(val = malloc(len + 1)),
            XML_FORMAT_ERROR, "xml_attributes()")
   
        memcpy(val, src, len);
        val[len] = 0;
        
        object_put_string(__box,
                          MINIBOX_MEMORY_RELEASE, key,
                          MINIBOX_MEMORY_RELEASE, val);
    }

    return 0;
}

int xml_object(box_t __box, xml_t *__xml);

int xml_array(box_t __box, xml_t *__xml)
{
    box_t box;
    int level = __xml->level;
    
    do
    {
        if (xml_next(__xml)) return -1;
        
        switch (__xml->token.type)
        {
            case XML_VALUE:
                
                switch (xml_value_type(&__xml->token))
                {
                    case MINIBOX_TYPE_NUMBER: {
                        double value = atof(__xml->token.src);
                        array_add_number(__box, value);
                    }   break;
                    case MINIBOX_TYPE_STRING: {
                        char *str = xml_copy_key(&__xml->token);
                        array_add_string(__box, MINIBOX_MEMORY_RELEASE, str);
                    }   break;
                    case MINIBOX_TRUE:
                        array_add_boolean(__box, 1);
                        break;
                    case MINIBOX_FALSE:
                        array_add_boolean(__box, 0);
                        break;
                    case MINIBOX_TYPE_NULL:
                        array_add_null(__box);
                        break;
                    default: break;
                }
                break;
                
            case XML_ATTRIBUTE_OPEN:
                
                if (xml_is_array(__xml))
                {
                    box_t arr;
                    char *key;
                    EIF(!(key = xml_array_key(__xml)),
                        XML_FORMAT_ERROR, "xml_array")
                    EIF(!(box = new_object()),
                        BOX_CREATE_ERROR, "xml_array")
                    EIF(xml_attributes(box, __xml),
                        XML_FORMAT_ERROR, "xml_array")
                    EIF(!(arr = new_array()),
                        BOX_CREATE_ERROR, "xml_array")
                    EIF(xml_array(arr, __xml),
                        XML_FORMAT_ERROR, "xml_array")
                    
                    object_put_box(box,
                                   MINIBOX_MEMORY_RELEASE, key,
                                   MINIBOX_MEMORY_RELEASE, arr);
                } else {
                    
                    EIF(!(box = new_object()),
                        BOX_CREATE_ERROR, "xml_array")
                    EIF(xml_attributes(box, __xml),
                        XML_FORMAT_ERROR, "xml_array")
                    EIF(xml_object(box, __xml),
                        XML_FORMAT_ERROR, "xml_array")
                }
                array_add_box(__box,
                              MINIBOX_MEMORY_RELEASE, box);
                break;
                
            case XML_ATTRIBUTE_CLOSE:
                
                EIF(!(box = new_object()),
                    BOX_CREATE_ERROR, "xml_array")
                EIF(xml_attributes(box, __xml),
                    XML_FORMAT_ERROR, "xml_array")
                array_add_box(__box,
                              MINIBOX_MEMORY_RELEASE, box);
                break;
                
            case XML_OBJECT:
                
                if (xml_is_array(__xml))
                {
                    EIF(!(box = new_array()),
                        BOX_CREATE_ERROR, "xml_array")
                    EIF(xml_array(box, __xml),
                        XML_FORMAT_ERROR, "xml_array")
                } else {
                    EIF(!(box = new_object()),
                        BOX_CREATE_ERROR, "xml_array")
                    EIF(xml_object(box, __xml),
                        XML_FORMAT_ERROR, "xml_array")
                }
                array_add_box(__box,
                              MINIBOX_MEMORY_RELEASE, box);
                break;
                
            case XML_CLOSE:
            case XML_LABEL:
                break;
                
            default:
                ERROR(XML_FORMAT_ERROR, "xml_array")
                return -1;
        }
    } while (level <= __xml->level);

    return 0;
}

int xml_object(box_t __box, xml_t *__xml)
{
    int level = __xml->level;
    char *key = NULL;
    box_t box;
    
    do
    {
        if (xml_next(__xml)) return -1;
        
        switch (__xml->token.type)
        {
            case XML_VALUE:
                switch (xml_value_type(&__xml->token))
                {
                    case MINIBOX_TYPE_NUMBER: {
                        double value = atof(__xml->token.src);
                        object_put_number(__box,
                                          MINIBOX_MEMORY_RELEASE, key,
                                          value);
                    }   break;
                    case MINIBOX_TYPE_STRING: {
                        char *str = xml_copy_key(&__xml->token);
                        object_put_string(__box,
                                          MINIBOX_MEMORY_RELEASE, key,
                                          MINIBOX_MEMORY_RELEASE, str);
                    }   break;
                    case MINIBOX_TRUE:
                        object_put_boolean(__box,
                                           MINIBOX_MEMORY_RELEASE, key,
                                           1);
                        break;
                    case MINIBOX_FALSE:
                        object_put_boolean(__box,
                                           MINIBOX_MEMORY_RELEASE, key,
                                           0);
                        break;
                    case MINIBOX_TYPE_NULL:
                        object_put_null(__box,
                                        MINIBOX_MEMORY_RELEASE, key);
                        break;
                    default: break;
                }
                break;
                
            case XML_ATTRIBUTE_OPEN:
            
                EIF(!(key = xml_copy_key(&__xml->token)),
                    BOX_MEMORY_ERROR, "xml_object")
                
                if (xml_is_array(__xml))
                {
                    box_t arr;
                    char *ak;
                    EIF(!(ak = xml_array_key(__xml)),
                        XML_FORMAT_ERROR, "xml_object")
                    EIF(!(box = new_object()),
                        BOX_CREATE_ERROR, "xml_object")
                    EIF(xml_attributes(box, __xml),
                        XML_FORMAT_ERROR, "xml_object")
                    EIF(!(arr = new_array()),
                        BOX_CREATE_ERROR, "xml_object")
                    EIF(xml_array(arr, __xml),
                        XML_FORMAT_ERROR, "xml_object")
                    
                    object_put_box(box,
                                   MINIBOX_MEMORY_RELEASE, ak,
                                   MINIBOX_MEMORY_RELEASE, arr);
                    
                } else {
                    
                    EIF(!(box = new_object()),
                        BOX_CREATE_ERROR, "xml_object")
                    EIF(xml_attributes(box, __xml),
                        XML_FORMAT_ERROR, "xml_object")
                    EIF(xml_object(box, __xml),
                        XML_FORMAT_ERROR, "xml_object")
                }
                object_put_box(__box,
                               MINIBOX_MEMORY_RELEASE, key,
                               MINIBOX_MEMORY_RELEASE, box);
                break;
                
            case XML_ATTRIBUTE_CLOSE:
                
                EIF(!(key = xml_copy_key(&__xml->token)),
                    BOX_MEMORY_ERROR, "xml_object")
                EIF(!(box = new_object()),
                    BOX_CREATE_ERROR, "xml_object")
                EIF(xml_attributes(box, __xml),
                    XML_FORMAT_ERROR, "xml_object")
                
                object_put_box(__box,
                               MINIBOX_MEMORY_RELEASE, key,
                               MINIBOX_MEMORY_RELEASE, box);
                break;
                
            case XML_OBJECT:
            
                EIF(!(key = xml_copy_key(&__xml->token)),
                    BOX_MEMORY_ERROR, "xml_object")
                
                if (xml_is_array(__xml))
                {
                    EIF(!(box = new_array()),
                        BOX_CREATE_ERROR, "xml_object")
                    EIF(xml_array(box, __xml),
                        XML_FORMAT_ERROR, "xml_object")
                    
                } else {
                    EIF(!(box = new_object()),
                        BOX_CREATE_ERROR, "xml_object")
                    EIF(xml_object(box, __xml),
                        XML_FORMAT_ERROR, "xml_object")
                }
                object_put_box(__box,
                               MINIBOX_MEMORY_RELEASE, key,
                               MINIBOX_MEMORY_RELEASE, box);
                break;
                
            case XML_LABEL:
                
                key = xml_copy_key(&__xml->token);
                break;
                
            case XML_CLOSE:
                
                key = NULL;
                break;
                
            default:
                
                ERROR(XML_FORMAT_ERROR, "xml_object")
                return -1;
        }
    } while (level <= __xml->level);
    
    return 0;
}

const char* xml_parse_start(box_t __box, const char *__src)
{
    xml_t xml = { __src, __src, 0, { 0, 0, 0, 0 } };
    
    if (xml_next(&xml)) return NULL;
    
    if (xml.token.type == XML_ATTRIBUTE_OPEN)
        if (xml_attributes(__box, &xml)) return NULL;
    
    if (xml_is_array(&xml)) {
        char *key = xml_array_key(&xml);
        box_t arr = new_array();
        if (xml_array(arr, &xml)) return NULL;
        object_put_box(__box,
                       MINIBOX_MEMORY_RELEASE, key,
                       MINIBOX_MEMORY_RELEASE, arr);
    } else {
        if (xml_object(__box, &xml)) return NULL;
    }
    
    return xml.ptr;
}

const char* xml_parse_header(const char *__src)
{
    __src = strchr(++__src, '>');
    if (__src) __src++;
    return __src;
}

const char* xml_parse_doctype(const char *__src)
{
    
    return strchr(__src, '>') + 1;
}

static void xml_put_value(box_t __str, long __type, const void *__value, int *__level)
{
    switch (__type) {
        case MINIBOX_TYPE_STRING:
        case MINIBOX_TPAR_STRING:
            stream_add(__str, *((char **) __value));
            break;
            
        case MINIBOX_TYPE_ARRAY:
        case MINIBOX_TPAR_ARRAY:
            array_xml(*(box_t *)__value, __str, __level);
            break;
            
        case MINIBOX_TYPE_OBJECT:
        case MINIBOX_TPAR_OBJECT:
            object_xml(*(box_t *)__value, __str, __level);
            break;
            
        default: box_put_value(__str, __type, __value, __level);
    }
}

void array_xml(box_t __box, box_t __str, int *__level)
{
    long i;
    *__level += 1;
    
    for (i = 0; i < BOXS; i += V2S)
    {
        void *value = BOXB + i + MINIBOX_VALUE;
        long type  = *(long*) (BOXB + i + MINIBOX_TYPE);
        stream_paragraph(__str, *__level);
        stream_add(__str, "<item>");
        xml_put_value(__str, type, value, __level);
        stream_add(__str, "</item>");
    }
    
    *__level -= 1;
    stream_paragraph(__str, *__level);
}

void object_xml(box_t __box, box_t __str, int *__level)
{
    long i, s = BOXS;
    *__level += 1;
    
    for (i = 0; i < s; i += V3S)
    {
        void *value = (BOXB + i + MINIBOX_VALUE);
        long type  = *(long *) (BOXB + i + MINIBOX_TYPE );
        char *key = *(char **) (BOXB + i + MINIBOX_KEY  );
        
        stream_paragraph(__str, *__level);
        stream_add_char(__str, '<');
        stream_add(__str, key);
        stream_add_char(__str, '>');
        xml_put_value(__str, type, value, __level);
        stream_add(__str, "</");
        stream_add(__str, key);
        stream_add_char(__str, '>');
    }
    *__level -= 1;
    stream_paragraph(__str, *__level);
}


