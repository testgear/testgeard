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
#include <stdarg.h>
#include <string.h>
#include "testgear/plugin.h"

static struct plugin *plugin;
static struct plugin_properties *property;
static FILE *log_file;

void log_info(const char *format, ...)
{
    va_list args;
    fprintf(log_file, "[%s] ", plugin->name);
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    fprintf(log_file, "\n");
}

void log_error(const char *format, ...)
{
    va_list args;
    fprintf(log_file, "[%s] Error: ", plugin->name);
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    fprintf(log_file, "\n");
}

static void verify_properties(struct plugin_properties *property)
{
    int i,j;

    // Check for duplicate property names
    for (i=0; property[i].name; i++)
    {
        for (j=0; property[j].name; j++)
        {
            // Do not compare itself
            if (i != j)
            {
                if (strcmp(property[i].name, property[j].name) == 0)
                    printf("Warning: Duplicate property name found! (%s)\n", property[i].name);
            }
        }
    }
}

static void initialize_properties(struct plugin_properties *property)
{
    int i;

    for (i=0; property[i].name; i++)
    {
        switch (property[i].type)
        {
            case CHAR:
                property[i].data = malloc(sizeof(char));
                *((char *)property[i].data) = 0;
                break;
            case SHORT:
                property[i].data = malloc(sizeof(short));
                *((short *)property[i].data) = 0;
                break;
            case INT:
                property[i].data = malloc(sizeof(int));
                *((int *)property[i].data) = 0;
                break;
            case LONG:
                property[i].data = malloc(sizeof(long));
                *((long *)property[i].data) = 0;
                break;
            case FLOAT:
                property[i].data = malloc(sizeof(float));
                *((float *)property[i].data) = 0;
                break;
            case DOUBLE:
                property[i].data = malloc(sizeof(double));
                *((double *)property[i].data) = 0;
                break;
            case STRING:
                property[i].data = malloc(4);
                strcpy(property[i].data, "");
                break;
            case DATA:
                property[i].data = NULL;
                break;
            case COMMAND:
            default:
                break;
        }
    }
}

int init(struct init_data *data)
{
    log_file = data->log_file;
    verify_properties(plugin->properties);
    initialize_properties(plugin->properties);
    return 0;
}

void register_plugin(struct plugin *plug)
{
    plugin = plug;
    property = plug->properties;
    plug->init = &init;
}

int list_properties(char *properties)
{
    int i;
    char buffer[4096] = "";

    // Traverse all variables and commands
    for (i=0; property[i].name; i++)
    {
        if (property[i].type != COMMAND)
        {
            // Variable found
            sprintf(buffer, "%s:", property[i].name);
            strcat(properties, buffer);
            sprintf(buffer, "%d,", property[i].type);
            strcat(properties, buffer);
        } else
        {
            // Command found
            sprintf(buffer, "%s:%d,", property[i].name, COMMAND);
            strcat(properties, buffer);
        }
    }

    // Remove trailing ','
    properties[strlen(properties)-1] = 0;

    return 0;
}

static int find_property(char *name, int type)
{
    int i;

    // Find property matching name
    for (i=0; property[i].name; i++)
    {
        if (strcmp(name, property[i].name) == 0)
        {
            if (type != -1)
            {
                // Check matching type
                if (property[i].type != type)
                {
                    printf("Warning: Property %s has different type\n", property[i].name);
                    return -1;
                }
            }
            return i;
        }
    }

    printf("Warning: Property %s not found\n", name);
    return -1;
}

int get__char(char *name, char *value)
{
    int i = find_property(name, CHAR);
    if (i >= 0)
    {
        *value = *((char *)property[i].data);
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
    int i = find_property(name, CHAR);
    if (i >= 0)
    {
        *((char *)property[i].data) = value;
        return 0;
    }

    return -1;
}

int get__short(char *name, short *value)
{
    int i = find_property(name, SHORT);
    if (i >= 0)
    {
        *value = *((short *)property[i].data);
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
    int i = find_property(name, SHORT);
    if (i >= 0)
    {
        *((short *)property[i].data) = value;
        return 0;
    }

    return -1;
}

int get__int(char *name, int *value)
{
    int i = find_property(name, INT);
    if (i >= 0)
    {
        *value = *((int *)property[i].data);
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
    int i = find_property(name, INT);
    if (i >= 0)
    {
        *((int *)property[i].data) = value;
        return 0;
    }

    return -1;
}

int get__long(char *name, long *value)
{
    int i = find_property(name, LONG);
    if (i >= 0)
    {
        *value = *((long *)property[i].data);
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
    int i = find_property(name, LONG);
    if (i >= 0)
    {
        *((long *)property[i].data) = value;
        return 0;
    }

    return -1;
}

int get__float(char *name, float *value)
{
    int i = find_property(name, FLOAT);
    if (i >= 0)
    {
        *value = *((float *)property[i].data);
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
    int i = find_property(name, FLOAT);
    if (i >= 0)
    {
        *((float *)property[i].data) = value;
        return 0;
    }

    return -1;
}

int get__double(char *name, double *value)
{
    int i = find_property(name, DOUBLE);
    if (i >= 0)
    {
        *value = *((double *)property[i].data);
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
    int i = find_property(name, DOUBLE);
    if (i >= 0)
    {
        *((double *)property[i].data) = value;
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

    // Lookup property
    int i = find_property(name, STRING);
    if (i >= 0)
        return (char *)property[i].data;

    // Property not found
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
    int i = find_property(name, STRING);
    if (i >= 0)
    {
        // Free previously allocated memory
        free(property[i].data);

        // Reallocate new memory
        property[i].data = malloc(strlen(value)+1);

        // Set new string
        strcpy(property[i].data, value);

        return 0;
    }

    return -1;
}

int run(char *command_name, int *return_value)
{
    int (*function)(void);

    int i = find_property(command_name, COMMAND);
    if (i >= 0)
    {
        function = property[i].function;
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

    // Lookup property
    i = find_property(name, -1);
    if (i >= 0)
        return (char *)property[i].description;

    // Variable or command not found
    return NULL;
}
