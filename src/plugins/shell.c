/*
 * Shell command plugin for Test Gear
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
    {   .name = "command",
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
    .license = "Unknown",
    .commands = shell_commands,
    .vars = shell_vars,
};


plugin_register(shell);
