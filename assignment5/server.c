#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8081
#define BUFFER_SIZE 1024

void analyze_text(char *text, char *response)
{
    int chars = 0, words = 0, vowels = 0;
    int in_word = 0;

    for (int i = 0; text[i] != '\0'; i++)
    {
        chars++;

        if (text[i] == ' ' || text[i] == '\t' || text[i] == '\n')
        {
            in_word = 0;
        }
        else
        {
            if (in_word == 0)
            {
                words++;
                in_word = 1;
            }
        }

        if (tolower((unsigned char)text[i]) == 'a' ||
            tolower((unsigned char)text[i]) == 'e' ||
            tolower((unsigned char)text[i]) == 'i' ||
            tolower((unsigned char)text[i]) == 'o' ||
            tolower((unsigned char)text[i]) == 'u')
        {
            vowels++;
        }
    }

    sprintf(response,
            "Chars = %d , Words = %d , Vowels = %d",
            chars, words, vowels);
}

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);

    client_fd = accept(server_fd,
                       (struct sockaddr *)&client_addr,
                       &client_len);

    if (client_fd < 0)
    {
        perror("Accept failed");
        close(server_fd);
        return 1;
    }

    printf("Client connected.\n");

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        memset(response, 0, sizeof(response));

        int bytes_received = recv(client_fd,
                                  buffer,
                                  sizeof(buffer) - 1,
                                  0);

        if (bytes_received <= 0)
        {
            break;
        }

        buffer[bytes_received] = '\0';

        printf("Received: %s\n", buffer);

        if (strcmp(buffer, "bye") == 0)
        {
            printf("Client ended the session.\n");
            break;
        }

        /*
         * Expected format:
         * ANALYZE|TEXT
         */

        char *delimiter = strchr(buffer, '|');

        if (delimiter == NULL)
        {
            strcpy(response, "Invalid format. Use ANALYZE|TEXT");
        }
        else
        {
            *delimiter = '\0';

            char *command = buffer;
            char *text = delimiter + 1;

            if (strcmp(command, "ANALYZE") == 0)
            {
                analyze_text(text, response);
            }
            else
            {
                strcpy(response, "Invalid command. Use ANALYZE|TEXT");
            }
        }

        send(client_fd, response, strlen(response), 0);
    }

    close(client_fd);
    close(server_fd);

    return 0;
}
