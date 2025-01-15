#include "global.h"

int main () {

    char filename[BUFSIZ]; memset(filename, 0, BUFSIZ);
    /* Initialisation du socket (socket, bind, listen) */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); perror("socket ");
        if (server_fd == -1) return EXIT_FAILURE;

    struct sockaddr_in server_addr = {
        .sin_addr.s_addr = INADDR_ANY,
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT)
    };

    // Reutilise le meme port en empechant le bind : already in use
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
        perror("setsockpopt(SO_REUSEADDR) failed ");

    #ifdef SO_REUSEPORT
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0)
           perror("setsockopt(SO_REUSEPORT) failed ");
    #endif

    int error = bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)); perror("bind ");
        if (error == -1) return EXIT_FAILURE;

    error = listen(server_fd, 0); perror("listen ");
        if (error == - 1) return EXIT_FAILURE;
    printf("Server listen in port: %d\n", SERVER_PORT);

    while (1){
    /* Attente de la connexion client */
    struct sockaddr_in client_addr;
    socklen_t len;
    printf("En attente de connexion du client...\n");
    int client_fd = accept(server_fd,(struct sockaddr*)&client_addr,&len); perror("accept ");
        if(client_fd == -1) {close(server_fd); return EXIT_FAILURE;}
    
    /* Client connecté, Envoi de message de bienvenue */
    char welcome[BUFSIZ]; memset(welcome, 0, BUFSIZ);
    strcpy(welcome, "Bienvenue dans le ☁️☁️  ZenCloud ☁️☁️\n");
    int error = send(client_fd, welcome, strlen(welcome), 0); perror("send ");
        if(error == -1){ close(client_fd); close(server_fd); return EXIT_FAILURE; }
    printf("%s\n", welcome);

    /* Attente du choix du client */
    char action[BUFSIZ]; memset(action, 0, BUFSIZ);
    error = recv(client_fd, action, sizeof(action), 0); perror("recv action ");
        if (error == -1) { close(client_fd); return EXIT_FAILURE; }
    printf("action client : %s\n", action);

    /* Si le choix est list, on envoie la liste de fichiers au client */
    if (strcasecmp(action, "list") == 0) {
        list(client_fd, server_fd);   
    /* Si le choix est download, on reçoit le nom du fichier et on envoie le fichier au client */    
    } else if (strcasecmp(action, "download") == 0) {
        upload(filename, client_fd, server_fd);
    /* Si le choix est upload, on reçoit le nom du fichier et on télécharge le fichier aupres du client */  
    } else if (strcasecmp(action, "upload") == 0) {
        download(filename, client_fd);
    } 
    }  
    return 0;
    
}