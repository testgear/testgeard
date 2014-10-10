/*
 * Dummy plugin for Test Gear (for testing only)
 *
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "testgear/plugin.h"

static int dummy_load(void)
{
    // Set defaults
    set_char("char0", 0);
    set_short("short0", 0);
    set_int("int0", 0);
    set_long("long0", 0);
    set_float("float0", 0.0);
    set_double("double0", 0.0);
    set_string("string0", "Hi world!");

    return 0;
}

static int dummy_command0(void)
{
    printf("Running command0!\n");

    printf("char0 = %d\n", get_char("char0"));
    printf("short0 = %d\n", get_short("short0"));
    printf("int0 = %d\n", get_int("int0"));
    printf("long0 = %ld\n", get_long("long0"));
    printf("float0 = %f\n", get_float("float0"));
    printf("double0 = %f\n", get_double("double0"));
    printf("string0 = %s\n", get_string("string0"));

    return 0;
}

// Plugin properties
static struct plugin_properties dummy_properties[] =
{
    {   .name = "char0",
        .type = CHAR,
        .description = "Test char 0" },

    {   .name = "short0",
        .type = SHORT,
        .description = "Test short 0" },

    {   .name = "int0",
        .type = INT,
        .description = "Test int 0" },

    {   .name = "long0",
        .type = LONG,
        .description = "Test long 0" },

    {   .name = "float0",
        .type = FLOAT,
        .description = "Test float 0" },

    {   .name = "double0",
        .type = DOUBLE,
        .description = "Test double 0" },

    {   .name = "string0",
        .type = STRING,
        .description = "Test string 0" },

    {   .name = "command0",
        .type = COMMAND,
        .function = dummy_command0,
        .description = "Run command" },

    { }
};

// Plugin configuration
struct plugin dummy =
{
    .name = "dummy",
    .version = "0.2",
    .description = "Dummy plugin (for testing only)",
    .author = "Martin Lund",
    .license = "BSD-3",
    .properties = dummy_properties,
    .load = dummy_load,
};

plugin_register(dummy);
