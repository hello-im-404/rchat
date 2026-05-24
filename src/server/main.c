#include "server.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int port = 4040;
    if (argc == 2) {
        port = atoi(argv[1]);
        if (port <= 0) {
            printf("Invalid port\n");
            return 1;
        }
    } else {
        printf("Enter which port will listen server: ");
        if (scanf("%d", &port) != 1 || port < 1 || port > 65535) {
            printf("Invalid port\n");
            return 1;
        }
    }
    server_start(port);
    return 0;
}
