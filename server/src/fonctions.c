#pragma once

#include "global.h"

void list (int client_fd, int server_fd) {
            struct dirent *fileinfo;
        int count = 1;

        // opendir() renvoie un pointeur de type DIR. 
        DIR *dd = opendir("server/public"); 
            if (dd == NULL) {printf("Impossible d'ouvrir le répertoire.\n"); return;}
        
        while ((fileinfo = readdir(dd)) != NULL) {
            // On cache les fichiers . et ..
            if ( strcmp( fileinfo->d_name, "." ) && strcmp( fileinfo->d_name, ".." ) ) {
                printf("%d - %s\n",count, fileinfo->d_name);
                count++;
                char listfiles[BUFSIZ]; memset(listfiles, 0, BUFSIZ);
                strcpy(listfiles, fileinfo->d_name); 
                printf("%s\n", listfiles);
                // usleep(1000000);
                int error = send(client_fd, listfiles, BUFSIZ, 0); perror("send ");
                    if(error == -1){ close(client_fd); close(server_fd); return; }
            }
        }
        closedir(dd);    
}

void upload (char filename[], int client_fd, int server_fd) {
    int error = recv(client_fd, filename, BUFSIZ, 0); perror("recv filename ");
        if (error == -1) { close(client_fd); return; }
    printf("Client want to download %s\n", filename);

    char path[BUFSIZ*2]; memset(path, 0, BUFSIZ*2);
    sprintf(path, "server/public/%s", filename);
    printf("Fichier localisé dans : ..%s\n", path);

    FILE * file = fopen(path, "rb");

        if (file) {
            fseek(file,0,SEEK_END);
            long long int sizeFile = ftell(file);
            fseek(file, 0, SEEK_SET);
            printf("Taille du fichier : %lld octets\n", sizeFile);

            error = send(client_fd, &sizeFile, sizeof(long long int), 0); perror("send sizeFile ");
                if(error == -1){ close(client_fd); close(server_fd); return; }

            char *sendfile = malloc(sizeFile);
                fread(sendfile, 1, sizeFile, file); perror("fread ");

                error = send(client_fd, sendfile, sizeFile, 0); perror("send sendfile ");
                    if(error == -1){ 
                        close(client_fd); close(server_fd); return; 
                    }

                printf ("Octets envoyés : %d\n", error);
                free(sendfile); 
        } else {
            printf("Pas de fichier à ce nom\n");
        }
}

void download (char filename[], int client_fd) {
    int error = recv(client_fd, filename, BUFSIZ-1, 0); perror("recv filename ");
            if (error == -1) { close(client_fd); return; }
        printf("Client want to upload %s\n", filename);

        char path[BUFSIZ*2]; memset(path, 0, BUFSIZ*2);
        sprintf(path, "server/public/%s", filename);
        printf("Il sera téléchargé dans : ..%s\n", path);

        long long int sizeFile = 0;
        error = recv(client_fd, &sizeFile, sizeof(long long int), 0); perror("recv sizeFile ");
            if (error == -1) { close(client_fd); return; }
        printf("Taille du fichier : %lld octets\n", sizeFile);

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