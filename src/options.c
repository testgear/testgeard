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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include "testgear/options.h"

struct option_t option =
{
    false,
    RUNNING,
    8000
};


void print_options_help(char *argv[])
{
    printf("Usage: %s [options]\n", argv[0]);
    printf("\n");
    printf("Options:\n");
    printf("  --connection <connection>  Connection type ('tcp' or 'serial')\n");
    printf("  --device <device>          Serial device (eg. '/dev/ttyUSB1')\n");
    printf("  --port <port>              TCP listen port (default is %d)\n", option.port);
    printf("  --daemon                   Daemonize\n");
    printf("  --version                  Display version\n");
    printf("  --help                     Display help\n");
    printf("\n");
}

void parse_options(int argc, char *argv[])
{
    int c;

    while (1)
    {
        static struct option long_options[] =
        {
            {"daemon",	no_argument, 0, 'z'},
            {"version",	no_argument, 0, 'v'},
            {"help",	   no_argument, 0, 'h'},
            {0,         0,           0,  0 }
        };

        // getopt_long stores the option index here
        int option_index = 0;

        // Parse argument using getopt_long
        c = getopt_long (argc, argv, "", long_options, &option_index);

        // Detect the end of the options
        if (c == -1)
            break;

        switch (c)
        {
            case 0:
                // If this option set a flag, do nothing else now
                if (long_options[option_index].flag != 0)
                    break;
                printf("option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg %s", optarg);
                printf("\n");
                break;

            case 'z':
                option.daemon = true;
                break;

            case 'v':
                printf("testgeard v%s\n", VERSION);
                printf("Copyright (c) 2012-2014 Martin Lund\n\n");
                printf("This is open source software. See README for license details.\n");
                exit(0);
                break;

            case 'h':
                print_options_help(argv);
                exit(0);
                break;

            case '?':
                // getopt_long already printed an error message
                exit(1);
                break;

            default:
                exit(1);
        }
    }

    // Print any remaining command line arguments (unknown options)
    if (optind < argc)
    {
        printf("%s: unknown arguments: ", argv[0]);
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
        exit(1);
    }
}
