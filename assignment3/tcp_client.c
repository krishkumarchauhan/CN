#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int sock;

    struct sockaddr_in server_addr;

    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    char *server_ip;

    /* Default IP = localhost */
    server_ip = "127.0.0.1";

    /* If IP is provided through command line */
    if (argc == 2)
    {
        server_ip = argv[1];
    }

    /* Create TCP socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    /* Clear server address */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    /* Convert IP address */
    if (inet_pton(AF_INET,
                  server_ip,
                  &server_addr.sin_addr) <= 0)
    {
        printf("Invalid IP address: %s\n", server_ip);
        close(sock);
        return 1;
    }

    /* Connect to server */
    if (connect(sock,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    printf("Connected to server %s:%d\n",
           server_ip, PORT);

    printf("\n");
    printf("Available commands:\n");
    printf("UP|text   - Convert text to uppercase\n");
    printf("LOW|text  - Convert text to lowercase\n");
    printf("REV|text  - Reverse text\n");
    printf("bye       - Close connection\n");
    printf("\n");

    /* Communication loop */
    while (1)
    {
        printf("Enter request: ");

        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
        {
            break;
        }

        /* Remove newline */
        buffer[strcspn(buffer, "\n")] = '\0';

        /* Ignore empty input */
        if (strlen(buffer) == 0)
        {
            continue;
        }

        /* Send request */
        if (send(sock,
                 buffer,
                 strlen(buffer),
                 0) < 0)
        {
            perror("Send failed");
            break;
        }

        /* Check bye */
        if (strcmp(buffer, "bye") == 0)
        {
            printf("Closing connection...\n");
            break;
        }

        /* Receive response */
        memset(response, 0, sizeof(response));

        int received = recv(sock,
                            response,
                            BUFFER_SIZE - 1,
                            0);

        if (received < 0)
        {
            perror("Receive failed");
            break;
        }

        if (received == 0)
        {
            printf("Server disconnected.\n");
            break;
        }

        response[received] = '\0';

        printf("Server response: %s\n\n", response);
    }

    close(sock);

    printf("Client closed.\n");

    return 0;
}
