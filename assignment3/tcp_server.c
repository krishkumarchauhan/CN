#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8081
#define BUFFER_SIZE 1024

void transform_text(char *command, char *text)
{
    int i;
    int len;

    if (strcmp(command, "UP") == 0)
    {
        for (i = 0; text[i] != '\0'; i++)
        {
            text[i] = toupper((unsigned char)text[i]);
        }
    }
    else if (strcmp(command, "LOW") == 0)
    {
        for (i = 0; text[i] != '\0'; i++)
        {
            text[i] = tolower((unsigned char)text[i]);
        }
    }
    else if (strcmp(command, "REV") == 0)
    {
        len = strlen(text);

        for (i = 0; i < len / 2; i++)
        {
            char temp = text[i];
            text[i] = text[len - i - 1];
            text[len - i - 1] = temp;
        }
    }
}

int main()
{
    int server_fd;
    int client_socket;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t client_len;

    char buffer[BUFFER_SIZE];

    /* Create TCP socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    /* Allow port reuse */
    int opt = 1;

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        close(server_fd);
        return 1;
    }

    /* Set server address */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    /* Bind */
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    /* Listen */
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    printf("Server started...\n");
    printf("Listening on port %d...\n", PORT);

    /* Accept client */
    client_len = sizeof(client_addr);

    client_socket = accept(server_fd,
                           (struct sockaddr *)&client_addr,
                           &client_len);

    if (client_socket < 0)
    {
        perror("Accept failed");
        close(server_fd);
        return 1;
    }

    printf("Client connected.\n");

    /* Communication loop */
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));

        int received = recv(client_socket,
                            buffer,
                            BUFFER_SIZE - 1,
                            0);

        if (received < 0)
        {
            perror("Receive failed");
            break;
        }

        if (received == 0)
        {
            printf("Client disconnected.\n");
            break;
        }

        buffer[received] = '\0';

        printf("Request: %s\n", buffer);

        /* Check for bye */
        if (strcmp(buffer, "bye") == 0)
        {
            printf("Client sent bye.\n");
            break;
        }

        /* Find | */
        char *separator = strchr(buffer, '|');

        if (separator == NULL)
        {
            char error_message[] = "Invalid format. Use COMMAND|TEXT";

            send(client_socket,
                 error_message,
                 strlen(error_message),
                 0);

            continue;
        }

        /* Separate command and text */
        *separator = '\0';

        char *command = buffer;
        char *text = separator + 1;

        /* Check command */
        if (strcmp(command, "UP") != 0 &&
            strcmp(command, "LOW") != 0 &&
            strcmp(command, "REV") != 0)
        {
            char error_message[] = "Invalid command";

            send(client_socket,
                 error_message,
                 strlen(error_message),
                 0);

            continue;
        }

        /* Transform text */
        transform_text(command, text);

        /* Send response */
        if (send(client_socket,
                 text,
                 strlen(text),
                 0) < 0)
        {
            perror("Send failed");
            break;
        }

        printf("Response: %s\n", text);
    }

    close(client_socket);
    close(server_fd);

    printf("Server closed.\n");

    return 0;
}
