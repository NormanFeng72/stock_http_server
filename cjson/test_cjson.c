/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include "iconv.h"
#include "errno.h"

/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

/* cJSON */
/* JSON parser in C. */

/* disable warnings about old C89 functions in MSVC */
#if !defined(_CRT_SECURE_NO_DEPRECATE) && defined(_MSC_VER)
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifdef __GNUC__
#pragma GCC visibility push(default)
#endif
#if defined(_MSC_VER)
#pragma warning(push)
/* disable warning about single line comments in system headers */
#pragma warning(disable : 4001)
#endif

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <float.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include "tests/common.h"

#include "cJSON.h"

/* define our own boolean type */
#define true ((cJSON_bool)1)
#define false ((cJSON_bool)0)

typedef struct internal_hooks {
    void *(*allocate)(size_t size);
    void (*deallocate)(void *pointer);
    void *(*reallocate)(void *pointer, size_t size);
} internal_hooks;
#if defined(_MSC_VER)
/* work around MSVC error C2322: '...' address of dillimport '...' is not static */
static void *internal_malloc(size_t size) {
    return malloc(size);
}
static void internal_free(void *pointer) {
    free(pointer);
}
static void *internal_realloc(void *pointer, size_t size) {
    return realloc(pointer, size);
}
#else
#define internal_malloc malloc
#define internal_free free
#define internal_realloc realloc
#endif
static internal_hooks global_hooks = {internal_malloc, internal_free, internal_realloc};
// void reset(cJSON *item);
void reset(cJSON *item) {

    // if ((item->valuestring != NULL) && !(item->type & cJSON_IsReference))
    { global_hooks.deallocate(item->valuestring); }
    // if ((item->string != NULL) && !(item->type & cJSON_StringIsConst))
    { global_hooks.deallocate(item->string); }
    if ((item != NULL) && (item->child != NULL)) {
        cJSON_Delete(item->child);
    }

    memset(item, 0, sizeof(cJSON));
}

int strConvEncoding(char *inBuf, char *outBuf, char *from_code, char *to_code) {
    size_t readBytes = strlen(inBuf);
    size_t writeBytes = strlen(inBuf) * 2 + 1;
    printf("inbuff_size=[%d] outbuf_size=[%d]\n", readBytes, writeBytes);
    char *in = inBuf;
    char *out = outBuf;
    iconv_t convert = iconv_open(from_code, to_code);
    if (iconv(convert, &in, &readBytes, &out, &writeBytes) < 0) {
        return (-1);
    }
    iconv_close(convert);
    printf("[%s] [%s]\n", inBuf, outBuf);
    return (out - outBuf);
}

void *defMallocMethod(size_t sz) {
    // printf("defined malloc function is invoked.\n");
    return malloc(sz);
}

void defFreeMethod(void *ptr) {
    // printf("defined free function is invoked.\n");
    return free(ptr);
}

test_cJSON_CreateStringReference(char *content) {
    printf("--------------test_cJSON_CreateReference\n");
    cJSON *item = NULL;
    char *printJson = NULL;
    char *str;
    str = malloc(256);
    memset(str, 0, 256);
    strcpy(str, "this is a reference to string!!!");
    item = cJSON_CreateStringReference(str);
    printJson = cJSON_Print(item);
    printf("-\tpased json:%s\n", printJson);
    if (item != NULL) {
        cJSON_Delete(item);
    }
    if (printJson != NULL) {
        free(printJson);
    }
    free(str);
}

void test_cJSON_Hooks(char *content) {
    printf("--------------test_cJSON_Hooks\n");
    cJSON_Hooks *myHookMethods = (cJSON_Hooks *)malloc(sizeof(cJSON_Hooks));
    myHookMethods->malloc_fn = defMallocMethod;
    myHookMethods->free_fn = defFreeMethod;
    cJSON_InitHooks(myHookMethods);
    cJSON *dummyJSON = cJSON_Parse(content);
    cJSON_Delete(dummyJSON);
    cJSON_InitHooks(NULL);
    myHookMethods->malloc_fn = malloc;
    myHookMethods->free_fn = free;
    cJSON_InitHooks(myHookMethods);
    dummyJSON = cJSON_Parse(content);
    cJSON_Delete(dummyJSON);
    if (myHookMethods != NULL) {
        free(myHookMethods);
    }
}

void test_cJSON_Parse(char *content) {
    printf("--------------test_cJSON_Parse\n");
    cJSON *parsed = NULL;
    char *printJson = NULL;
    // cJSON_ParseWithLength
    // parsed = cJSON_ParseWithLength(content, strlen(content));
    // printJson = cJSON_Print(parsed);
    // // printf("pased json: \n %s\n", printJson);
    // if (parsed != NULL)
    // {
    //     cJSON_Delete(parsed);
    // }
    // if (printJson != NULL)
    // {
    //     free(printJson);
    // }
    // cJSON_Parse
    parsed = cJSON_Parse(content);
    printJson = cJSON_Print(parsed);
    // printf("pased json: \n %s\n", printJson);
    if (parsed != NULL) {
        cJSON_Delete(parsed);
    }
    if (printJson != NULL) {
        free(printJson);
    }
    char *buf_string = NULL;
    buf_string = malloc(1024);
    sprintf(buf_string, "[{\"%d\":\t%c }]\n} \n]", *content, *content + 1);
    cJSON *name = NULL;
    cJSON *test_json = cJSON_Parse(buf_string);
    if (test_json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        free(buf_string);
        return 0;
    }
    name = cJSON_GetObjectItemCaseSensitive(test_json, "name");
    free(buf_string);
    // if (name != NULL)
    // {
    //     cJSON_Delete(name);
    // }
    if (test_json != NULL) {
        cJSON_Delete(test_json);
    }
}

void parseWithOpt(char *data, cJSON_bool require_null_terminated) {
    cJSON *parsed = NULL;
    char *printJson = NULL;
    char **retrun_pase_end = NULL;

    printJson = cJSON_Print(parsed);
    parsed = cJSON_ParseWithOpts(data, retrun_pase_end, require_null_terminated);
    if (parsed == NULL) {
        printf("test_cJSON_ParseWithOpts error: %s\n", cJSON_GetErrorPtr());
        return;
    }
    // printf("pased json: \n %s\n", printJson);
    cJSON_Delete(parsed);
    free(printJson);
}

void test_cJSON_ParseWithOpts(char *content) {
    printf("--------------test_cJSON_ParseWithOpts\n");

    parseWithOpt("\xEF\xBB\xBF{}", true);
    parseWithOpt("\xEF\xBB\xBF{}", false);
    parseWithOpt("", true);
    parseWithOpt("", false);
    parseWithOpt("{}", true);
    parseWithOpt("{}", false);
    parseWithOpt("\r\n{}", true);
    parseWithOpt("\r\n{}", false);

    char *str_encoded = malloc(strlen(content) * 4 + 1);
    char *str_buff = malloc(strlen(content) * 4 + 256);
    memset(str_encoded, 0, strlen(content) * 4 + 1);
    memset(str_buff, 0, strlen(content) * 4 + 256);

    parseWithOpt(content, true);
    parseWithOpt(content, false);

    strConvEncoding(content, str_encoded, "UTF-8", "UTF-16LE");

    sprintf(str_buff, "{\"name\": \"%s\"}", str_encoded);
    parseWithOpt(str_buff, true);
    parseWithOpt(str_buff, false);
    parseWithOpt(str_encoded, true);
    parseWithOpt(str_encoded, false);

    strConvEncoding(content, str_encoded, "UTF-8", "ASCII");
    sprintf(str_buff, "{\"name\": \"%s\"}", str_encoded);
    parseWithOpt(str_buff, true);
    parseWithOpt(str_buff, false);
    parseWithOpt(str_encoded, true);
    parseWithOpt(str_encoded, false);

    strConvEncoding(content, str_encoded, "UTF-8", "UTF-16");
    sprintf(str_buff, "{\"name\": \"%s\"}", str_encoded);
    parseWithOpt(str_buff, true);
    parseWithOpt(str_buff, false);
    parseWithOpt(str_encoded, true);
    parseWithOpt(str_encoded, false);

    strConvEncoding(content, str_encoded, "UTF-8", "Unicode");
    sprintf(str_buff, "{\"name\": \"%s\"}", str_encoded);
    parseWithOpt(str_buff, true);
    parseWithOpt(str_buff, false);
    parseWithOpt(str_encoded, true);
    parseWithOpt(str_encoded, false);

    strConvEncoding(content, str_encoded, "UTF-8", "GB2312");
    sprintf(str_buff, "{\"name\": \"%s\"}", str_encoded);
    parseWithOpt(str_buff, true);
    parseWithOpt(str_buff, false);
    parseWithOpt(str_encoded, true);
    parseWithOpt(str_encoded, false);

    strConvEncoding(content, str_encoded, "UTF-8", "GBK");
    sprintf(str_buff, "{\"name\": \"%s\"}", str_encoded);
    parseWithOpt(str_buff, true);
    parseWithOpt(str_buff, false);
    parseWithOpt(str_encoded, true);
    parseWithOpt(str_encoded, false);

    strConvEncoding(content, str_encoded, "UTF-8", "GB18030");
    sprintf(str_buff, "{\"name\": \"%s\"}", str_encoded);
    parseWithOpt(str_buff, true);
    parseWithOpt(str_buff, false);
    parseWithOpt(str_encoded, true);
    parseWithOpt(str_encoded, false);

    strConvEncoding(content, str_encoded, "UTF-8", "UTF-32");
    sprintf(str_buff, "{\"name\": \"%s\"}", str_encoded);
    parseWithOpt(str_buff, true);
    parseWithOpt(str_buff, false);
    parseWithOpt(str_encoded, true);
    parseWithOpt(str_encoded, false);

    sprintf(str_buff, "%s", "\\u34\\u123\\u34\\u110\\u97\\u109\\u101\\u34\\u58\\u32\\u34\\u97\\u98\\u99\\u34\\u125\\u34");
    parseWithOpt(str_buff, true);
    parseWithOpt(str_buff, false);

    parseWithOpt("\x22\x7b\x22\x6e\x61\x6d\x65\x22\x3a\x20\x22\x61\x62\x63\x22\x7d\x22", true);
    parseWithOpt("\x22\x7b\x22\x6e\x61\x6d\x65\x22\x3a\x20\x22\x61\x62\x63\x22\x7d\x22", false);

    parseWithOpt("\\u\xD800{\"name\": \" unicode \"}", true);
    parseWithOpt("\\u\xD800{\"name\": \" unicode \"}", false);

    free(str_encoded);
    free(str_buff);
}

void test_cJSON_ParseWithLengthOpts(char *content) {
    printf("--------------test_cJSON_ParseWithLengthOpts\n");
    cJSON *parsed = NULL;
    char *printJson = NULL;
    char **retrun_pase_end = NULL;
    cJSON_bool require_null_terminated = (strlen(content) % 2 == 0) ? true : false;
    parsed = cJSON_ParseWithLengthOpts(content, strlen(content), retrun_pase_end, require_null_terminated);
    if (parsed == NULL) {
        printf("test_cJSON_ParseWithLengthOpts error: %s\n", cJSON_GetErrorPtr());
        return;
    }
    printJson = cJSON_Print(parsed);
    // printf("pased json: \n %s\n", printJson);
    if (parsed != NULL) {
        cJSON_Delete(parsed);
    }
    if (printJson != NULL) {
        free(printJson);
    }
}

void test_cJSON_Print(char *content) {
    printf("--------------test_cJSON_Print\n");
    // cJSON_PrintUnformatted
    // cJSON_PrintBuffered
    // cJSON_PrintPreallocated

    cJSON *parsed = NULL;
    char *printJson = NULL;
    char *printBuffered = NULL;
    char *printUnformatted = NULL;
    char *buffer = NULL;
    int len = 1;
    cJSON_bool fmt = (strlen(content) % 2 == 0) ? true : false;

    parsed = cJSON_Parse(content);
    if (parsed != NULL) {
        printJson = cJSON_Print(parsed);
        // printf("printJson: \n %s\n", printJson);
        len = strlen(printJson) + 5;
        buffer = (char *)malloc(len);
        printBuffered = cJSON_PrintBuffered(parsed, len, fmt);
        // printf("printBuffered: \n %s\n", printBuffered);
        printUnformatted = cJSON_PrintUnformatted(parsed);
        // printf("printUnformatted: \n %s\n", printUnformatted);
        cJSON_PrintPreallocated(parsed, buffer, len, fmt);
        // printf("printPreallocated: \n %s\n", buffer);
    }

    if (parsed != NULL) {
        cJSON_Delete(parsed);
    }
    if (printJson != NULL) {
        free(printJson);
    }
    if (printBuffered != NULL) {
        free(printBuffered);
    }
    if (printUnformatted != NULL) {
        free(printUnformatted);
    }
    if (buffer != NULL) {
        free(buffer);
    }
}

// cJSON *test_cJSON_Array(char *content)
// {
//     cJSON *root = NULL;
//     cJSON *fld = NULL;
//     char *printJson = NULL;

//     printf("--------------test_cJSON_Array\n");
//     /* Our array of "records": */
//     root = cJSON_CreateArray();

//     for (int i = 0; i + 2 < strlen(content);)
//     {
//         cJSON_AddItemToArray(root, fld = cJSON_CreateObject());
//         cJSON_AddNumberToObject(fld, "width", (*content + i) + 1); //+1 to avoid 0 value
//         i++;
//         cJSON_AddNumberToObject(fld, "height", (*content + i) + 1);
//         i++;
//     }
//     printJson = cJSON_Print(root);
//     cJSON_AddItemToArray(root, cJSON_CreateRaw("{This is the arrary end!!!}"));
//     cJSON_AddItemToArray(root, cJSON_CreateString(printJson));
//     cJSON_AddItemToArray(root, cJSON_CreateNull());
//     cJSON_AddItemToArray(root, cJSON_CreateTrue());
//     cJSON_AddItemToArray(root, cJSON_CreateFalse());
//     cJSON_AddItemToArray(root, cJSON_CreateBool(1));
//     cJSON_AddItemToArray(root, cJSON_CreateBool(0));

//     if (printJson != NULL)
//     {
//         free(printJson);
//     }
//     printJson = cJSON_Print(root);
//     // printf("pased json: \n %s\n", printJson);
//     if (printJson != NULL)
//     {
//         free(printJson);
//     }
//     if (cJSON_IsArray(root))
//     {
//         // printf("array size = [%d]\n", cJSON_GetArraySize(root));
//         for (int i = 0; i < cJSON_GetArraySize(root); i++)
//         {
//             printJson = cJSON_Print(cJSON_GetArrayItem(root, i));
//             // printf("pased json: \n %s\n", printJson);
//             if (printJson != NULL)
//             {
//                 free(printJson);
//             }
//         }
//     }
//     return root;
// }

cJSON *test_cJSON_Array(char *content) {
    cJSON *root = NULL;
    cJSON *fld = NULL;
    char *printJson = NULL;

    printf("--------------test_cJSON_Array\n");
    /* Our array of "records": */
    root = cJSON_CreateArray();

    for (int i = 0; i + 2 < strlen(content);) {
        cJSON_AddItemToArray(root, fld = cJSON_CreateObject());
        cJSON_AddNumberToObject(fld, "width", (*content + i) + 1); //+1 to avoid 0 value
        i++;
        cJSON_AddNumberToObject(fld, "height", (*content + i) + 1);
        i++;
    }
    printJson = cJSON_Print(root);
    cJSON_AddItemToArray(root, cJSON_CreateRaw("{This is the arrary end!!!}"));
    cJSON_AddItemToArray(root, cJSON_CreateString(printJson));
    cJSON_AddItemToArray(root, cJSON_CreateNull());
    cJSON_AddItemToArray(root, cJSON_CreateTrue());
    cJSON_AddItemToArray(root, cJSON_CreateFalse());
    cJSON_AddItemToArray(root, cJSON_CreateBool(1));
    cJSON_AddItemToArray(root, cJSON_CreateBool(0));
    if (strlen(content) > 128) {
        cJSON_AddItemToArray(root, cJSON_CreateRaw(""));
        cJSON_AddItemToArray(root, cJSON_CreateRaw(NULL));
        // cJSON_AddItemToArray(root, fld = cJSON_CreateRaw("{raw data!!!}"));
        // fld->valuestring = NULL;
        cJSON_AddItemToArray(root, cJSON_CreateNull());
    }

    if (printJson != NULL) {
        free(printJson);
    }
    printJson = cJSON_Print(root);
    // printf("pased json: \n %s\n", printJson);
    if (printJson != NULL) {
        free(printJson);
    }
    if (cJSON_IsArray(root)) {
        // printf("array size = [%d]\n", cJSON_GetArraySize(root));
        for (int i = 0; i < cJSON_GetArraySize(root); i++) {
            printJson = cJSON_Print(cJSON_GetArrayItem(root, i));
            // printf("pased json: \n %s\n", printJson);
            if (printJson != NULL) {
                free(printJson);
            }
        }
    }
    return root;
}

// cJSON *test_cJSON_Array(char *content)
// {
//     cJSON *root = NULL;
//     cJSON *fld = NULL;
//     cJSON *item = NULL;
//     char *printJson = NULL;

//     printf("--------------test_cJSON_Array\n");
//     /* Our array of "records": */
//     root = cJSON_CreateArray();

//     for (int i = 0; i + 2 < strlen(content);)
//     {
//         cJSON_AddItemToArray(root, fld = cJSON_CreateObject());
//         cJSON_AddNumberToObject(fld, "width", (*content + i) + 1); //+1 to avoid 0 value
//         i++;
//         cJSON_AddNumberToObject(fld, "height", (*content + i) + 1);
//         i++;
//     }
//     printJson = cJSON_Print(root);

//     cJSON_AddItemToArray(root, cJSON_CreateRaw("{This is the arrary end!!!}"));
//     cJSON_AddItemToArray(root, cJSON_CreateString(printJson));
//     cJSON_AddItemToArray(root, cJSON_CreateNull());
//     cJSON_AddItemToArray(root, cJSON_CreateTrue());
//     cJSON_AddItemToArray(root, cJSON_CreateFalse());
//     cJSON_AddItemToArray(root, cJSON_CreateBool(1));
//     cJSON_AddItemToArray(root, cJSON_CreateBool(0));
//     if (printJson != NULL)
//     {
//         free(printJson);
//     }
//     if (strlen(content) > 100)
//     {
//         // cJSON_AddItemToArray(root, cJSON_CreateRaw(""));
//         // cJSON_AddItemToArray(root, cJSON_CreateRaw(NULL));
//         // cJSON_AddItemToArray(root, fld = cJSON_CreateRaw("{raw data!!!}"));
//         // fld->valuestring = NULL;

//         // item = cJSON_CreateRaw("{raw data!!!}");
//         // item->valuestring = NULL;
//         // printJson = cJSON_Print(root);
//         // // printf("pased json: \n %s\n", printJson);
//         // if (printJson != NULL)
//         // {
//         //     free(printJson);
//         // }
//         // item->valuestring = "test";
//     }

//     printJson = cJSON_Print(root);
//     // printf("pased json: \n %s\n", printJson);
//     if (printJson != NULL)
//     {
//         free(printJson);
//     }
//     if (cJSON_IsArray(root))
//     {
//         // printf("array size = [%d]\n", cJSON_GetArraySize(root));
//         for (int i = 0; i < cJSON_GetArraySize(root); i++)
//         {
//             printJson = cJSON_Print(cJSON_GetArrayItem(root, i));
//             // printf("pased json: \n %s\n", printJson);
//             if (printJson != NULL)
//             {
//                 free(printJson);
//             }
//         }
//     }
//     // if (item != NULL)
//     // {
//     //     // cJSON_Delete(item);
//     // }
//     // return root;
// }

void test_cJSON_ArrayForEach(cJSON *array) {
    cJSON *element = NULL;
    int width = 0, height = 0;
    char *printJson = NULL;
    printf("--------------test_cJSON_ArrayForEach\n");
    // if (!cJSON_IsArray(array) || cJSON_IsInvalid(array) || cJSON_GetArraySize(array) == 0)
    // {
    //     return;
    // }
    int size = cJSON_GetArraySize(array);
    int ints[size];
    float floats[size];
    double doubles[size];
    char *str_arrary[size];
    cJSON *height_json;
    cJSON *width_json;
    char str[256] = {0};

    cJSON_IsInvalid(array);
    cJSON_ArrayForEach(element, array) {
        if (cJSON_IsObject(element)) {
            if (cJSON_HasObjectItem(element, "width")) {
                width_json = cJSON_GetObjectItem(element, "width");
            }
            if (cJSON_HasObjectItem(element, "height")) {
                height_json = cJSON_GetObjectItemCaseSensitive(element, "height");
            }
            if (cJSON_IsNumber(width_json) && cJSON_IsNumber(height_json)) {
                width = width_json->valueint;
                height = height_json->valueint;
                // printf("width=[%d], height=[%d]\n", width_json->valueint, height_json->valueint);
            }
        }
    }
    for (int i = 0; i < cJSON_GetArraySize(array); i++) {
        str_arrary[i] = malloc(128);
        memset(str_arrary[i], 0, 128);
        strcpy(str_arrary[i], "");
        // if (!cJSON_IsInvalid(cJSON_GetArrayItem(array, i)))
        {
            cJSON *width_json = cJSON_GetObjectItem(cJSON_GetArrayItem(array, i), "width");
            cJSON *height_json = cJSON_GetObjectItem(cJSON_GetArrayItem(array, i), "height");
            if (cJSON_IsNumber(width_json) && cJSON_IsNumber(height_json)) {
                ints[i] = width_json->valueint;
                if (cJSON_GetNumberValue(width_json) != NAN) {
                    printf("- \tWidth=[%d]\n", cJSON_GetNumberValue(width_json));
                }
                if (height_json->valueint == 0) {
                    floats[i] = 1;
                }
                else {
                    floats[i] = (float)width_json->valueint / (height_json->valueint + 1);
                }
                if (width_json->valueint == 0) {
                    doubles[i] = 1;
                }
                else {
                    doubles[i] = (double)height_json->valueint / (width_json->valueint + 1);
                }
                sprintf(str, "%d", ints[i]);
                strcpy(str_arrary[i], str);

                // printf("- \tArray Size=[%d], i=[%d], ints[i]=%d, floats[i]=%f, doubles[i]=%f\n", size, i, ints[i], floats[i], doubles[i]);
            }
            if (cJSON_IsRaw(cJSON_GetArrayItem(array, i))) {
                printf("- \tArray Size=[%d], i=[%d], is raw data=%s\n", size, i, cJSON_GetArrayItem(array, i)->valuestring);
            }
            if (cJSON_IsString(cJSON_GetArrayItem(array, i))) {
                printf("- \tArray Size=[%d], i=[%d], is string\n", size, i);
                if (cJSON_GetStringValue(cJSON_GetArrayItem(array, i)) != NULL) {
                    printf("- \tArray Size=[%d], i=[%d], is string=%s\n", size, i, cJSON_GetStringValue(cJSON_GetArrayItem(array, i)));
                }
            }
            if (cJSON_IsNull(cJSON_GetArrayItem(array, i))) {
                printf("- \tArray Size=[%d], i=[%d], is NULL\n", size, i);
                // printf("- \tArray Size=[%d], i=[%d], is NULL=%s\n", size, i, cJSON_GetStringValue(cJSON_GetArrayItem(array, i)));
            }

            if (cJSON_IsBool(cJSON_GetArrayItem(array, i))) {

                printf("- \tArray Size=[%d], i=[%d], is Bool\n", size, i);
                if (cJSON_IsFalse(cJSON_GetArrayItem(array, i))) {
                    printf("- \t\tArray Size=[%d], i=[%d], is False\n", size, i);
                }
                if (cJSON_IsTrue(cJSON_GetArrayItem(array, i))) {
                    printf("- \t\tArray Size=[%d], i=[%d], is True\n", size, i);
                }
            }
        }
    }
    cJSON *int_array = NULL;
    cJSON *float_array = NULL;
    cJSON *double_array = NULL;
    cJSON *string_array = NULL;

    // printf("-\tstrings arrary size=%d\n", sizeof(str_arrary) / sizeof(char *));
    // for (int i = 0; i < sizeof(str_arrary) / sizeof(char *); i++)
    // {
    //     printf("-\tstrings[%d]=%s\n", i, str_arrary[i]);
    // }
    for (int i = 0; i < size; i++) {
        // printf("-\tsize=%d, ints[%d] = %d\n", size, i, ints[i]);
    }
    size = sizeof(ints) / sizeof(ints[0]);
    for (int i = 0; i < size; i++) {
        // printf("-\tsize=%d, ints[%d] = %d\n", size, i, ints[i]);
        if (ints[i] < 0) ints[i] = 0;
    }
    int_array = cJSON_CreateIntArray(ints, size);
    size = sizeof(floats) / sizeof(floats[0]);
    for (int i = 0; i < size; i++) {
        // printf("-\tsize=%d, floats[%d] = %f\n", size, i, floats[i]);
        if (floats[i] < 0 || isnan(floats[i]) || floats[i] >= FLT_MAX) floats[i] = 0;
    }
    float_array = cJSON_CreateFloatArray(floats, size);
    size = sizeof(doubles) / sizeof(doubles[0]);
    for (int i = 0; i < size; i++) {
        printf("-\tsize=%d, doubles[%d] = %f\n", size, i, doubles[i]);
        if (doubles[i] < 0 || isnan(doubles[i]) || doubles[i] >= DBL_MAX) doubles[i] = 0;
    }
    double_array = cJSON_CreateDoubleArray(doubles, size);
    size = sizeof(str_arrary) / sizeof(str_arrary[0]);
    for (int i = 0; i < size; i++) {
        // printf("-\tsize=%d, ints[%d] = %d\n", size, i, ints[i]);
        if (str_arrary[i] == NULL) {
            str_arrary[i] = malloc(128);
            memset(str_arrary[i], 0, 128);
            strcpy(str_arrary[i], "");
        }
    }
    string_array = cJSON_CreateStringArray(str_arrary, size);

    printJson = cJSON_Print(int_array);
    // printf("int array json: \n %s\n", printJson);
    if (printJson != NULL) {
        free(printJson);
    }
    printJson = cJSON_Print(float_array);
    // printf("float array json: \n %s\n", printJson);
    if (printJson != NULL) {
        free(printJson);
    }
    printJson = cJSON_Print(double_array);
    // printf("double array json: \n %s\n", printJson);
    if (printJson != NULL) {
        free(printJson);
    }
    printJson = cJSON_Print(string_array);
    // printf("strings array json: \n %s\n", printJson);
    if (printJson != NULL) {
        free(printJson);
    }

    cJSON_AddItemToArray(array, int_array);
    cJSON_InsertItemInArray(array, size, float_array);
    cJSON_ReplaceItemInArray(array, size, cJSON_CreateNull());
    size = cJSON_GetArraySize(array);
    cJSON_AddItemReferenceToArray(array, double_array);
    printJson = cJSON_Print(array);
    // printf("array json: \n %s\n", printJson);
    if (printJson != NULL) {
        free(printJson);
    }
    cJSON *detached = cJSON_DetachItemFromArray(array, size);
    printJson = cJSON_Print(detached);
    // printf("array json: \n %s\n", printJson);
    if (printJson != NULL) {
        free(printJson);
    }
    if (detached != NULL) {
        cJSON_Delete(detached);
    }
    if (float_array != NULL) {
        // cJSON_Delete(float_array);
    }
    if (double_array != NULL) {
        cJSON_Delete(double_array);
    }
    if (string_array != NULL) {
        cJSON_Delete(string_array);
    }

    for (int i = 0; i < sizeof(str_arrary) / sizeof(char *); i++) {
        if (str_arrary[i] != NULL) free(str_arrary[i]);
    }
}

cJSON *test_cJSON_Object(char *content) {
    cJSON *root = NULL;
    cJSON *fld = NULL;
    cJSON *item_obj = NULL;
    char *printJson = NULL;

    printf("--------------test_cJSON_Object\n");
    /* Our array of "records": */
    root = cJSON_CreateObject();

    char delim[] = "\n";
    char rawBuffer[1096] = {0};
    int len = 0;
    char *ptr = strtok(content, delim);

    while (ptr != NULL) {
        // printf("'%s'\n", ptr);
        len = strlen(ptr);
        if (len > 1024) {
            len = 1024;
        }
        memset(rawBuffer, 0, 1096);
        memcpy(rawBuffer, ptr, len);
        cJSON_AddItemToObject(root, "splitted-string", fld = cJSON_CreateObject());
        cJSON_AddStringToObject(fld, "type", ptr);
        cJSON_AddNumberToObject(fld, "len", strlen(ptr));
        (strlen(ptr) % 2 == 0) ? cJSON_AddTrueToObject(fld, "even") : cJSON_AddFalseToObject(fld, "even");
        sprintf(rawBuffer, "{\"raw-data\":\"%s\"}", ptr);
        cJSON_AddRawToObject(root, "raw-object", rawBuffer);
        // printf("-\t len=%d, rawBuffer=%s\n", len, rawBuffer);
        ptr = strtok(NULL, delim);
    }
    item_obj = cJSON_AddObjectToObject(root, "item-object");
    cJSON_AddStringToObject(item_obj, "object-name", "testing");
    cJSON_AddItemToObjectCS(root, "CS-string", cJSON_CreateString("go CS-string"));
    cJSON_AddNullToObject(root, "NULL-ITEM");
    cJSON_AddBoolToObject(root, "Bool-item", 0);
    cJSON_AddNumberToObject(root, "number-item", 999);
    cJSON_AddStringToObject(root, "string-item", "hello world!!!");
    cJSON_AddRawToObject(root, "raw-item", "{\"name\": \"MIUI 4K\"}");

    cJSON_AddArrayToObject(root, "Array");
    cJSON_AddArrayToObject(root, "Array-replace1");
    cJSON_AddArrayToObject(root, "Array-replace2");
    cJSON_AddArrayToObject(root, "Array-replace3");

    printJson = cJSON_Print(root);
    // printf("-\tpased json: \n %s\n", printJson);
    if (printJson != NULL) {
        free(printJson);
    }

    return root;
}

void test_cJSON_Minify(char *content) {
    cJSON *parsed = NULL;
    char *printJson = NULL;

    printf("--------------test_cJSON_Minify\n");
    char unclosed_multiline_comment[] = "/* bla";
    char pending_escape[] = "\"\\";
    cJSON_Minify(unclosed_multiline_comment);
    cJSON_Minify(pending_escape);

    parsed = cJSON_Parse(content);
    cJSON_IsInvalid(parsed);
    printJson = cJSON_Print(parsed);
    // printf("pased json: \n %s\n", printJson);

    cJSON_Minify(printJson);
    // printf("Minified:%s\n", printJson);
    if (parsed != NULL) {
        cJSON_Delete(parsed);
    }
    if (printJson != NULL) {
        free(printJson);
    }

    char buff[16];
    memset(buff, 0, 16);
    if (strlen(content) >= 8 && strlen(content) < 16) {
        strcpy(buff, content);
        printf("before cJSON_Minify, buff:\n%s\n", buff);
        cJSON_Minify(buff);
        printf("After cJSON_Minify, buff:\n%s\n", buff);
    }
    if (strlen(content) % 6 == 0) {
        char testdata[8] = {'\t', '\t', '\t', '\t', '\"', '\\', 'n', 'n'};
        cJSON_Minify(testdata); // vuln function    CVE-2019-11834
        // printf("target:%s\n",testdata);
    }
}

void test_cJSON_CreateObjectReference(cJSON *item) {
    char *printJson = NULL;
    cJSON *root = NULL;
    printf("--------------test_cJSON_CreateObjectReference\n");
    root = cJSON_CreateObjectReference(item);
    if (cJSON_IsObject(root)) {
        printf("-\tIt is an object!\n");
    }
    printJson = cJSON_Print(root->child);
    // printf("-\tobject printjson:%s\n", printJson);
    if (printJson != NULL) {
        free(printJson);
    }

    if (root != NULL) {
        cJSON_Delete(root);
    }
}

static cJSON_bool compare_from_string(const char *const a, const char *const b, const cJSON_bool case_sensitive) {
    cJSON *a_json = NULL;
    cJSON *b_json = NULL;
    cJSON_bool result = false;

    a_json = cJSON_Parse(a);
    b_json = cJSON_Parse(b);

    result = cJSON_Compare(a_json, b_json, case_sensitive);

    cJSON_Delete(a_json);
    cJSON_Delete(b_json);

    return result;
}

void test_cJSON_Compare(cJSON *a, cJSON *b) {
    printf("--------------test_cJSON_Compare\n");
    printf("-\tcJSON Compare in case insensitive, a == a? [%d]\n", cJSON_Compare(a, a, 0));
    printf("-\tcJSON Compare in case insensitive, a == b? [%d]\n", cJSON_Compare(a, b, 0));
    printf("-\tcJSON Compare in case sensitive, a == b? [%d]\n", cJSON_Compare(a, b, 1));
    cJSON *duplicate = cJSON_Duplicate(a, true);
    printf("-\tcJSON Compare in case sensitive, a == duplicate? [%d], case insensitive? [%d] \n", cJSON_Compare(a, duplicate, true),
           cJSON_Compare(a, duplicate, false));
    if (duplicate != NULL) {
        cJSON_Delete(duplicate);
    }
    cJSON_Compare(NULL, NULL, false);
    cJSON_Compare(NULL, NULL, true);
    cJSON invalid[1];
    memset(invalid, '\0', sizeof(invalid));
    invalid->type = cJSON_Number | cJSON_String;
    cJSON_Compare(invalid, invalid, true);
    cJSON_Compare(invalid, invalid, false);
    compare_from_string("[1,2,3]", "[1,2]", true);
    compare_from_string("[1,2,3]", "[1,2]", false);
}

void test_cJSON_Replace_Object(cJSON *root) {
    char *printJson = NULL;
    printf("--------------test_cJSON_Replace_Object\n");
    cJSON *item = cJSON_GetObjectItem(root, "Array-replace1");
    // if (item == NULL || cJSON_IsInvalid(item))
    // {
    //     printf("-\tFailed to get object item: Array-replace1!");
    // }

    printJson = cJSON_Print(item);
    printf("-\titem-object:%s\n", printJson);
    if (printJson != NULL || !cJSON_IsObject(item) || cJSON_IsInvalid(item)) {
        free(printJson);
    }

    cJSON *cjson_skill = cJSON_CreateArray();
    cJSON_AddItemToArray(cjson_skill, cJSON_CreateString("C"));
    cJSON_AddItemToArray(cjson_skill, cJSON_CreateString("C++"));
    cJSON_AddItemToArray(cjson_skill, cJSON_CreateString("Python"));
    cJSON_AddItemToArray(cjson_skill, cJSON_CreateString("Java"));

    cJSON_ReplaceItemViaPointer(root, item, cjson_skill);

    cJSON_ReplaceItemInObject(root, "raw-item", cJSON_CreateRaw("{\"name\": \"abc testing...\"}"));
    cJSON_ReplaceItemInObjectCaseSensitive(root, "string-item", cJSON_CreateString("repleaced string"));

    item = cJSON_GetObjectItem(root, "number-item");
    cJSON_SetNumberHelper(item, INT_MAX);
    cJSON_SetNumberHelper(item, INT_MAX + 1);
    cJSON_SetNumberHelper(item, INT_MIN);
    cJSON_SetNumberHelper(item, INT_MIN - 1);
    cJSON_SetNumberHelper(item, 33);

    item = cJSON_GetObjectItem(root, "string-item");
    // cJSON_SetValuestring(item, "repleace the string again!");
}

void test_cJSON_Remove_Object(cJSON *root) {
    char *printJson = NULL;
    printf("--------------test_cJSON_Remove_Object\n");
    cJSON *item = cJSON_GetObjectItem(root, "item-object");
    // if (item == NULL || cJSON_IsInvalid(item))
    // {
    //     printf("-\tFailed to to item-object!");
    // }

    printJson = cJSON_Print(item);
    printf("-\titem-object:%s\n", printJson);
    if (printJson != NULL || !cJSON_IsObject(item) || cJSON_IsInvalid(item)) {
        free(printJson);
    }
    cJSON_DeleteItemFromObjectCaseSensitive(root, "ITEM-object");
    cJSON_DeleteItemFromObject(root, "item-object");
    item = cJSON_DetachItemFromObject(root, "Bool-item");
    if (item != NULL) {
        cJSON_Delete(item);
    }
    item = cJSON_DetachItemFromObjectCaseSensitive(root, "null-item");
    // if (item != NULL)
    // {
    //     cJSON_Delete(item);
    // }
    item = cJSON_DetachItemFromObjectCaseSensitive(root, "NULL-ITEM");
    if (item != NULL) {
        cJSON_Delete(item);
    }
    cJSON_DeleteItemFromObjectCaseSensitive(root, "string-item");

    printJson = cJSON_Print(root);
    // printf("-\tremoved item-object:%s\n", printJson);
    if (printJson != NULL) {
        free(printJson);
    }
}

void test_cJSON_Array_Reference(cJSON *array) {
    char *printJson = NULL;
    cJSON *fld = NULL;
    cJSON *root = NULL;
    /* Used by some code below as an example datatype. */
    struct record {
        const char *precision;
        double lat;
        double lon;
        const char *address;
        const char *city;
        const char *state;
        const char *zip;
        const char *country;
    };
    struct record fields[2] = {{"zip", 37.7668, -1.223959e+2, "", "SAN FRANCISCO", "CA", "94107", "US"},
                               {"zip", 37.371991, -1.22026e+2, "", "SUNNYVALE", "CA", "94085", "US"}};
    printf("--------------test_cJSON_Array_Reference\n");

    /* Our array of "records": */
    root = cJSON_CreateArray();
    for (int i = 0; i < 2; i++) {
        cJSON_AddItemToArray(root, fld = cJSON_CreateObject());
        cJSON_AddStringToObject(fld, "precision", fields[i].precision);
        cJSON_AddNumberToObject(fld, "Latitude", fields[i].lat);
        cJSON_AddNumberToObject(fld, "Longitude", fields[i].lon);
        cJSON_AddStringToObject(fld, "Address", fields[i].address);
        cJSON_AddStringToObject(fld, "City", fields[i].city);
        cJSON_AddStringToObject(fld, "State", fields[i].state);
        cJSON_AddStringToObject(fld, "Zip", fields[i].zip);
        cJSON_AddStringToObject(fld, "Country", fields[i].country);
    }
    cJSON_AddItemReferenceToArray(array, root);

    printJson = cJSON_Print(array);
    // printf("-\tAdd Item Reference to Array:%s\n", printJson);
    if (printJson != NULL) {
        free(printJson);
    }
    cJSON_DetachItemFromArray(array, root);
    // if (cJSON_DetachItemFromArray(array, root))
    // {
    //     printJson = cJSON_Print(array);
    //     // printf("-\tDetach Item from Array:%s\n", printJson);
    //     if (printJson != NULL)
    //     {
    //         free(printJson);
    //     }
    // }
    cJSON *duplicate = cJSON_Duplicate(array, true);
    if (root != NULL) {
        cJSON_Delete(root);
    }

    cJSON *weekdays = cJSON_AddArrayToObject(duplicate, "weekdays");
    cJSON_AddItemToArray(weekdays, cJSON_CreateString("Sunday"));
    cJSON_AddItemToArray(weekdays, cJSON_CreateString("Monday"));
    cJSON_AddItemToArray(weekdays, cJSON_CreateString("Tuesday"));
    cJSON_AddItemToArray(weekdays, cJSON_CreateString("Wednesday"));
    cJSON_AddItemToArray(weekdays, cJSON_CreateString("Thursday"));
    cJSON_AddItemToArray(weekdays, cJSON_CreateString("Friday"));
    cJSON_AddItemToArray(weekdays, cJSON_CreateString("Saturday"));
    printJson = cJSON_Print(duplicate);
    // printf("-\tAdd Item Reference to Array:%s\n", printJson);
    if (printJson != NULL) {
        free(printJson);
    }
    if (duplicate != NULL) {
        cJSON_Delete(duplicate);
    }
}

void test_cJSON_CreateArrayReference(cJSON *array) {
    char *printJson = NULL;
    cJSON *root = NULL;
    printf("--------------test_cJSON_CreateArrayReference\n");
    root = cJSON_CreateArrayReference(array);
    if (cJSON_IsArray(root)) {
        printf("-\tIt is an array!, size=%d\n", cJSON_GetArraySize(root->child));
    }
    if (root != NULL) {
        cJSON_Delete(root);
    }
}

void test_cJSON_Del_Array_Item(cJSON *array) {
    char *printJson = NULL;
    cJSON *root = NULL;
    cJSON *item = NULL;
    int index = cJSON_GetArraySize(array);
    printf("--------------test_cJSON_Del_Array_Item\n");
    if (index < 5) {
        return;
    }
    item = cJSON_GetArrayItem(array, index / 2);
    item = cJSON_DetachItemViaPointer(array, item);
    if (item != NULL) {
        cJSON_Delete(item);
    }
    item = cJSON_DetachItemFromArray(array, 0);
    if (item != NULL) {
        cJSON_Delete(item);
    }
    cJSON_DeleteItemFromArray(array, 0);
}

void test_cJSON_AddItemReferenceToObject(cJSON *cobject) {
    char *printJson = NULL;
    cJSON *item = cJSON_CreateString("testing add reference to object!!!");
    cJSON_AddItemReferenceToObject(cobject, "object_reference", item);
    printJson = cJSON_Print(cobject);
    // printf("-\tAdd Item Reference to object:%s\n", printJson);
    if (printJson != NULL) {
        free(printJson);
    }
    if (item != NULL) {
        cJSON_Delete(item);
    }
}

void test_Print_CJSON_Raw(char *content) {
    cJSON *item = NULL;
    cJSON *pasedjson = NULL;
    char *printJson;
    char *tmp;
    printf("--------------test_Print_CJSON_Raw\n");
    // printf("content len=%d, content=%s \n", strlen(content), content);
    char *str_json = malloc(128);
    memset(str_json, 0, 128);
    sprintf(str_json, "\"000000000000000000/%c", *content);
    pasedjson = cJSON_Parse(str_json);
    printJson = cJSON_Print(pasedjson);
    printf("print json string: %s\n", printJson);

    if (printJson != NULL) {
        free(printJson);
    }
    if (pasedjson != NULL) {
        cJSON_Delete(pasedjson);
    }
    free(str_json);

    if (*content >= 125) {

        item = cJSON_CreateRaw("test");
        // tmp = malloc(sizeof(char *));
        // memcpy(tmp, item->valuestring, sizeof(char *));
        // global_hooks.deallocate(item->string);
        free(item->valuestring);
        item->valuestring = NULL;
        printJson = cJSON_Print(item);
        // printf("-\tpased json: \n %s\n", printJson);
        // if (printJson != NULL)
        // {
        //     free(printJson);
        // }
    }
    // item->valuestring = "test";
    if (item != NULL) {
        cJSON_Delete(item);
        // free(tmp);
    }
}

void test_AddItemToObject(cJSON *content) {
    cJSON *item = NULL;
    cJSON *replaced = NULL;
    char *printJson;
    char *tmp;
    printf("--------------test_AddItemToObject\n");
    cJSON *theobject = cJSON_Parse(content);
    printf("\tlen=%d\n", strlen(content));
    if (!cJSON_IsArray(theobject)) {
        if (theobject != NULL) {
            cJSON_Delete(theobject);
        }
        return;
    }
    item = cJSON_CreateObject();
    printf("Try to get string for None-string cjson: %s\n", cJSON_GetStringValue(item));
    printf("Try to get string for None-number cjson: %s\n", cJSON_GetNumberValue(item));
    cJSON_AddItemToObject(item, "string", cJSON_CreateString("ShangHai"));
    replaced = cJSON_DetachItemFromObject(item, "string");
    cJSON_AddItemToObject(theobject, replaced->string, replaced);
    if (item != NULL) {
        cJSON_Delete(item);
    }
    if (theobject != NULL) {
        cJSON_Delete(theobject);
    }
}

void test_cJSON_malloc() {
    cJSON *root = NULL;
    printf("--------------test_cJSON_malloc\n");
    root = cJSON_malloc(128);
    cJSON_free(root);
}

void test_cJSON_SetValuestring(char *content) {
    cJSON *item = NULL;
    printf("--------------test_cJSON_SetValuestring\n");
    item = cJSON_CreateString("go go up!");
    if (strlen(content) % 66 == 0) item->valuestring = NULL;
    cJSON_SetValuestring(item, "go to finishing!");
    cJSON_SetValuestring(item, "");
    cJSON_SetValuestring(item, content);
    cJSON_Delete(item);
}

void test_cJSON_ParseWithLength(char *content) {
    cJSON *item = NULL;
    printf("--------------test_cJSON_ParseWithLength\n");
    item = cJSON_ParseWithLength(content, strlen(content));
    cJSON_Delete(item);
}

void parse_with_encoding(char *data) {
    cJSON *item = NULL;
    char *printJson;
    item = cJSON_ParseWithLength(data, strlen(data));
    printJson = cJSON_Print(item);
    printf("print json string: %s\n", printJson);
    if (printJson != NULL) {
        free(printJson);
    }
    cJSON_Delete(item);
}

void test_different_encoding(char *content) {
    printf("--------------test_different_encoding\n");

    char *str_encoded = malloc(strlen(content) * 2 + 1);
    char *str_buff = malloc(strlen(content) * 2 + 256);
    memset(str_encoded, 0, strlen(content) * 2 + 1);
    memset(str_buff, 0, strlen(content) * 2 + 256);

    strConvEncoding(content, str_encoded, "UTF-8", "ASCII");
    sprintf(str_buff, "{\"name\": \"%s\"}", str_encoded);
    parse_with_encoding(str_buff);

    strConvEncoding(content, str_encoded, "UTF-8", "UTF-16");
    sprintf(str_buff, "{\"name\": \"%s\"}", str_encoded);
    parse_with_encoding(str_buff);

    strConvEncoding(content, str_encoded, "UTF-8", "Unicode");
    sprintf(str_buff, "{\"name\": \"%s\"}", str_encoded);
    parse_with_encoding(str_buff);

    strConvEncoding(content, str_encoded, "UTF-8", "GB2312");
    sprintf(str_buff, "{\"name\": \"%s\"}", str_encoded);
    parse_with_encoding(str_buff);

    sprintf(str_buff, "%s", "\\u34\\u123\\u34\\u110\\u97\\u109\\u101\\u34\\u58\\u32\\u34\\u97\\u98\\u99\\u34\\u125\\u34");
    parse_with_encoding(str_buff);

    parse_with_encoding("\x22\x7b\x22\x6e\x61\x6d\x65\x22\x3a\x20\x22\x61\x62\x63\x22\x7d\x22");

    parse_with_encoding("\\u\xD800{\"name\": \" unicode \"}");

    parse_with_encoding(content);

    free(str_encoded);
    free(str_buff);
}

void test_cJSON_CreateNumber(char *content) {
    cJSON *item = cJSON_CreateNumber(content[0]);
    char *printJson;
    float number;
    memcpy(&number, content, sizeof(float) > strlen(content) ? strlen(content) : sizeof(float));
    cJSON_Delete(item);
    item = cJSON_CreateNumber(number);
    printJson = cJSON_Print(NULL);
    free(printJson);
    printJson = cJSON_Print(item);
    free(printJson);
    cJSON_Delete(item);
}

void test_cJSON_InsertItemInArray(char *content) {
    printf("--------------test_cJSON_InsertItemInArray\n");
    cJSON *array = cJSON_CreateArray();
    // cJSON *temp1 = cJSON_CreateString("item1");
    cJSON *temp2 = cJSON_CreateString("item2");

    // add_item_to_array(array, temp1);
    // add_item_to_array(array, temp2);
    cJSON_AddItemToArray(array, cJSON_CreateString("item1"));
    cJSON_AddItemToArray(array, temp2);

    // manually set the prev to be NULL to make a corrupted array
    if (strlen(content) % 3 == 0) {
        temp2->prev = NULL;
    }

    // SEGV as after_inserted->prev is NULL, which is passed to newitem->prev, making newitem->prev->next a NULL pointer using
    cJSON_InsertItemInArray(array, 1, cJSON_CreateString("item"));

    cJSON_Delete(array);
}

int test_cjson(char *content) {
    cJSON *root = NULL;
    cJSON *root_Array = NULL;
    char *printJson = NULL;

    printf("cJson library version: [%s]\n", cJSON_Version());
    test_AddItemToObject(content);

    test_cJSON_CreateStringReference(content);

    test_cJSON_Hooks(content);
    test_cJSON_Parse(content);
    test_cJSON_ParseWithOpts(content);
    test_cJSON_ParseWithLengthOpts(content);
    test_cJSON_Print(content);
    root_Array = test_cJSON_Array(content);
    test_cJSON_Array_Reference(root_Array);
    if (root_Array != NULL) {
        cJSON_Delete(root_Array);
    }
    root_Array = test_cJSON_Array(content);
    test_cJSON_ArrayForEach(root_Array);
    test_cJSON_CreateArrayReference(root_Array);
    test_cJSON_Del_Array_Item(root_Array);

    test_cJSON_Minify(content);

    root = test_cJSON_Object(content);
    test_cJSON_CreateObjectReference(root);

    test_cJSON_Compare(root, root_Array);
    test_cJSON_AddItemReferenceToObject(root);
    if (root != NULL) {
        cJSON_Delete(root);
    }
    root = test_cJSON_Object(content);
    test_cJSON_Replace_Object(root);

    test_Print_CJSON_Raw(content);

    test_cJSON_Remove_Object(root);

    // printJson = cJSON_Print(root_Array);
    // printf("array json: \n %s\n", printJson);
    // if (printJson != NULL)
    // {
    //     free(printJson);
    // }

    test_cJSON_malloc();
    test_cJSON_SetValuestring(content);
    test_cJSON_ParseWithLength(content);
    test_different_encoding(content);
    test_cJSON_CreateNumber(content);
    if (strlen(content) % 11 == 0) {
        test_cJSON_InsertItemInArray(content);
    }

    /* cleanup resources */
    reset(root);
    reset(root_Array);
    if (root_Array != NULL) {

        cJSON_Delete(root_Array);
    }
    if (root != NULL) {

        cJSON_Delete(root);
    }
}
