#pragma once

#include "global.h"

void list (int error, int client_fd) {
    printf("Fichiers présent dans le cloud : \n");
        while (error > 0) {
        char listfiles[BUFSIZ]; memset(listfiles, 0 , BUFSIZ);
        error = recv(client_fd, listfiles, sizeof(listfiles), 0); // perror("recv listfiles ");
            if (error == -1) { close(client_fd); return; }
        printf("%s\n", listfiles);
        }
}

void download (char filename[], int client_fd) {
    printf("Fichier à télécharger : %s\n", filename);
    int error = send(client_fd, filename, strlen(filename), 0); perror("send filename ");
        if (error == -1) { close(client_fd); return; }

    long long int sizeFile = 0;
    error = recv(client_fd, &sizeFile, sizeof(long long int), 0); perror("recv sizeFile ");
        if (error == -1) { close(client_fd); return; }
    printf("Taille du fichier : %lld octets\n", sizeFile);

    
    char path[BUFSIZ*2]; memset(path, 0, BUFSIZ*2);
    sprintf(path, "client/public/%s", filename);
    printf("Il sera téléchargé dans : ..%s\n", path);

    FILE *file = fopen(path, "ab");
        fseek(file, 0, SEEK_SET);
    printf("Téléchargement en cours : %s\n", filename);

    char *recvfile = malloc(sizeFile);
    while ((error = recv(client_fd, recvfile, sizeFile, 0)) > 0) {
        printf("octets reçus = %d\n", error);
        fwrite(recvfile, sizeof(char), error, file); // perror("fwrite ");
    }
        fclose(file);
        free(recvfile);
}

void upload (char filename[], int client_fd) {
    int filename_len = strlen(filename) + 1;  // Inclut le '\0'
    int error = send(client_fd, filename, filename_len, 0); perror("send filename ");
        if (error == -1) { close(client_fd); return; }


    char path[BUFSIZ*2]; memset(path, 0, BUFSIZ*2);
    sprintf(path, "client/public/%s", filename);
    printf("Fichier localisé dans : ..%s\n", path);

    FILE * file = fopen(path, "rb");

    if (file != NULL) {
        fseek(file,0,SEEK_END);
        long long int sizeFile = ftell(file);
        fseek(file, 0, SEEK_SET);
        printf("Taille du fichier : %lld octets\n", sizeFile);

        error = send(client_fd, &sizeFile, sizeof(long long int), 0); perror("send sizeFile ");
            if(error == -1){ close(client_fd); return; }

        char *sendfile = malloc(sizeFile);
        fread(sendfile, 1, sizeFile, file); perror("fread ");

        error = send(client_fd, sendfile, sizeFile, 0); perror("send sendfile ");
            if(error == -1){ 
                close(client_fd); return; 
            }

        printf ("Octets envoyés : %d\n", error);
        free(sendfile); 
    } else {
        printf("Pas de fichier à ce nom\n");
    }       
}