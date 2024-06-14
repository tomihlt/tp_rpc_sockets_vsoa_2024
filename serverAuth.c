#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#define PORT 8080

char* usuarioValido(char *user, char *pw);
 
int main(int argc, char const* argv[]){
	int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };
  
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
  
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
  
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
	
	/*
		COMUNICACION
	*/
	char *token;
	while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
		
        valread = read(new_socket, buffer, 1024); // "user;pw"
        buffer[valread] = '\0'; // Null-terminate the buffer
        printf("Received buffer: %s\n", buffer);
        
        char *user = strtok(buffer, ";");
        char *pw = strtok(NULL, ";");
        printf("Intento de logeo del usuario %s con pw %s\n", user, pw);
        
        char *token = usuarioValido(user, pw);
        if (token != NULL) {
            printf("El usuario %s es valido y su token es %s.\n", user, token);
            send(new_socket, token, strlen(token), 0);
            free(token); // Free the dynamically allocated token
        } else {
            printf("No existe el usuario %s o la contraseña es invalida.\n", user);
            send(new_socket, "NULL", strlen("NULL"), 0);
        }
		
        // closing the connected socket
        close(new_socket);

        // Clear the buffer
        memset(buffer, 0, sizeof(buffer));
    }
	/*
		FIN COMUNICACION
	*/
	
    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);

    return 0;
}

char* usuarioValido(char *user, char *pw) {
    FILE *file = fopen("users.txt", "r");
    if (file == NULL) {
        printf("No se pudo abrir el archivo.\n");
        return NULL;
    }

    const int MAX_LEN = 1024 + 2; // user+pw+token <= 1024 caracteres
    char line[MAX_LEN];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';

        char *token = strtok(line, ";");
        if (token != NULL && strcmp(token, user) == 0) {
            token = strtok(NULL, ";"); // Saltar contraseña
            if (token != NULL && strcmp(token, pw) == 0) {
                token = strtok(NULL, ";"); // Obtener el token
                fclose(file);
                return strdup(token); // Devolver una copia dinámica del token
            }
        }
    }

    fclose(file);
    return NULL; // Usuario o contraseña no encontrados
}