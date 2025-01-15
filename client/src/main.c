#include "global.h"

int main (int argc, char** argv) {

    char path[BUFSIZ*2]; memset(path, 0, BUFSIZ*2);
    char action[BUFSIZ]; memset(action, 0, BUFSIZ);
    if (argc > 1) {
        strcpy(action, argv[1]);
        printf("action : %s\n", action);
    }

    char filename[BUFSIZ]; memset(filename, 0, BUFSIZ);
    if (argc > 2) {
        strcpy(filename, argv[2]);
        printf("filename : %s\n", filename);
    }
    // filename[strlen(filename)-1] = 0;

    /* Initialisation du socket, bind et connect */
    int client_fd = socket(AF_INET, SOCK_STREAM, 0); perror("socket ");
        if (client_fd == -1) return EXIT_FAILURE;

    // struct sockaddr_in client_addr = {
    //     .sin_addr.s_addr = INADDR_ANY,
    //     .sin_family = AF_INET,
    //     .sin_port = htons(CLIENT_PORT)
    // };

    // int error = bind(client_fd, (struct sockaddr*)&client_addr, sizeof(client_addr)); perror("bind ");
    // if (error == -1 ) return EXIT_FAILURE;

    struct sockaddr_in server_addr = {
        .sin_addr.s_addr = INADDR_ANY,
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT)
    }; 

    int error = connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)); perror("connect ");
        if (error == -1) return EXIT_FAILURE;

    char welcome[BUFSIZ]; memset(welcome, 0 , BUFSIZ);
    error = recv(client_fd, welcome, sizeof(welcome)-1, 0); perror("welcome ");
        if (error == -1) { close(client_fd); return EXIT_FAILURE; }
    printf("%s\n", welcome);

    error = send(client_fd, action, BUFSIZ, 0); perror("send action()");
        if (error == -1) { close(client_fd); return EXIT_FAILURE; }
        printf("action : %s\n", action);

    if (strcasecmp(action, "list") == 0) {
        list (error, client_fd);
    } else if (strcasecmp(action, "download") == 0) {
        download(filename, client_fd);
    }  else if (strcasecmp(action, "upload") == 0) {
        upload(filename, client_fd);      
    }

    return 0;
}