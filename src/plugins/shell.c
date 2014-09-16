/*
 * Shell command plugin for Test Gear
 *
 * Copyright (c) 2012-2013, Martin Lund
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


static int shell_command(void)
{
    char *command;
    int status;

    command = get_string("command");

    printf("Firing command: %s\n", command);

    status = system(command);
    return WEXITSTATUS(status);
}


// Plugin commands
static struct plugin_command_table shell_commands[] =
{
    {   .name = "run",
        .function = shell_command,
        .description = "Run command" },

    { }
};


// Plugin variables
static struct plugin_var_table shell_vars[] =
{
    {   .name = "command",
        .type = STRING,
        .value = "",
        .description = "Command string" },

    { }
};


// Plugin configuration
struct plugin shell = {
    .name = "shell",
    .version = "0.1",
    .description = "Shell plugin",
    .author = "Martin Lund",
    .license = "BSD-3",
    .commands = shell_commands,
    .vars = shell_vars,
};


plugin_register(shell);
