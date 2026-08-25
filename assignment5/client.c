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

    char input[BUFFER_SIZE];
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
    {
        perror("Invalid server address");
        close(sock);
        return 1;
    }

    if (connect(sock,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    printf("Connected to server.\n");
    printf("Enter text to analyze.\n");
    printf("Type bye to exit.\n\n");

    while (1)
    {
        printf("Enter text: ");

        fgets(input, sizeof(input), stdin);

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "bye") == 0)
        {
            send(sock, "bye", 3, 0);
            break;
        }

        /*
         * Create request:
         * ANALYZE|TEXT
         */
        snprintf(request, sizeof(request), "ANALYZE|%s", input);

        send(sock, request, strlen(request), 0);

        memset(response, 0, sizeof(response));

        int bytes_received = recv(sock,
                                  response,
                                  sizeof(response) - 1,
                                  0);

        if (bytes_received <= 0)
        {
            printf("Server disconnected.\n");
            break;
        }

        response[bytes_received] = '\0';

        printf("Server Response: %s\n\n", response);
    }

    close(sock);

    printf("Client closed.\n");

    return 0;
}
