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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "testgear/options.h"
#include "testgear/debug.h"
#include "testgear/message.h"

int server_socket, client_socket;

void tcp_dump_data(void *data, int length)
{
    int i;
    char *bufferp;

    bufferp = data;
    (void)bufferp;

    for (i=0; i<length; i++)
    {
        if ((i%10 == 0) && (i !=0 ))
        {
            debug_printf_raw("\n");
            debug_printf("%32s"," ");
        }
        debug_printf_raw("0x%02x ", (unsigned char) bufferp[i]);
    }
}

int tcp_write(void *buffer, int length)
{
    int size;

    size = write(client_socket, buffer, length);

    // Debug
    debug_printf("Sending TCP data (%4d bytes):  ", size);
    tcp_dump_data(buffer, size);
    debug_printf_raw("\n");

    return size;
}

int tcp_read(void *buffer, int length)
{
    int size;

    size = read(client_socket, buffer, length);

    // Debug
    if (size)
    {
        debug_printf("Received TCP data (%4d bytes): ", size);
        tcp_dump_data(buffer, size);
        debug_printf_raw("\n");
    }

    return size;
}

int tcp_close(void)
{
    close(client_socket);
    return 0;
}

/*
 * tcp_server_start() - Starts TCP server
 *
 * This will listen for any incoming connections on provided port.
 * Receiving and sending of data will be performed by the test gear message
 * protocol handler ( handle_incoming_message() )
 */

void tcp_server_start(int port)
{
    int rc, length = sizeof(int);
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    // Create a reliable stream socket using TCP/IP
    if ((server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("Error: socket() call failed");
        exit (-1);
    }

    // Construct the server address structure
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    // Assign server address to socket
    if ((rc = bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address))) < 0)
    {
        perror("Error: bind() call failed");
        close(server_socket);
        exit(-1);
    }

    // Only allow 1 client to be connected at any time
    if((rc = listen(server_socket, 1)) < 0)
    {
        perror("Error: listen() call failed");
        close(server_socket);
        exit (-1);
    }

    debug_printf("Listening for incoming client connection on port %d...\n", port);

    // Wait and accept any incoming connection
    socklen_t sin_size = sizeof(struct sockaddr_in);
    if ((client_socket = accept(server_socket, (struct sockaddr *) &client_address, (socklen_t *) &sin_size)) < 0)
    {
        perror("Error: accept() call failed");
        close(server_socket);
        exit (-1);
    }

    debug_printf("Incoming connection from client (%s)\n", inet_ntoa(client_address.sin_addr));

    // Process incoming messages
    while (1)
        handle_incoming_message();
}
