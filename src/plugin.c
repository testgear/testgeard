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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "testgear/plugin.h"

static struct plugin *plugin;
static struct plugin_var_table *var;
static struct plugin_command_table *command;

static void initialize_vars(struct plugin_var_table *var)
{
    int i;

    for (i=0; var[i].name; i++)
    {
        switch (var[i].type)
        {
            case CHAR:
                var[i].data = malloc(sizeof(char));
                *((char *)var[i].data) = 0;
                break;
            case SHORT:
                var[i].data = malloc(sizeof(short));
                *((short *)var[i].data) = 0;
                break;
            case INT:
                var[i].data = malloc(sizeof(int));
                *((int *)var[i].data) = 0;
                break;
            case LONG:
                var[i].data = malloc(sizeof(long));
                *((long *)var[i].data) = 0;
                break;
            case FLOAT:
                var[i].data = malloc(sizeof(float));
                *((float *)var[i].data) = 0;
                break;
            case DOUBLE:
                var[i].data = malloc(sizeof(double));
                *((double *)var[i].data) = 0;
                break;
            case STRING:
                var[i].data = malloc(4);
                strcpy(var[i].data, "");
                break;
            case DATA:
                var[i].data = NULL;
                break;
            default:
                break;
        }
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
    var = plug->vars;
    command = plug->commands;
    plug->init = &init;
}

int list_properties(char *properties)
{
    int i;
    char buffer[256] = "";

    // Traverse all variables
    for (i=0; var[i].name; i++)
    {
        sprintf(buffer, "%s:", var[i].name);
        strcat(properties, buffer);
        sprintf(buffer, "%d,", var[i].type);
        strcat(properties, buffer);
    }

    buffer[0] = 0;

    // Traverse all commands
    for (i=0; command[i].name; i++)
    {
        sprintf(buffer, "%s:%d,", command[i].name, COMMAND);
        strcat(properties, buffer);
    }

    properties[strlen(properties)-1] = 0;

    return 0;
}

static int find_variable(char *name, int type)
{
    int i;

    // Find variable matching name
    for (i=0; var[i].name; i++)
    {
        if (strcmp(name, var[i].name) == 0)
        {
            if (type != -1)
            {
                // Check matching type
                if (var[i].type != type)
                {
                    printf("Warning: Variable %s has different type\n", var[i].name);
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
    for (i=0; command[i].name; i++)
    {
        if (strcmp(name, command[i].name) == 0)
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
        *value = *((char *)var[i].data);
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

int set_char(char *name, char value)
{
    int i = find_variable(name, CHAR);
    if (i >= 0)
    {
        *((char *)var[i].data) = value;
        return 0;
    }

    return -1;
}

int get__short(char *name, short *value)
{
    int i = find_variable(name, SHORT);
    if (i >= 0)
    {
        *value = *((short *)var[i].data);
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

int set_short(char *name, short value)
{
    int i = find_variable(name, SHORT);
    if (i >= 0)
    {
        *((short *)var[i].data) = value;
        return 0;
    }

    return -1;
}

int get__int(char *name, int *value)
{
    int i = find_variable(name, INT);
    if (i >= 0)
    {
        *value = *((int *)var[i].data);
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
        *((int *)var[i].data) = value;
        return 0;
    }

    return -1;
}

int get__long(char *name, long *value)
{
    int i = find_variable(name, LONG);
    if (i >= 0)
    {
        *value = *((long *)var[i].data);
        return 0;
    }

    printf("Warning: Variable %s not found\n", name);
    return -1;
}

long get_long(char *name)
{
    long value;

    if (get__long(name, &value) == 0)
        return value;

    return -1;
}

int set_long(char *name, long value)
{
    int i = find_variable(name, LONG);
    if (i >= 0)
    {
        *((long *)var[i].data) = value;
        return 0;
    }

    return -1;
}

int get__float(char *name, float *value)
{
    int i = find_variable(name, FLOAT);
    if (i >= 0)
    {
        *value = *((float *)var[i].data);
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

int set_float(char *name, float value)
{
    int i = find_variable(name, FLOAT);
    if (i >= 0)
    {
        *((float *)var[i].data) = value;
        return 0;
    }

    return -1;
}

int get__double(char *name, double *value)
{
    int i = find_variable(name, DOUBLE);
    if (i >= 0)
    {
        *value = *((double *)var[i].data);
        return 0;
    }

    printf("Warning: Variable %s not found\n", name);
    return -1;
}

double get_double(char *name)
{
    double value;

    if (get__double(name, &value) == 0)
        return value;

    return -1;
}

int set_double(char *name, double value)
{
    int i = find_variable(name, DOUBLE);
    if (i >= 0)
    {
        *((double *)var[i].data) = value;
        return 0;
    }

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
        return (char *)var[i].data;

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
        free(var[i].data);

        // Reallocate new memory
        var[i].data = malloc(strlen(value)+1);

        // Set new string
        strcpy(var[i].data, value);

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
        function = command[i].function;
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
        return (char *)var[i].description;

    // Lookup command
    i = find_command(name);
    if (i >= 0)
        return (char *)command[i].description;

    // Variable or command not found
    return NULL;
}
