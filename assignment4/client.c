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
    int sock;
    struct sockaddr_in server_addr;

    char query[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1",
                  &server_addr.sin_addr) <= 0)
    {
        perror("Invalid server address");
        close(sock);
        exit(1);
    }

    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        exit(1);
    }

    printf("Connected to server.\n");

    while (1)
    {
        printf("\nEnter Registration Number (or bye): ");
        fgets(query, sizeof(query), stdin);

        query[strcspn(query, "\r\n")] = '\0';

        send(sock, query, strlen(query), 0);

        memset(response, 0, BUFFER_SIZE);

        int bytes_received = recv(sock, response,
                                  BUFFER_SIZE - 1, 0);

        if (bytes_received <= 0)
        {
            printf("Server disconnected.\n");
            break;
        }

        response[bytes_received] = '\0';

        printf("Server Output: %s\n", response);

        if (strcmp(query, "bye") == 0)
        {
            break;
        }
    }

    close(sock);

    printf("Connection closed.\n");

    return 0;
}
