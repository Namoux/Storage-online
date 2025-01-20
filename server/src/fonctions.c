#pragma once

#include "global.h"

float conversionOctets (long long int octets, float octetsConverted, char unite[3]) {

    if (octets < 1000) {
        printf("Pas besoin de conversion\n");
    // On converti les octets en ko
    } else if (octets < 1000000 ) {
        octetsConverted =octets / 1000.00;
        strcpy(unite,"ko");
        printf("octets convertis = %.2f %s\n", octetsConverted, unite);
    // On converti les octets en mo
    } else if (octets < 1000000000) {
        octetsConverted = octets / 1000000.00;
        strcpy(unite,"mo");
        printf("octets convertis = %.2f%s\n", octetsConverted, unite);
    // On converti les octets en go
    } else if (octets < 1000000000000) {
        octetsConverted = octets / 1000000000.00;
        strcpy(unite,"go");
        printf("octets convertis = %.2f %s\n", octetsConverted, unite);
    }
    return octetsConverted;
}

void list (int client_fd, int server_fd) {
        struct dirent *fileinfo;
        int count = 1;

        // opendir() renvoie un pointeur de type DIR. Scan du dossier.
        DIR *dd = opendir("server/public"); 
            if (dd == NULL) {printf("Impossible d'ouvrir le répertoire.\n"); return;}
        
        while ((fileinfo = readdir(dd)) != NULL) {
            // On cache les fichiers . et ..
            if ( strcmp( fileinfo->d_name, "." ) && strcmp( fileinfo->d_name, ".." ) ) {
                printf("%d - %s\n",count, fileinfo->d_name);
                count++;
                char listfiles[BUFSIZ]; memset(listfiles, 0, BUFSIZ);
                // On copie le nom des fichiers dans listfiles
                strcpy(listfiles, fileinfo->d_name); 
                // printf("%s\n", listfiles);
                // On envoie la liste au client
                int error = send(client_fd, listfiles, BUFSIZ, 0); perror("send ");
                    if(error == -1){ close(client_fd); close(server_fd); return; }
            }
        }
        // On ferme le dossier et le client 
        closedir(dd);  
        close(client_fd);  
}

void upload (char filename[], int client_fd, int server_fd) {
    // On reçoit le nom du fichier
    int error = recv(client_fd, filename, BUFSIZ, 0); perror("recv filename ");
        if (error == -1) { close(client_fd); return; }
    printf("Client want to download %s\n", filename);

    // On crée le chemin que l'on met dans path
    char path[BUFSIZ*2]; memset(path, 0, BUFSIZ*2);
    sprintf(path, "server/public/%s", filename);

    // Ouverture du fichier
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
                if(error == -1){ close(client_fd); close(server_fd); return; }

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
                        if (sent == -1) { close(client_fd); close(server_fd); fclose(file); return; }

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

void download (char filename[], int client_fd) {
    // On reçoit le nom du fichier
    int error = recv(client_fd, filename, BUFSIZ-1, 0); perror("recv filename ");
            if (error == -1) { close(client_fd); return; }
        printf("Client want to upload %s\n", filename);

        // On crée son chemin et on le met dans path
        char path[BUFSIZ*2]; memset(path, 0, BUFSIZ*2);
        sprintf(path, "server/public/%s", filename);

        // On ouvre le fichier mode ab pr le creer si inexistant
        FILE *file = fopen(path, "ab");

            if (file) {
                printf("Il sera téléchargé dans : ..%s\n", path);

                // on reçoit la taille du fichier, la taille peut etre tres grande, on le stocke donc dans long long int qui fait 8 octets
                long long int sizeFile = 0;
                error = recv(client_fd, &sizeFile, sizeof(long long int), 0); perror("recv sizeFile ");
                    if (error == -1) { close(client_fd); return; }
                printf("Taille du fichier : %lld octets\n", sizeFile);

                // Conversion des octets avec la fonction
                char Unit[3];
                float sizeFileConverted = 0;
                sizeFileConverted = conversionOctets(sizeFile, sizeFileConverted, Unit);
                printf("sizefile converti = %.2f %s\n", sizeFileConverted, Unit);

                // On alloue la memoire pour recevoir le fichier
                char *recvfile = malloc(sizeFile);
                long long int totalrecv = 0;
                if (sizeFile > 0) {
                    fseek(file, 0, SEEK_SET);
                    printf("Téléchargement en cours : %s\n", filename);

                // On reçoit le fichier tant que les octets reçus est superieur a 0 
                while ((error = recv(client_fd, recvfile, sizeFile, 0)) > 0) {
                    totalrecv += error;
                    printf("octets reçus = %lld sur %lld\n", totalrecv, sizeFile);
                    // On ecrit le fichier par octets reçus
                    fwrite(recvfile, sizeof(char), error, file); // perror("fwrite ");
                }

                printf("Téléchargement terminé.\n");

                } else {
                    printf("Aucune reception de donnée du client...\n");
                }
                    // On libere la memoire, ferme le fichier et le client
                    fclose(file);
                    free(recvfile);
                    close(client_fd);

            } else { // Ici ca bug à corriger
                printf("Fichier deja existant dans le serveur\n");
            }
}

