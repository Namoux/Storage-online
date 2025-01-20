#pragma once

#include "global.h"

void list (int error, int client_fd) {
    printf("Fichiers présent dans le cloud : \n");
        while (error > 0) { // Tant qu'il y a des données dans la reception, on se met en mode recv
        char listfiles[BUFSIZ]; memset(listfiles, 0 , BUFSIZ);
        error = recv(client_fd, listfiles, sizeof(listfiles), 0); // perror("recv listfiles ");
            if (error == -1) { close(client_fd); return; }
        printf("%s\n", listfiles);
        }
}

void download (char filename[], int client_fd) {
    // On reçoit le nom du fichier
    printf("Fichier à télécharger : %s\n", filename);
    long long int error = send(client_fd, filename, strlen(filename), 0);
        if (error == -1) { perror("send filename "); close(client_fd); return; }

    // On crée le chemin que l'on met dans path
    char path[BUFSIZ*2]; memset(path, 0, BUFSIZ*2);
    sprintf(path, "client/public/%s", filename);

    // Ouverture du fichier mode ab pour la creation du fichier
    FILE *file = fopen(path, "wb");
        if (file) {
            printf("Il sera téléchargé dans : ..%s\n", path);

            // On reçoit la taille du fichier
            long long int sizeFile = 0;
            error = recv(client_fd, &sizeFile, sizeof(long long int), 0); 
                if (error == -1) { perror("recv sizeFile "); close(client_fd); fclose(file); return; }
            printf("Taille du fichier : %lld octets\n", sizeFile);

            fseek(file, 0, SEEK_SET);
            printf("Téléchargement en cours : %s\n", filename);

            // On alloue de la memoire a sendfile pour y stocker le fichier
            char *recvfile = malloc(sizeFile);
            long long int totalrecv = 0;
            // On reçoit le fichier tant que les octets reçus est superieur a 0
            while ((error = recv(client_fd, recvfile, sizeFile, 0)) > 1) {
                totalrecv += error;
                printf("octets reçus = %lld sur %lld\n", totalrecv, sizeFile);
                // On ecrit le fichier par octets reçus
                fwrite(recvfile, sizeof(char), error, file); // perror("fwrite ");
            }

            printf("Téléchargement terminé.\n");

            // On libere la memoire, ferme le fichier et le client
            fclose(file);
            free(recvfile);
            close(client_fd); 

        } else {
            printf("Le fichier demandé est deja existant\n");
        }
}

void upload (char filename[], int client_fd) {
    int filename_len = strlen(filename) + 1;  // Inclut le '\0', evite le bug d'une reception avec des caracteres en trop
    // On envoie le nom du fichier
    int error = send(client_fd, filename, filename_len, 0); perror("send filename ");
        if (error == -1) { close(client_fd); return; }

    // On crée son chemin et on le met dans path 
    char path[BUFSIZ*2]; memset(path, 0, BUFSIZ*2);
    sprintf(path, "client/public/%s", filename);

    // On ouvre le fichier 
    FILE * file = fopen(path, "rb");

    if (file != NULL) {
        printf("Fichier localisé dans : ..%s\n", path);

        // On determine la taille du fichier en positionnant le curseur
        fseek(file,0,SEEK_END);
        long long int sizeFile = ftell(file);
        fseek(file, 0, SEEK_SET);
        printf("Taille du fichier : %lld octets\n", sizeFile);

        // On envoie la taille du fichier au client 
        error = send(client_fd, &sizeFile, sizeof(long long int), 0); perror("send sizeFile ");
            if(error == -1) { close(client_fd); return; }

        // On alloue de la memoire a sendfile pour y stocker le fichier
        char *sendfile = malloc(sizeFile);
        long long int totalsent = 0;

        // On cree une boucle tant que les octets totaux envoyés sont inferieurs a la taille totale du fichier
        while (totalsent < sizeFile) {
            // lecture du fichier
            long long int octetslus = fread(sendfile, 1, sizeFile, file);
                if (octetslus == 0) { close(client_fd); fclose(file); return; } 

            // On cree une boucle tant que les octets envoyés sont inferieur a octets lus
            long long int octetssent = 0;
            while (octetssent < octetslus) {
                // On envoie le fichier, send n'envoi max que 2go, on envoi le restant si le fichier est trop volumineux
                long long int sent = send(client_fd, sendfile + octetssent, octetslus - octetssent, 0);
                    if (sent == -1) { close(client_fd); fclose(file); return; }

            // On remplit octetsent et totalsent en fonction de ce qui est envoyé
            octetssent += sent;
            totalsent += sent;
            printf("Octets envoyés %lld sur %lld\n", totalsent, sizeFile);   
            }
        }  

        printf("Fichier Envoyé\n");

        // on libere la memoire, ferme le fichier et le client
        free(sendfile); 
        fclose(file);
        close(client_fd);

    } else {
        printf("Pas de fichier à ce nom\n");
    }       
}