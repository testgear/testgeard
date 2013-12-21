/*
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
#include "testgear/plugin.h"

static struct plugin *plugin;
static struct plugin_var_table *vars;
static struct plugin_command_table *commands;

static void initialize_vars(struct plugin_var_table *vars)
{
    int i;

    for (i=0; vars[i].name; i++)
    {
        if (vars[i].value)
        {
            switch (vars[i].type)
            {
                case CHAR:
                    vars[i].data = malloc(sizeof(char));
                    *((char *)vars[i].data) = atoi(vars[i].value);
                    break;
                case SHORT:
                    vars[i].data = malloc(sizeof(short));
                    *((short *)vars[i].data) = atoi(vars[i].value);
                    break;
                case INT:
                    vars[i].data = malloc(sizeof(int));
                    *((int *)vars[i].data) = atoi(vars[i].value);
                    break;
                case FLOAT:
                    vars[i].data = malloc(sizeof(float));
                    *((float *)vars[i].data) = strtof(vars[i].value, NULL);
                    break;
                case STRING:
                    vars[i].data = malloc(strlen(vars[i].value));
                    strcpy(vars[i].data, vars[i].value);
                    break;
                case DATA:
                    vars[i].data = vars[i].value;
                    break;
                default:
                    break;
            }
        } else
            vars[i].data = NULL;
    }
}

int init(void)
{
    initialize_vars(plugin->vars);
    return 0;
}

void register_plugin(struct plugin *plug)
{
    plugin = plug;
    vars = plug->vars;
    commands = plug->commands;
    plug->init = &init;
}

static int find_variable(char *name, int type)
{
    int i;

    // Find variable matching name
    for (i=0; vars[i].name; i++)
    {
        if (strcmp(name, vars[i].name) == 0)
        {
            if (type != -1)
            {
                // Check matching type
                if (vars[i].type != type)
                {
                    printf("Warning: Variable has different type\n");
                    return -1;
                }
            }
            return i;
        }
    }

    printf("Warning: Variable %s not found\n", name);
    return -1;
}

static int find_command(char *name)
{
    int i;

    // Find command matching name
    for (i=0; commands[i].name; i++)
    {
        if (strcmp(name, commands[i].name) == 0)
            return i;
    }

    printf("Warning: Command %s not found\n", name);
    return -1;
}

int get__char(char *name, char *value)
{
    int i = find_variable(name, CHAR);
    if (i >= 0)
    {
        *value = *((char *)vars[i].data);
        return 0;
    }

    printf("Warning: Variable %s not found\n", name);
    return -1;
}

char get_char(char *name)
{
    char value;

    if (get__char(name, &value) == 0)
        return value;

    return -1;
}


int get__short(char *name, short *value)
{
    int i = find_variable(name, SHORT);
    if (i >= 0)
    {
        *value = *((short *)vars[i].data);
        return 0;
    }

    printf("Warning: Variable %s not found\n", name);
    return -1;
}

short get_short(char *name)
{
    short value;

    if (get__short(name, &value) == 0)
        return value;

    return -1;
}

int get__int(char *name, int *value)
{
    int i = find_variable(name, INT);
    if (i >= 0)
    {
        *value = *((int *)vars[i].data);
        return 0;
    }

    printf("Warning: Variable %s not found\n", name);
    return -1;
}

int get_int(char *name)
{
    int value;

    if (get__int(name, &value) == 0)
        return value;

    return -1;
}

int set_int(char *name, int value)
{
    int i = find_variable(name, INT);
    if (i >= 0)
    {
        *((int *)vars[i].data) = value;
        return 0;
    }

    return -1;
}

int get__float(char *name, float *value)
{
    int i = find_variable(name, FLOAT);
    if (i >= 0)
    {
        *value = *((float *)vars[i].data);
        return 0;
    }

    printf("Warning: Variable %s not found\n", name);
    return -1;
}

float get_float(char *name)
{
    float value;

    if (get__float(name, &value) == 0)
        return value;

    return -1;
}

char * get_string(char *name)
{
    // First, check reserved variable words
    if (strcmp(name, "name") == 0)
        return (char *) plugin->name;
    if (strcmp(name, "version") == 0)
        return (char *) plugin->version;
    if (strcmp(name, "description") == 0)
        return (char *) plugin->description;
    if (strcmp(name, "author") == 0)
        return (char *) plugin->author;
    if (strcmp(name, "license") == 0)
        return (char *) plugin->license;

    // Lookup variable
    int i = find_variable(name, STRING);
    if (i >= 0)
        return (char *)vars[i].data;

    // Variable not found
    return NULL;
}

int set_string(char *name, char *value)
{
    // First, check reserved variable words
    if (strcmp(name, "name") == 0)
        return -1;
    if (strcmp(name, "version") == 0)
        return -1;
    if (strcmp(name, "description") == 0)
        return -1;
    if (strcmp(name, "author") == 0)
        return -1;
    if (strcmp(name, "license") == 0)
        return -1;

    // Lookup variable
    int i = find_variable(name, STRING);
    if (i >= 0)
    {
        // Free previously allocated memory
        free(vars[i].data);

        // Reallocate new memory
        vars[i].data = malloc(strlen(value)+1);

        // Set new string
        strcpy(vars[i].data, value);

        return 0;
    }

    return -1;
}

int run(char *command_name, int *return_value)
{
    int (*function)(void);

    int i = find_command(command_name);
    if (i >= 0)
    {
        function = commands[i].function;
        *return_value = (*function)();
        return 0;
    }

    printf("Warning: Command %s not found\n", command_name);
    return -1;
}

char * describe(char *name)
{
    int i;

    // First, check reserved variable words
    if (name[0] == 0)
        return (char *) plugin->description;
    if (strcmp(name, "name") == 0)
        return "Plugin name";
    if (strcmp(name, "version") == 0)
        return "Plugin version";
    if (strcmp(name, "description") == 0)
        return "Plugin description";
    if (strcmp(name, "author") == 0)
        return "Plugin author";
    if (strcmp(name, "license") == 0)
        return "Plugin license";

    // Lookup variable
    i = find_variable(name, -1);
    if (i >= 0)
        return (char *)vars[i].description;

    // Lookup command
    i = find_command(name);
    if (i >= 0)
        return (char *)commands[i].description;

    // Variable or command not found
    return NULL;
}
