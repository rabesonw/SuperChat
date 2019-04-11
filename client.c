#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MSG 256
#define LIM "."

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
void readMsg(int socket, char* msg){
	ssize_t rcv = recv(socket, msg, 256, 0);
}

/*
	checkDigits : Char* -> Int
	pre-requisites : IP given must not contain dots
	checks if IP given in parameter has only digits
	and not letters / other characters
	returns 1 if correct IP, 0 if not
*/
int checkDigits(char *ip) {
	while (*ip) {
		if (*ip >= '0' && *ip <= '9') {
			++ip;
		} else {
			return 0;
		}
	}
	return 1;
}

/*
	validIP : Char* -> Int
	checks if IP given in parameter is a correct IP
	returns 1 if correct IP, 0 if not
*/
int validIP(char *ip) {
	int num, dots = 0;
	char *ipDigits;

	if (ip == NULL) {
		return 0;
	}

	ipDigits = strtok(ip, LIM);

	if (ipDigits == NULL) {
		return 0;
	}

	while (ipDigits) {
		if (!checkDigits(ipDigits)) {
			return 0;
		}

		num = atoi(ipDigits);

		if (num >= 0 && num <= 255) {
			ipDigits = strtok(NULL, LIM);
			if (ipDigits != NULL) {
				++dots;
			} 
		} else {
			return 0;
		}
	}

	if (dots != 3) {
		return 0;
	}

	return 1;
}

int main(int argc, char *argv[]) {

	/* lors de l execution du programme il faut renseigner le port et l adresse ip
	s il n y a pas le bon nombre d'argument on n execute pas le programme*/
	if (argc != 3) {
		printf("Pas le bon nombre d'arguments\n");
		exit(0);
	} else if(strlen(argv[1]) <= 4 || atoi(argv[1]) <= 1024) {
		printf("Mauvais port\n");
		exit(0);
	} else if(!validIP(argv[2])) {
		printf("Mauvaise adresse IP\n");
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
	char msg[256];
	readMsg(socket, msg);
	printf("order received %s\n", msg);
	

	if (msg[0] == '0'){
		/* Client qui ecrit en premier
		tant que nous lisons pas le message "fin" le chat continu */
		while(strcmp(msg, "fin") != 0){
			printf("Ecrivez votre message : ");
			writeMsg(socket);
			readMsg(socket, msg);
			printf("Message recu : %s\n", msg);
		}
	}else{
		/* Client qui attend d'abord un message
		tant que nous lisons pas le message "fin" le chat continu */
		while(strcmp(msg, "fin") != 0){
			readMsg(socket, msg);
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
