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

static int dummy_command0(void)
{
    printf("Running command!\n");
    return 0;
}

// Plugin commands
static struct plugin_command_table dummy_commands[] =
{
    {   .name = "command0",
        .function = dummy_command0,
        .description = "Run command" },

    { }
};

// Plugin variables
static struct plugin_var_table dummy_vars[] =
{
    {   .name = "char0",
        .type = CHAR,
        .value = "",
        .description = "Test char 0" },

    {   .name = "short0",
        .type = SHORT,
        .value = "",
        .description = "Test short 0" },

    {   .name = "int0",
        .type = INT,
        .value = "",
        .description = "Test int 0" },

    {   .name = "long0",
        .type = LONG,
        .value = "",
        .description = "Test long 0" },

    {   .name = "float0",
        .type = FLOAT,
        .value = "",
        .description = "Test float 0" },

    {   .name = "double0",
        .type = DOUBLE,
        .value = "",
        .description = "Test double 0" },

    {   .name = "string0",
        .type = STRING,
        .value = "",
        .description = "Test string 0" },

    { }
};

// Plugin configuration
struct plugin dummy = {
    .name = "dummy",
    .version = "0.1",
    .description = "Dummy plugin (for testing only)",
    .author = "Martin Lund",
    .license = "BSD-3",
    .commands = dummy_commands,
    .vars = dummy_vars,
};

plugin_register(dummy);
