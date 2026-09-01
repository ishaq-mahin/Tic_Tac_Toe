#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8888
#define SERVER_IP "127.0.0.1"

int main()
{
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server_addr;
    char buffer[512];

    WSAStartup(MAKEWORD(2, 2), &wsa);
    sock = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr))< 0)
    {
        printf("Connection failed! Make sure the server is running.\n");
        return 1;
    }

    printf("Connected to server!\n");

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        int recv_size = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (recv_size <= 0) break;

        buffer[recv_size] = '\0';

        if (strstr(buffer, "GAME_OVER"))
        {
            printf("%s\n", buffer);
            break;
        }

        // Detect YOUR_TURN even if combined with board rendering data
        if (strstr(buffer, "YOUR_TURN"))
        {
            char *turn_signal = strstr(buffer, "YOUR_TURN");
            *turn_signal = '\0'; // Cut off signal to print board nicely
            if (strlen(buffer) > 0)
            {
                printf("%s", buffer);
            }

            int row, col;
            printf("Your Turn! Enter move (row 1-3 and col 1-3): ");

            // Validate stdin numbers
            if (scanf("%d %d", &row, &col) != 2)
            {
                while (getchar() != '\n'); // Clear invalid input buffer
                row = 0;
                col = 0;           // Force server retry logic
            }

            snprintf(buffer, sizeof(buffer), "%d %d", row, col);
            send(sock, buffer, strlen(buffer), 0);
        }
        else if (strstr(buffer, "WAIT_TURN"))
        {
            char *wait_signal = strstr(buffer, "WAIT_TURN");
            *wait_signal = '\0';
            if (strlen(buffer) > 0)
            {
                printf("%s", buffer);
            }
            printf("Opponent's turn. Waiting...\n");
        }
        else
        {
            printf("%s", buffer);
        }
    }
    closesocket(sock);
    WSACleanup();

    // Window will now freeze here instead of instantly closing neither deadlock occurs
    printf("\nGame has ended.\n");
    system("pause");
    return 0;
}
