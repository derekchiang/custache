// Copyright (c) 2014, Derek Chiang <derekchiang93@gmail.com>
//
// This software is licensed under the BSD 2-Clause License.
// I trust that you will use it for good :)

// cgems, a personal collection of useful routines in C

#pragma once

#include <stdlib.h>
#include <stdio.h>

#pragma mark =============== CLANG ===============

#define SHOULD_NOT_REACH printf("error: should not reach %s:%d", __FILE__, __LINE__); exit(-1);

#pragma mark =============== IO ===============
 
// Read an entire file and return its content as a malloc-ed string
static char *read_entire_file(const char *filename) {
    FILE *fp;
    long lSize;
    char *buffer;

    fp = fopen(filename , "rb");
    if(!fp) perror(filename), exit(1);

    fseek(fp, 0L, SEEK_END);
    lSize = ftell(fp);
    rewind(fp);

    buffer = calloc(1, lSize + 1);
    if (!buffer) fclose(fp), fputs("memory alloc fails",stderr), exit(1);

    if (fread(buffer, lSize, 1, fp) != 1)
        fclose(fp), free(buffer), fputs("entire read fails",stderr), exit(1);

    fclose(fp);
    return buffer;
}

