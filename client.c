#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MSG 32

struct sockaddr_in adServ;

int initialisation(int port, char* ip){
	int sCli = socket(PF_INET, SOCK_STREAM, 0);
	adServ.sin_family = AF_INET;
	adServ.sin_port = htons(port);
	int res = inet_pton(AF_INET, ip, &(adServ.sin_addr));
	return sCli;
}

int connection(int socket){
	socklen_t lgA = sizeof(struct sockaddr_in);
	int res = connect(socket, (struct sockaddr*)&adServ, lgA);
	return res;
}

int writeMsg(int socket){
	char message[MSG];
	fgets(message, MSG, stdin);
	char *pos = strchr(message, '\n');
	*pos = '\0';
	ssize_t sender = send(socket, message, strlen(message)+1, 0);
	return sender;
}

char* readMsg(int socket){
	char msg[256];
	ssize_t rcv = recv(socket, msg, 256, 0);
	return msg;
}

char* readID(int socket) {
	char msg[2];
	ssize_t id = recv(socket, msg, 1, 0);
	return msg;
}

int main(int argc, char *argv[]) {

	if (argc != 3) {
		printf("Pas le bon nombre d'arguments\n");
		exit(0);
	}

	char* port = argv[1];
	char* ip = argv[2];
	int socket = initialisation(atoi(port), ip);
	
	if (connection(socket) == 0){
		printf("Connexion réussie \n");
	} else{
		printf("Connexion échouée \n");exit(0);
	}

	/* Variable que le serveur donne au client 0 ou 1 pour savoir qui engage la conversation*/
	int ordre = atoi(readID(socket));
	char* msg;

	if (ordre == 0){
		/*Client qui ecrit en premier*/
		while(strcmp(msg, "fin") != 0){
			printf("Ecrivez votre message : ");
			writeMsg(socket);
			msg = readMsg(socket);
			printf("Message recu : %s\n", msg);
		}
		/*Client qui attend d'abord un message*/
	}else{
		while(strcmp(msg, "fin") != 0){
			msg = readMsg(socket);
			printf("Message recu : %s\n", msg);
			if(strcmp(msg, "fin") != 0){
				printf("Ecrivez votre message : ");
				writeMsg(socket);
			}
		}
	}
	close(socket);
}