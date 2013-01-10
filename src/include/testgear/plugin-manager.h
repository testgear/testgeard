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

#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

void plugin_manager_start(void);

int plugin_load(char *name);
int plugin_unload(char *name);

int plugin_get_char(char *plugin_name, char *variable_name, char *value);
int plugin_set_char(char *plugin_name, char *variable_name, char value);

int plugin_get_short(char *plugin_name, char *variable_name, short *value);
int plugin_set_short(char *plugin_name, char *variable_name, short value);

int plugin_get_int(char *plugin_name, char *variable_name, int *value);
int plugin_set_int(char *plugin_name, char *variable_name, int value);

int plugin_get_float(char *plugin_name, char *variable_name, float *value);
int plugin_set_float(char *plugin_name, char *variable_name, float value);

int plugin_get_string(char *plugin_name, char *variable_name, char *value);
int plugin_set_string(char *plugin_name, char *variable_name, char *value);

int plugin_run(char *plugin_name, char *command_name, int *return_value);

int plugin_describe(char *plugin_name, char *name, char *value);

#endif
