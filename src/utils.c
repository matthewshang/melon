#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

#include <Windows.h>

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

double milliseconds()
{
    LARGE_INTEGER time;
    LARGE_INTEGER frequency;
    double pcfreq;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&time);
    pcfreq = (double)frequency.QuadPart / 1000.0;

    return (double) time.QuadPart / pcfreq;
}