/*
 * Copyright (c) 2012-2014, Martin Lund
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <errno.h>
#include "testgear/log.h"

FILE *log_file = NULL;

static int mkpath(char *dir, mode_t mode)
{
    if (!dir)
    {
        errno = EINVAL;
        return 1;
    }

    if (strlen(dir) == 1 && dir[0] == '/')
        return 0;

    mkpath(dirname(strdupa(dir)), mode);

    return mkdir(dir, mode);
}

void log_init(void)
{
    int status;

    // Make sure log dir exits
    status = mkpath(LOGDIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if ((status != 0) && (errno != EEXIST))
    {
        fprintf(stderr, "Error: Unable to create log directory (%s)\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Open log file
    log_file = fopen(LOGDIR "/testgeard.log", "w+");
    if (log_file == NULL)
    {
        fprintf(stderr, "Error: Unable to open log file (%s)\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Disable buffering
    setbuf(log_file, NULL);
}

void log_exit(void)
{
    int status;

    if (log_file)
    {
        // Close log file
        status = fclose(log_file);
        if (status != 0)
            fprintf(stderr, "Error: Could not close log file (%s)\n", strerror(errno));
    }
}

void log_info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    fprintf(log_file, "\n");
}

void log_error(const char *format, ...)
{
    va_list args;
    fprintf(log_file, "Error: ");
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    fprintf(log_file, "\n");
}
