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
#include <unistd.h>
#include "testgear/debug.h"
#include "testgear/message.h"
#include "testgear/tcp.h"
#ifdef SERVER
#include "testgear/plugin-manager.h"
#else
#include "testgear/testgear.h"
#endif

/*
 * === Test Gear message protocol ===
 *
 * Message format (bytes):
 *  message[0]    = prefix (0xBD)
 *  message[1-4]  = ID (32 bit)
 *  message[5]    = type
 *  message[6-9]  = payload length (32 bit)
 *  message[10-*] = payload
 * 
 * Possible message types include:
 *  LIST_PLUGINS,
 *  PLUGIN_LOAD, PLUGIN_UNLOAD,
 *  PLUGIN_LIST_PROPERTIES,
 *  GET_CHAR, GET_SHORT, GET_INT, GET_FLOAT,
 *  SET_CHAR, SET_SHORT, SET_INT, SET_FLOAT,
 *  RSP_OK, RSP_ERROR
 *
 * Payload format depends on message type:
 *  LIST_PLUGINS:
 *   No payload (payload length = 0)
 *  PLUGIN_LOAD, PLUGIN_UNLOAD, PLUGIN_LIST_PROPERTIES:
 *   payload[0]   = plugin name length
 *   payload[1-*] = plugin name
 *  GET_CHAR, GET_SHORT, GET_INT, GET_FLOAT, GET_STRING:
 *   payload[0]   = variable name length
 *   payload[1-*] = variable name
 *  SET_CHAR:
 *   payload[0]   = variable name length
 *   payload[1-*] = variable name
 *   payload[*-*] = value (1 byte)
 *  SET_SHORT:
 *   payload[0]   = variable name length
 *   payload[1-*] = variable name
 *   payload[*-*] = value (2 bytes)
 *  SET_INT:
 *   payload[0]   = variable name length
 *   payload[1-*] = variable name
 *   payload[*-*] = value (4 bytes)
 *  SET_FLOAT:
 *   payload[0]   = variable name length
 *   payload[1-*] = variable name
 *   payload[*-*] = value (4 bytes)
 *  SET_STRING:
 *   payload[0]   = variable name length
 *   payload[1-*] = variable name
 *   payload[*]   = string length (1 byte)
 *   payload[*-*] = string data
 *  RUN:
 *   payload[0]   = function name length
 *   payload[1-*] = function name
 *  DESCRIBE:
 *   payload[0]   = name length
 *   payload[1-*] = name
 *  RSP_OK, RSP_ERROR:
 *   payload[0-3] = response data length
 *   payload[4-*] = response data
 *
 *  Data formats for RSP_OK response:
 *  (LIST_PLUGINS):
 *   data[0-N] = plugins string (N bytes)
 *  (PLUGIN_LIST_PROPERTIES):
 *   data[0-N] = plugin properties string (N bytes)
 *  (GET_CHAR):
 *   data[0] = value (1 byte)
 *  (GET_SHORT):
 *   data[0-1] = value (2 bytes)
 *  (GET_INT):
 *   data[0-3] = value (4 bytes)
 *  (GET_FLOAT):
 *   data[0-4] = value (4 bytes)
 *  (GET_STRING):
 *   data[0-N] = string (N bytes)
 *  (RUN):
 *   data[0-3] = function return value (4 bytes)
 *
 *  Data format for RSP_ERROR response:
 *   data[0-N] = error string (N bytes)
 *
 *  RSP_ERROR only relates to Test Gear errors ("plugin not found", "variable
 *  not found", "out of memory", etc.)
 */

#define MSG_PREFIX 0xBD // (binary: 10111101)
#define MSG_HEADER_SIZE 10
#define MSG_NAME_LENGTH_MAX 256

static unsigned int message_counter = 0;

struct __attribute__((__packed__)) msg_header_t
{
   unsigned char prefix;
   unsigned int id;
   unsigned char type;
   unsigned int payload_length;
   char payload; // Fake payload item (for reference only)
};

static int value_size(int command)
{ 
    switch (command)
    {
        case GET_CHAR:
            return sizeof(char);
        case GET_SHORT:
            return sizeof(short);
        case GET_INT:
            return sizeof(int);
        case GET_FLOAT:
            return sizeof(float);
        default:
            return 0;
            break;
    }

    return 0;
} 

static int create_message(void **msg_buffer,
                   char type,
                   const char *name,
                   void *value,
                   int value_length,
                   unsigned int *id)
{
    struct msg_header_t *message;
    unsigned char name_length = 0;
    int msg_length;
    char *payload;

    if (name != NULL)
        name_length = strlen(name);
    else
        name_length = 0;

    if ((type != RSP_OK) || (type != RSP_ERROR))
    {
        // Verify length of name
        if (name_length > MSG_NAME_LENGTH_MAX)
        {
            printf("Error: Length of name must not exceed %d bytes\n", MSG_NAME_LENGTH_MAX);
            return -1;
        }
    }

    // Allocate memory for message buffer (worst case)
    *msg_buffer = malloc(MSG_HEADER_SIZE + 1 + name_length + 4 + value_length);
    if (msg_buffer == NULL)
    {
        perror("Error: malloc() failed");
        return -1;
    }

    // Create message header
    message         = *msg_buffer;
    message->prefix = MSG_PREFIX;
    message->id     = message_counter;
    message->type   = type;
    
    payload = &message->payload;

    switch (type)
    {
        case LIST_PLUGINS:
            message->payload_length = 0;
            break;
        case PLUGIN_LOAD:
        case PLUGIN_UNLOAD:
        case PLUGIN_LIST_PROPERTIES:
        case GET_CHAR:
        case GET_SHORT:
        case GET_INT:
        case GET_FLOAT:
        case GET_STRING:
        case GET_DATA:
        case RUN:
        case DESCRIBE:
            payload[0] = name_length;
            strcpy(&payload[1], name);
            message->payload_length = 1 + name_length;
            break;
        case SET_CHAR:
        case SET_SHORT:
        case SET_INT:
        case SET_FLOAT:
            payload[0] = name_length;
            strcpy(&payload[1], name);
            memcpy(&payload[1+name_length], value, value_length);
            message->payload_length = 1 + name_length + value_length;
            break;
        case SET_STRING:
            payload[0] = name_length;
            strcpy(&payload[1], name);
            payload[1+name_length] = value_length;
            memcpy(&payload[1+name_length+1], value, value_length);
            message->payload_length = 1 + name_length + 1 + value_length;
            break;
        case RSP_OK:
        case RSP_ERROR:
            memcpy(&payload[0], value, value_length);
            message->payload_length = value_length;
            break;
        case SET_DATA:
        default:
            printf("Error: Unknown message type\n");
            exit(-1);
            break;
    }
 
    msg_length = MSG_HEADER_SIZE + message->payload_length;

    // Return message ID
    *id = message_counter;

    // Increase message counter
    message_counter++;

    // Return length of contructed message
    return msg_length;
}

static int verify_request(void *msg_buffer)
{
    struct msg_header_t *message;
    message = msg_buffer;

    // Verify message prefix
    if (message->prefix != MSG_PREFIX)
    {
        printf("Error: Received invalid request message (invalid prefix)\n");
        return -1;
    }

    return 0;
}

static int verify_response(void *msg_buffer, unsigned int id)
{
    struct msg_header_t *message;
    message = msg_buffer;

    // Verify message prefix
    if (message->prefix != MSG_PREFIX)
    {
        printf("Error: Received invalid response message (invalid prefix)\n");
        return -1;
    }

    // Verify that we are receiving a response type message
    if ((message->type != RSP_OK) && (message->type != RSP_ERROR))
    {
        printf("Error: Received invalid response message (invalid response type)\n");
        return -1;
    }

    // Verify response id
    if (message->id != id)
    {
        printf("Error: Received invalid response message (invalid response id)\n");
        return -1;
    }

    return 0;
}

static int decode_value(void *payload, int payload_size, char type, void *value)
{
    char *p;

    switch (type)
    {
        case LIST_PLUGINS:
        case PLUGIN_LIST_PROPERTIES:
            memcpy(value, payload, payload_size);
            p = payload;
            p[payload_size]=0;
            break;
        case PLUGIN_LOAD:
        case PLUGIN_UNLOAD:
        case GET_CHAR:
            memcpy(value, payload, sizeof(char));
            break;
        case GET_SHORT:
            memcpy(value, payload, sizeof(short));
            break;
        case GET_INT:
            memcpy(value, payload, sizeof(int));
            break;
        case GET_FLOAT:
            memcpy(value, payload, sizeof(float));
            break;
        case GET_STRING:
            memcpy(value, payload, payload_size);
            p = payload;
            p[payload_size]=0;
            break;
        case DESCRIBE:
            memcpy(value, payload, payload_size);
            p = payload;
            p[payload_size]=0;
            break;
        case RUN:
            memcpy(value, payload, sizeof(int));
            break;
        default:
            // Do nothing (no value returned for set commands)
            break;
    }

    return 0;
}

static int decode_name(void *payload, void *name)
{
    unsigned char length;
    char *p, *n;

    p = payload;
    length = p[0];

    memcpy(name, &p[1], length);

    // Terminate name string
    n = name;
    n[length] = 0;

    return 0;
}

#ifdef DEBUG
static char *message_type(type)
{
    switch (type)
    {
        case LIST_PLUGINS:
            return "LIST_PLUGINS";
        case PLUGIN_LOAD:
            return "PLUGIN_LOAD";
        case PLUGIN_UNLOAD:
            return "PLUGIN_UNLOAD";
        case PLUGIN_LIST_PROPERTIES:
            return "PLUGIN_LIST_PROPERTIES";
        case GET_CHAR:
            return "GET_CHAR";
        case GET_SHORT:
            return "GET_SHORT";
        case GET_INT:
            return "GET_INT";
        case GET_FLOAT:
            return "GET_FLOAT";
        case GET_STRING:
            return "GET_STRING";
        case GET_DATA:
            return "GET_DATA";
        case SET_CHAR:
            return "SET_CHAR";
        case SET_SHORT:
            return "SET_SHORT";
        case SET_INT:
            return "SET_INT";
        case SET_FLOAT:
            return "SET_FLOAT";
        case SET_STRING:
            return "SET_STRING";
        case SET_DATA:
            return "SET_DATA";
        case RUN:
            return "RUN";
        case DESCRIBE:
            return "DESCRIBE";
        case RSP_OK:
            return "RSP_OK";
        case RSP_ERROR:
            return "RSP_ERROR";
        default:
            break;
    }
    return "unknown";
}
#endif

#ifndef SERVER
int submit_message(int handle,
                   int type,
                   const char *name,
                   void *get_value,
                   void *set_value,
                   int set_value_size,
                   int timeout)
{
    struct msg_header_t msg_header;
    char *response_message;
    char *message;
    char *payload;
    int length;
    unsigned int id;
    int ret;

    // Create request message
    length = create_message( (void *) &message, type, name, set_value, set_value_size, &id);
    if (length < 0)
        return -1;

    debug_printf("Sending %s (%x) message with ID %d\n", message_type(type), type, id);

    // Send request message
    ret = tcp_write(message, length);
    if (ret < 0 )
    {
        return -1;
    }

    // Free request message buffer
    free(message);

    // Receive response message header
    if (tcp_read(&msg_header, MSG_HEADER_SIZE) == 0)
    {
        printf("Server closed connection1\n");
        close(server_socket);
        exit(-1);
    }

    // Verify response message
    verify_response(&msg_header, id);

    if (msg_header.payload_length > 0)
    {
        // Allocate memory for payload buffer
        payload = malloc(msg_header.payload_length + 1);
        if (payload == NULL)
        {
            perror("Error: malloc() failed");
            return -1;
        }

        // Receive payload
        if (tcp_read(payload, msg_header.payload_length) == 0)
        {
            printf("Server closed connection2\n");
            close(server_socket);
            free(payload);
            exit(-1);
        }

        if (msg_header.type == RSP_OK)
        {
            // Extract value from response message
            decode_value(payload, msg_header.payload_length, type, get_value);
        }

        free(payload);

        if (msg_header.type == RSP_ERROR)
        {
            // Payload is the error string
            tg_error = payload;
            return -1;
        }
    }

    return 0;
}

#endif

#ifdef SERVER

int decode_tg_string(char *string, char *plugin, char *variable)
{
    int i;

    // Find first occurence of '.'
    for (i=0; i<strlen(string); i++)
    {
        if (string[i] == '.')
            break;
    }

    // Return plugin name and variable name
    if (i<strlen(string))
    {
        memcpy(plugin, string, i);
        plugin[i] = 0;
        strcpy(variable, &string[i+1]);
    }
    else
    {
        memcpy(plugin, string, i);
        plugin[i] = 0;
        variable[0] = 0;
        return -1;
    }

    return 0;
}

int handle_incoming_message(int server_socket)
{
    struct msg_header_t msg_header;
    unsigned int id;
    char *response_message;
    char *payload;
    int length, ret;
    char name[MSG_NAME_LENGTH_MAX];
    int response_type;
    int response_size = 0;
    char response_value[65536] = "";
    char plugin_name[256] = "";
    char variable_name[256] = "";

    /* 1. Receive message (blocking)
     * 1.1 Receive header length
     * 1.2 Decode payload length
     * 1.3 Allocate payload length memory
     * 1.4 Receive payload length
     * 2. Decode message
     * 3. Execute request
     * 4. Send response (blocking)
     */

    // Receive message header
    if (tcp_read(&msg_header, MSG_HEADER_SIZE) == 0)
    {
        printf("Client closed connection\n");
        close(client_socket);
        exit(-1);
    }

    // Verify message header
    verify_request(&msg_header);

    if (msg_header.type != LIST_PLUGINS)
    {
        // Allocate memory for payload buffer
        payload = malloc(msg_header.payload_length);
        if (payload == NULL)
        {
            perror("Error: malloc() failed");
            return -1;
        }

        // Receive payload
        if (tcp_read(payload, msg_header.payload_length) == 0)
        {
            printf("Client closed connection\n");
            close(client_socket);
            free(payload);
            exit(-1);
        }
    }

    debug_printf("Received message (id = %d, type = %s, payload size = %d)\n", msg_header.id, message_type(msg_header.type), msg_header.payload_length);

    if (msg_header.type != LIST_PLUGINS)
    {
        // Decode name part of payload (eg. "fb.xres")
        decode_name(payload, &name);
    }

    // For all message types except plugin load/unload/list
    if ((msg_header.type != PLUGIN_LOAD) && 
        (msg_header.type != PLUGIN_UNLOAD) &&
        (msg_header.type != LIST_PLUGINS))
    {
        // Decode plugin name and variable name
        decode_tg_string(name, (char *) &plugin_name, (char *) &variable_name);
    }

    // Decode message and execute request
    switch (msg_header.type)
    {
        case LIST_PLUGINS:
            debug_printf("LIST_PLUGINS()\n");
            if (list_plugins((char *) &response_value))
            {
                response_type = RSP_ERROR;
                sprintf(response_value, "Failed to list plugins");
                response_size = strlen(response_value) + 1;
            }
            else
            {
                response_type = RSP_OK;
                response_size = strlen(response_value);
            }
            break;

        case PLUGIN_LOAD:
            debug_printf("PLUGIN_LOAD(%s)\n", name);
            if (plugin_load(name))
            {
                response_type = RSP_ERROR;
                sprintf(response_value, "Failed to load %s plugin", name);
                response_size = strlen(response_value) + 1;
            }
            else
                response_type = RSP_OK;
            break;
        case PLUGIN_UNLOAD:
            debug_printf("PLUGIN_UNLOAD(%s)\n", name);
            if (plugin_unload(name))
            {
                response_type = RSP_ERROR;
                sprintf(response_value, "Failed to unload %s plugin", name);
                response_size = strlen(response_value) + 1;
            }
            else
                response_type = RSP_OK;
            break;
        case PLUGIN_LIST_PROPERTIES:
            debug_printf("PLUGIN_LIST_PROPERTIES()\n");
            if (plugin_list_properties(plugin_name, (char *) &response_value))
            {
                response_type = RSP_ERROR;
                sprintf(response_value, "Failed to list plugin properties");
                response_size = strlen(response_value) + 1;
            }
            else
            {
                response_type = RSP_OK;
                response_size = strlen(response_value);
            }
            break;
        case GET_CHAR:
            debug_printf("GET_CHAR(%s)\n", name);
            if (plugin_get_char(plugin_name, variable_name, (char *) response_value) == 0)
            {
                response_type = RSP_OK;
                response_size = sizeof(char);
            }
            else
            {
                response_type = RSP_ERROR;
                sprintf(response_value, "Variable %s of char type not found", name);
                response_size = strlen(response_value) + 1;
            }
            break;
        case GET_SHORT:
            debug_printf("GET_SHORT(%s)\n", name);
            if (plugin_get_short(plugin_name, variable_name, (short *) response_value) == 0)
            {
                response_type = RSP_OK;
                response_size = sizeof(short);
            }
            else
            {
                response_type = RSP_ERROR;
                sprintf(response_value, "Variable %s of short type not found", name);
                response_size = strlen(response_value) + 1;
            }
            break;
        case GET_INT:
            debug_printf("GET_INT(%s)\n", name);
            if (plugin_get_int(plugin_name, variable_name, (int *) response_value) == 0)
            {
                response_type = RSP_OK;
                response_size = sizeof(int);
            }
            else
            {
                response_type = RSP_ERROR;
                sprintf(response_value, "Variable %s of int type not found", name);
                response_size = strlen(response_value) + 1;
            }
            break;
        case GET_FLOAT:
            debug_printf("GET_SHORT(%s)\n", name);
            if (plugin_get_float(plugin_name, variable_name, (float *) response_value) == 0)
            {
                response_type = RSP_OK;
                response_size = sizeof(float);
            }
            else
            {
                response_type = RSP_ERROR;
                sprintf(response_value, "Variable %s of float type not found", name);
                response_size = strlen(response_value) + 1;
            }
            break;
        case GET_STRING:
            debug_printf("GET_STRING(%s)\n", name);
            if (plugin_get_string(plugin_name, variable_name, (char *) &response_value) == 0)
                response_type = RSP_OK;
            else
            {
                response_type = RSP_ERROR;
                sprintf(response_value, "Variable %s of string type not found", name);
            }
            response_size = strlen(response_value) + 1;
            break;
        case GET_DATA:
            break;
        case SET_CHAR:
            debug_printf("SET_CHAR(%s)\n", name);
            char *char_value = (char *) &payload[strlen(name)+1];
            if (plugin_set_char(plugin_name, variable_name, *char_value) == 0)
                response_type = RSP_OK;
            else
            {
                response_type = RSP_ERROR;
                sprintf(response_value, "Variable %s of char type not found", name);
                response_size = strlen(response_value) + 1;
            }
            break;
        case SET_SHORT:
            debug_printf("SET_SHORT(%s)\n", name);
            short *short_value = (short *) &payload[strlen(name)+1];
            if (plugin_set_short(plugin_name, variable_name, *short_value) == 0)
                response_type = RSP_OK;
            else
            {
                response_type = RSP_ERROR;
                sprintf(response_value, "Variable %s of char type not found", name);
                response_size = strlen(response_value) + 1;
            }
            break;
        case SET_INT:
            debug_printf("SET_INT(%s)\n", name);
            int *int_value = (int *) &payload[strlen(name)+1];
            if (plugin_set_int(plugin_name, variable_name, *int_value) == 0)
                response_type = RSP_OK;
            else
            {
                response_type = RSP_ERROR;
                sprintf(response_value, "Variable %s of int type not found", name);
                response_size = strlen(response_value) + 1;
            }
            break;
        case SET_FLOAT:
            debug_printf("SET_FLOAT(%s)\n", name);
            float *float_value = (float *) &payload[strlen(name)+1];
            if (plugin_set_float(plugin_name, variable_name, *float_value) == 0)
                response_type = RSP_OK;
            else
            {
                response_type = RSP_ERROR;
                sprintf(response_value, "Variable %s of int type not found", name);
                response_size = strlen(response_value) + 1;
            }
            break;
        case SET_STRING:
            debug_printf("SET_STRING(%s)\n", name);
            char *string_value = (char *) &payload[1+strlen(name)+1];
            if (plugin_set_string(plugin_name, variable_name, string_value) == 0)
                response_type = RSP_OK;
            else
            {
                response_type = RSP_ERROR;
                sprintf(response_value, "Variable %s of string type not found", name);
                response_size = strlen(response_value) + 1;
            }
            break;
        case SET_DATA:
            break;
        case RUN:
            debug_printf("RUN(%s)\n", name);
            if (plugin_run(plugin_name, variable_name, (int *) response_value) == 0)
            {
                response_type = RSP_OK;
                response_size = sizeof(int);
            }
            else
            {
                response_type = RSP_ERROR;
                sprintf(response_value, "Command %s not found", name);
                response_size = strlen(response_value) + 1;
            }
            break;
        case DESCRIBE:
            debug_printf("DESCRIBE(%s)\n", name);
            if (plugin_describe(plugin_name, variable_name, (char *) &response_value) == 0)
                response_type = RSP_OK;
            else
            {
                response_type = RSP_ERROR;
                sprintf(response_value, "Variable or command %s not found", name);
            }
            response_size = strlen(response_value) + 1;
            break;
         default:
            break;
    }

    // Free payload memory
    if (msg_header.type != LIST_PLUGINS)
        free(payload);

    // Create response message
    length = create_message( (void *) &response_message, response_type, NULL, response_value, response_size, &id);
    if (length < 0)
        return -1;

    debug_printf("Sending %s (%x) message with ID %d\n", message_type(response_type), response_type, id);

    // Send response message
    ret = tcp_write(response_message, length);
    if (ret < 0 )
        return -1;

    return 0;
}

#endif // SERVER
