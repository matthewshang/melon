#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

const char *file_read(const char *path)
{
    FILE *f = NULL;
    char *ret = NULL;
    long  fsize = 0;

    f = fopen(path, "rb");
    if (!f) goto abort_open;
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    ret = malloc(fsize + 1);
    if (!ret) goto abort_read;
    fread(ret, fsize, 1, f);

    ret[fsize] = 0;

abort_read:
    fclose(f);
abort_open:
    return ret;
}

//char** strsplit(const char* string, char delim_char)
//{
//    char* copy = _strdup(string);
//    char** ret = NULL;
//    int elements = 0;
//    char* p = copy;
//    char* last = NULL;
//    char delim[2];
//    delim[0] = delim_char;
//    delim[1] = 0;
//    while (*p)
//    {
//        if (*p == delim_char)
//        {
//            elements++;
//            last = p;
//        }
//        p++;
//    }
//
//    if (last < (copy + strlen(copy) - 1)) elements++;
//    elements++;
//    ret = calloc(elements, sizeof(char *));
//
//    if (ret)
//    {
//        int idx = 0;
//        char* token = strtok(copy, delim);
//
//        while (token)
//        {
//            ret[idx++] = _strdup(token);
//            token = strtok(0, delim);
//        }
//        ret[idx] = 0;
//    }
//    free(copy);
//    return ret;
//}