#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8888
#define SIZE 3

char board[SIZE][SIZE];

void init_board() {
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            board[i][j] = ' ';
}

int check_win(char p) {
    for (int i = 0; i < SIZE; i++) {
        if ((board[i][0] == p && board[i][1] == p && board[i][2] == p) ||
            (board[0][i] == p && board[1][i] == p && board[2][i] == p))
            return 1;
    }
    if ((board[0][0] == p && board[1][1] == p && board[2][2] == p) ||
        (board[0][2] == p && board[1][1] == p && board[2][2] == p))
        return 1;
    return 0;
}

int is_full() {
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (board[i][j] == ' ') return 0;
    return 1;
}

int main() {
    WSADATA wsa;
    SOCKET server_fd, client_sockets[2];
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);

    WSAStartup(MAKEWORD(2, 2), &wsa);
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, 2);

    printf("Server running... Waiting for 2 players to connect on port %d...\n", PORT);

    // Accept Player 1 ('X') and Player 2 ('O')
    client_sockets[0] = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    printf("Player 1 (X) connected.\n");
    send(client_sockets[0], "Connected as Player X. Waiting for Player O...\n", 47, 0);

    client_sockets[1] = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    printf("Player 2 (O) connected.\nGame Started!\n");
    send(client_sockets[1], "Connected as Player O.\n", 23, 0);

    init_board();
    int turn = 0; // 0 for Player X, 1 for Player O
    int row, col;
    char buffer[256];

    while (1) {
        SOCKET active_player = client_sockets[turn];
        SOCKET waiting_player = client_sockets[1 - turn];
        char symbol = (turn == 0) ? 'X' : 'O';

        // Broadcast current board state to both clients
        snprintf(buffer, sizeof(buffer),
                 "\n %c | %c | %c \n---|---|---\n %c | %c | %c \n---|---|---\n %c | %c | %c \n",
                 board[0][0], board[0][1], board[0][2],
                 board[1][0], board[1][1], board[1][2],
                 board[2][0], board[2][1], board[2][2]);

        send(client_sockets[0], buffer, strlen(buffer), 0);
        send(client_sockets[1], buffer, strlen(buffer), 0);

        // Notify active player to move
        send(active_player, "YOUR_TURN", 9, 0);
        send(waiting_player, "WAIT_TURN", 9, 0);

        // Receive move coordinates (row col) from active player
        int recv_size = recv(active_player, buffer, sizeof(buffer) - 1, 0);
        if (recv_size <= 0) break;
        buffer[recv_size] = '\0';

        sscanf(buffer, "%d %d", &row, &col);
        row--; col--;

        if (row < 0 || row >= SIZE || col < 0 || col >= SIZE || board[row][col] != ' ') {
            send(active_player, "INVALID", 7, 0);
            continue; // Retry turn
        }

        board[row][col] = symbol;

        // Check for Win or Draw
        if (check_win(symbol)) {
            snprintf(buffer, sizeof(buffer), "\nGAME_OVER Player %c Wins!\n", symbol);
            send(client_sockets[0], buffer, strlen(buffer), 0);
            send(client_sockets[1], buffer, strlen(buffer), 0);
            break;
        }

        if (is_full()) {
            send(client_sockets[0], "\nGAME_OVER It's a Draw!\n", 24, 0);
            send(client_sockets[1], "\nGAME_OVER It's a Draw!\n", 24, 0);
            break;
        }

        turn = 1 - turn; // Switch turn
    }

    closesocket(client_sockets[0]);
    closesocket(client_sockets[1]);
    closesocket(server_fd);
    WSACleanup();
    return 0;
}
