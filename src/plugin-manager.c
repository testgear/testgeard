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
#include <dlfcn.h>
#include <string.h>
#include <errno.h>
#include "testgear/plugin-manager.h"
#include "testgear/plugin.h"
#include "testgear/debug.h"
#include "testgear/list.h"

list_p plugin_list;

struct plugin_item_t
{
    char name[256];
    void *handle;
};

struct plugin_item_t plugin_item;
struct plugin_item_t *plugin_item_p;

static void plugin_print_info(struct plugin *plugin)
{
    int i;

    // Print plugin information
    debug_printf("\n");
    debug_printf("Plugin information\n");
    debug_printf("------------------\n");
    debug_printf(" Name: %s\n", plugin->name);
    debug_printf(" Version: %s\n", plugin->version);
    debug_printf(" Description: %s\n", plugin->description);
    debug_printf(" Author: %s\n", plugin->author);
    debug_printf(" License: %s\n", plugin->license);
    debug_printf(" Commands: ");
    for (i=0; plugin->commands[i].name; i++)
        debug_printf_raw("%s ", plugin->commands[i].name);
    debug_printf_raw("\n");
    debug_printf(" Variables: ");
    for (i=0; plugin->vars[i].name; i++)
        debug_printf_raw("%s ", plugin->vars[i].name);
    debug_printf_raw("\n");
    debug_printf("\n");
}

int list_plugins(char *plugins)
{
    FILE *fp;
    char line_buffer[256];

    fp = popen("cd plugins; ls *.so", "r");
    if (fp == NULL)
    {
         printf("Error: %s\n", strerror(errno));
         return -1;
    }

    while (fgets(line_buffer, 256, fp) != NULL)
    {
        int length = strlen(line_buffer);
        line_buffer[length-1-3] = ',';
        line_buffer[length-3] = 0;
        strcat(plugins, line_buffer);
    }

    plugins[strlen(plugins)-1] = 0;

    pclose(fp);

    return 0;
}

int plugin_load(char *name)
{
    char filename[256];
    struct plugin * (*plugin_register)(void);
    int (*plugin_load)();
    struct plugin *plugin;
    struct plugin_command_table *commands;
    char *error;

    debug_printf("Loading plugin %s\n", name);

    // Fill in name in list element
    strcpy(plugin_item.name, name);

    // Check that plugin is not already loaded
    list_iter_p iter = list_iterator(plugin_list, FRONT);
    while (list_next(iter) != NULL)
    {
        plugin_item_p = (struct plugin_item_t *)list_current(iter);
        if (strcmp(plugin_item_p->name, name) == 0)
        {
            printf("Error: Plugin already loaded!\n");
            return -1;
        }
    }

    // Add location
    sprintf(filename, "./plugins/%s.so", name);

    // Open plugin
    plugin_item.handle = dlopen(filename, RTLD_LAZY);
    if (!plugin_item.handle)
    {
        fprintf(stderr, "%s\n", dlerror());
        return -1;
    }
    else
    {
        // Add plugin to list of loaded plugins
        list_add(plugin_list, &plugin_item, sizeof(plugin_item));

        // Call plugin_register()
        plugin_register = dlsym(plugin_item.handle, "plugin_register");
        if ((error = dlerror()) != NULL)
            fprintf(stderr, "%s\n", error);
        plugin = (*plugin_register)();

        // Initialize plugin variabes
        plugin->init();

        // Print plugin information
        plugin_print_info(plugin);

        // Call plugin load callback (if defined)
        if (plugin->load != NULL)
        {
            plugin_load = plugin->load;
            (*plugin_load)();
        }
    }

    return 0;
}

int plugin_unload(char *name)
{
    struct plugin * (*plugin_register)(void);
    int (*plugin_unload)(void);
    struct plugin *plugin;
    char *error;
    bool found = false;

    debug_printf("Unloading plugin %s\n", name);

    // Check that the plugin is loaded
    list_iter_p iter = list_iterator(plugin_list, FRONT);
    while (list_next(iter) != NULL)
    {
        plugin_item_p = (struct plugin_item_t *)list_current(iter);
        if (strcmp(plugin_item_p->name, name) == 0)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        printf("Error: Plugin not found!\n");
        return -1;
    }

    // Call plugin_register()
    plugin_register = dlsym(plugin_item_p->handle, "plugin_register");
    if ((error = dlerror()) != NULL)
    {
        fprintf(stderr, "%s\n", error);
        return -1;
    }
    plugin = (*plugin_register)();

    // Call plugin unload callback (if defined)
    if (plugin->unload != NULL)
    {
        plugin_unload = plugin->unload;
        (*plugin_unload)();
    }

    // Unload plugin
    if (dlclose(plugin_item_p->handle))
    {
        fprintf(stderr, "%s\n", error);
        return -1;
    }

    // Remove plugin from list of loaded plugins
    list_pluck(plugin_list, iter->current);

    return 0;
}

void plugin_manager_start(void)
{
    // Initialize plugins list
    plugin_list = create_list();
}

void * get_symbol_handle(char *plugin_name, char *symbol)
{
    bool found = false;
    char *error;
    void *symbol_handle;

    // Find plugin handle
    list_iter_p iter = list_iterator(plugin_list, FRONT);
    while (list_next(iter) != NULL)
    {
        plugin_item_p = (struct plugin_item_t *)list_current(iter);
        if (strcmp(plugin_item_p->name, plugin_name) == 0)
        {
            found = true;
            break;
        }
    }

    if (found)
        debug_printf("Found plugin %s\n", plugin_name);
    else
    {
        printf("Error: Plugin %s is not found!\n", plugin_name);
        return NULL;
    }

    // Resolve symbol handle for plugin
    symbol_handle = dlsym(plugin_item_p->handle, symbol);
    if ((error = dlerror()) != NULL)
    {
        fprintf(stderr, "%s\n", error);
        return NULL;
    }

    return symbol_handle;
}

int plugin_list_properties(char *plugin_name, char *properties)
{
    int (*list__properties)(char *properties);

    list__properties = get_symbol_handle(plugin_name, "list_properties");
    return (*list__properties)(properties);
}

int plugin_get_char(char *plugin_name, char *variable_name, char *value)
{
    int (*get__char)(char *name, char *value);

    get__char = get_symbol_handle(plugin_name, "get__char");
    return (*get__char)(variable_name, value);
}

int plugin_get_short(char *plugin_name, char *variable_name, short *value)
{
    int (*get__short)(char *name, short *value);

    get__short = get_symbol_handle(plugin_name, "get__short");
    return (*get__short)(variable_name, value);
}

int plugin_get_int(char *plugin_name, char *variable_name, int *value)
{
    int (*get__int)(char *name, int *value);

    get__int = get_symbol_handle(plugin_name, "get__int");
    return (*get__int)(variable_name, value);
}

int plugin_get_float(char *plugin_name, char *variable_name, float *value)
{
    int (*get__float)(char *name, float *value);

    get__float = get_symbol_handle(plugin_name, "get__float");
    return (*get__float)(variable_name, value);
}

int plugin_get_string(char *plugin_name, char *variable_name, char *value)
{
    char *string;
    char * (*get_string)(char *name);

    get_string = get_symbol_handle(plugin_name, "get_string");
    string = (*get_string)(variable_name);

    if (string == NULL)
        return -1;
    else
    {
        strcpy(value, string);
        return 0;
    }
}

int plugin_set_char(char *plugin_name, char *variable_name, char value)
{
    int (*set_char)(char *name, char value);

    set_char = get_symbol_handle(plugin_name, "set_char");
    return (*set_char)(variable_name, value);
}

int plugin_set_short(char *plugin_name, char *variable_name, short value)
{
    int (*set_short)(char *name, short value);

    set_short = get_symbol_handle(plugin_name, "set_short");
    return (*set_short)(variable_name, value);
}

int plugin_set_int(char *plugin_name, char *variable_name, int value)
{
    int (*set_int)(char *name, int value);

    set_int = get_symbol_handle(plugin_name, "set_int");
    return (*set_int)(variable_name, value);
}

int plugin_set_float(char *plugin_name, char *variable_name, float value)
{
    int (*set_float)(char *name, float value);

    set_float = get_symbol_handle(plugin_name, "set_float");
    return (*set_float)(variable_name, value);
}

int plugin_set_string(char *plugin_name, char *variable_name, char *value)
{
    int (*set_string)(char *name, char *value);

    set_string = get_symbol_handle(plugin_name, "set_string");
    return (*set_string)(variable_name, value);
}

int plugin_run(char *plugin_name, char *command_name, int *return_value)
{
    int (*run)(char *name, int *return_value);

    run = get_symbol_handle(plugin_name, "run");
    return (*run)(command_name, return_value);
}

int plugin_describe(char *plugin_name, char *name, char *value)
{
    char *string;
    char * (*describe)(char *name);

    describe = get_symbol_handle(plugin_name, "describe");
    string = (*describe)(name);

    if (string == NULL)
        return -1;
    else
    {
        strcpy(value, string);
        return 0;
    }
}
