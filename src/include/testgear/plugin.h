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

#ifndef PLUGIN_H
#define PLUGIN_H

#include <stdbool.h>

struct plugin_command_table
{
   const char *name;
   int (*function)(void);
   const char *description;
};

enum var_type
{
   CHAR,
   SHORT,
   INT,
   LONG,
   FLOAT,
   DOUBLE,
   STRING,
   DATA,
   COMMAND // TODO: rename var_type -> type (merge var and command types)
};

struct var_type_description_t
{
   enum var_type type;
   char *description;
};

struct plugin_var_table
{
   const char *name;
   const enum var_type type;
   const char *description;
   void *data;
};

struct plugin
{
   const char *name;
   const char *version;
   const char *description;
   const char *author;
   const char *license;
   int (*load)(void);
   int (*unload)(void);
   int (*get)(void);
   int (*set)(void);
   int (*init)(void);
   struct plugin_command_table *commands;
   struct plugin_var_table *vars;
};

void register_plugin(struct plugin *plugin);

#define plugin_register(x) \
        struct plugin * plugin_register(void) \
        { register_plugin(&x); return &x; }

int set_char(char *name, char value);
int set_short(char *name, short value);
int set_int(char *name, int value);
int set_long(char *name, long value);
int set_float(char *name, float value);
int set_double(char *name, double value);
int set_string(char *name, char *value);
int set_data(char *name, void *value);

char   get_char(char *name);
short  get_short(char *name);
int    get_int(char *name);
long   get_long(char *name);
float  get_float(char *name);
double get_double(char *name);
char * get_string(char *name);
void * get_data(void *name);

#endif
