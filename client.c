#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MSG 32

/* rend adServ global */
struct sockaddr_in adServ;

/* cree la socket pour se connecter a un port d'une adresse ip */
int initSocket(int port, char* ip){
	int sCli = socket(PF_INET, SOCK_STREAM, 0);
	adServ.sin_family = AF_INET;
	adServ.sin_port = htons(port);
	int res = inet_pton(AF_INET, ip, &(adServ.sin_addr));
	return sCli;
}

/* se connecte a la socket que l on a initialise */
int connection(int socket){
	socklen_t lgA = sizeof(struct sockaddr_in);
	int res = connect(socket, (struct sockaddr*)&adServ, lgA);
	return res;
}

/* fonction d ecriture d un message qui est envoye sur le serveur */
int writeMsg(int socket){
	char message[MSG];
	fgets(message, MSG, stdin);
	char *pos = strchr(message, '\n');
	*pos = '\0';
	ssize_t sender = send(socket, message, strlen(message)+1, 0);
	return sender;
}

/* fonction de recoit d un message */
char* readMsg(int socket){
	char msg[256];
	ssize_t rcv = recv(socket, msg, 256, 0);
	printf("order received %d", strlen(msg));
	return msg;
}

int main(int argc, char *argv[]) {

	/* lors de l execution du programme il faut renseigner le port et l adresse ip
	s il n y a pas le bon nombre d'argument on n execute pas le programme*/
	if (argc != 3) {
		printf("Pas le bon nombre d'arguments\n");
		exit(0);
	}

	/* initialisation du port et de l adresse ip entrez lors de l execution du programme */
	char* port = argv[1];
	char* ip = argv[2];

	/* initialisation de la socket en appelant la fonction correspondante */
	int socket = initSocket(atoi(port), ip);
	
	/* nous informe si la connexion a bien ete effectuee*/
	if (connection(socket) == 0){
		printf("Connexion réussie \n");
	} else{
		printf("Connexion échouée \n");exit(0);
	}

	/* Variable que le serveur donne au client, 0 ou 1 pour savoir qui engage la conversation*/
	char* ordre; 
	ordre = readMsg(socket);
	printf("order received %s %d", ordre, strlen(ordre));
	char* msg;

	if (ordre == 0){
		/* Client qui ecrit en premier
		tant que nous lisons pas le message "fin" le chat continu */
		while(strcmp(msg, "fin") != 0){
			printf("Ecrivez votre message : ");
			writeMsg(socket);
			msg = readMsg(socket);
			printf("Message recu : %s\n", msg);
		}
	}else{
		/* Client qui attend d'abord un message
		tant que nous lisons pas le message "fin" le chat continu */
		while(strcmp(msg, "fin") != 0){
			msg = readMsg(socket);
			printf("Message recu : %s\n", msg);
			/* reverifie que le message recu n'est pas "fin" pour ne pas continuer a ecrire */
			if(strcmp(msg, "fin") != 0){
				printf("Ecrivez votre message : ");
				writeMsg(socket);
			}
		}
	}
	/* fermeture de la socket */
	close(socket);
}