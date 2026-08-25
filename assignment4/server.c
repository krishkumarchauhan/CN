#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    client_fd = accept(server_fd,
                       (struct sockaddr *)&client_addr,
                       &client_len);

    if (client_fd < 0)
    {
        perror("Accept failed");
        close(server_fd);
        exit(1);
    }

    printf("Client connected.\n");

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        int bytes_received = recv(client_fd, buffer,
                                  BUFFER_SIZE - 1, 0);

        if (bytes_received <= 0)
        {
            printf("Client disconnected.\n");
            break;
        }

        buffer[strcspn(buffer, "\r\n")] = '\0';

        if (strcmp(buffer, "bye") == 0)
        {
            printf("Client requested termination.\n");

            send(client_fd, "Connection terminated", 21, 0);
            break;
        }

        printf("Query received: %s\n", buffer);

        FILE *file = fopen("students.txt", "r");

        if (file == NULL)
        {
            send(client_fd, "Database file not found", 24, 0);
            continue;
        }

        char line[BUFFER_SIZE];
        char regno[100];
        char fname[100];
        char branch[100];
        char college[200];

        int found = 0;

        while (fgets(line, sizeof(line), file))
        {
            line[strcspn(line, "\r\n")] = '\0';

            if (sscanf(line, "%99[^|]|%99[^|]|%99[^|]|%199[^\n]",
                       regno, fname, branch, college) == 4)
            {
                if (strcmp(regno, buffer) == 0)
                {
                    char response[BUFFER_SIZE];

                    snprintf(response, sizeof(response),
                             "Name: %s, Branch: %s, College: %s",
                             fname, branch, college);

                    send(client_fd, response,
                         strlen(response), 0);

                    found = 1;
                    break;
                }
            }
        }

        fclose(file);

        if (!found)
        {
            char response[] = "Data not found";
            send(client_fd, response, strlen(response), 0);
        }
    }

    close(client_fd);
    close(server_fd);

    printf("Server terminated.\n");

    return 0;
}
