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

#ifndef MESSAGE_H
#define MESSAGE_H

enum msg_type_t
{
    LIST_PLUGINS,
    PLUGIN_LOAD,
    PLUGIN_UNLOAD,
    PLUGIN_LIST_PROPERTIES,
    GET_CHAR,
    SET_CHAR,
    GET_SHORT,
    SET_SHORT,
    GET_INT,
    SET_INT,
    GET_LONG,
    SET_LONG,
    GET_FLOAT,
    SET_FLOAT,
    GET_DOUBLE,
    SET_DOUBLE,
    GET_STRING,
    SET_STRING,
    GET_DATA,
    SET_DATA,
    RUN,
    DESCRIBE,
    RSP_OK,
    RSP_ERROR,
};

int submit_message(int handle,
                 int type,
                 const char *name,
                 void *get_value,
                 void *set_value,
                 int set_value_size,
                 int timeout);

int handle_incoming_message(void);

struct message_io_t
{
    int (*write)(void *buffer, int length);
    int (*read)(void *buffer, int length);
    int (*close)(void);
};

int message_register_io(struct message_io_t *io);

#endif
