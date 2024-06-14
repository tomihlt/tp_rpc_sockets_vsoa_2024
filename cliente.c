#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080

#include "app.h"

void auth(char *server_ip, char *user, char *pw);
void interact(char *server_ip, char *token, char *operacion, char *contenido);
  
int main(int argc, char *argv[])
{	
	char* server_ip = NULL;
	char* user = NULL;
	char* pw = NULL;
	char* token = NULL;
	char* operacion = NULL;
	char* contenido = NULL;
	int opt;
	
	while ((opt = getopt(argc, argv, "u:p:t:o:c:")) != -1) {
        switch (opt) {
            case 'u':
                user = optarg;
                break;
            case 'p':
                pw = optarg;
                break;
			case 't':
				token = optarg;
				break;
			case 'o':
				operacion = optarg;
				break;
			case 'c':
				contenido = optarg;
				break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        server_ip = argv[optind];
    }
	
	if (server_ip && user && pw && !(token || operacion || contenido)) {
        auth(server_ip, user, pw);
    }else if(server_ip && token && (operacion || contenido) && !(user || pw)){
		interact(server_ip, token, operacion, contenido);
	}
	
    
    return 0;
}

void auth(char *server_ip, char *user, char *pw){
	int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    //char* hello = "Hello from client";
    char buffer[1024] = { 0 };
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
  
    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return;
    }
  
    if ((status
         = connect(client_fd, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        return;
    }
	/*
		COMUNICACION
	*/
	char *credenciales = (char *) malloc(1024 * sizeof(char));
	strcpy(credenciales, user); // user
	strcat(credenciales, ";"); // user;
	strcat(credenciales, pw); // user;pw
	
    send(client_fd, credenciales, strlen(credenciales), 0);
    //printf("Hello message sent\n");
    valread = read(client_fd, buffer, 1024);
	if(buffer == "NULL")
		printf("Usuario o contraseña invalido.\n");
	else
		printf("Token para %s: %s\n", user, buffer);
	/*
		FIN COMUNICACION
	*/
  
    // closing the connected socket
    close(client_fd);
}

void interact(char *server_ip, char *token, char *operacion, char *contenido){
	
	// Conexion RPC
	CLIENT *clnt;
	clnt = clnt_create (server_ip, APP_PROG, ADD_VERS, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (server_ip);
		exit (1);
	}
	
	// Validar usuario
	int *esValido;
	str validUser;
	validUser.s = strdup(token);
	esValido = usuariovalido_1(&validUser, clnt);
	if((esValido == (int *) NULL) || (! *esValido)){
		printf("Token no válido.\n");
		clnt_destroy (clnt);
		return;
	}else{
		printf("Token válido.\n");
	}
	
	if(operacion){
		if(!strcmp(operacion, "list")){
			str  *result_1;
			char *files_1_arg;
			result_1 = files_1((void*)&files_1_arg, clnt);
			if (result_1 == (str *) NULL) {
				clnt_perror (clnt, "call failed");
			}
			printf("Lista de archivos en el directorio:\n%s\n", result_1->s);
		}else if(!strcmp(operacion, "read")){
			if(!contenido){
				printf("Debe especificar el nombre del archivo con el parámetro -c.\n");
				clnt_destroy (clnt);
				return;
			}
			str  *result_2;
			str  readfile_1_arg;
			readfile_1_arg.s = malloc(1024 * sizeof(char));
			strcpy(readfile_1_arg.s, contenido);
			result_2 = readfile_1(&readfile_1_arg, clnt);
			if (result_2 == (str *) NULL) {
				clnt_perror (clnt, "call failed");
			}else
				printf("%s\n", result_2->s);
		}else if(!strcmp(operacion, "write")){
			if(!contenido){
				printf("Debe especificar el nombre del archivo con el parámetro -c.\n");
				clnt_destroy (clnt);
				return;
			}
			printf("Ingrese el contenido a escribir (maximo 1024 caracteres):");
			char *texto = (char *) malloc(1024 * sizeof(char));
			// scanf("%1023s", texto);
			fgets(texto, 1024, stdin);
			int  *result_3;
			strw  writefile_1_arg;
			writefile_1_arg.file = malloc(1024 * sizeof(char));
			writefile_1_arg.s = malloc(1024 * sizeof(char));
			strcpy(writefile_1_arg.file, contenido);
			strcpy(writefile_1_arg.s, texto);
			result_3 = writefile_1(&writefile_1_arg, clnt);
			if (result_3 == (int *) NULL) {
				clnt_perror (clnt, "call failed");
			}else if(!*result_3)
				printf("El archivo '%s' no existe.\n", contenido);
			else
				printf("Se escribió en el archivo '%s'\n", contenido);
		}
	}
	
	clnt_destroy (clnt);
}