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

/* adServ API global variable */
struct sockaddr_in adServ;

/*
	initSocket : Int x Int -> Int
	Initializes the socket for the client
	returns the socket created
*/
int initSocket(int port, char* ip){
	/*declaring server socket as ipv4 and tcp*/
	int sCli = socket(PF_INET, SOCK_STREAM, 0);

	/*IPV4*/
	adServ.sin_family = AF_INET;
	/*port*/
	adServ.sin_port = htons(port);
	/*Converts IPv4 addresses from text to binary form*/
	int res = inet_pton(AF_INET, ip, &(adServ.sin_addr));
	return sCli;
}

/*
	connection : Int -> Int
	connects client to server by its socket
	returns 0 if connected, 1 if not
*/
int connection(int socket){
	socklen_t lgA = sizeof(struct sockaddr_in);
	int res = connect(socket, (struct sockaddr*)&adServ, lgA);
	return res;
}

/*
	writeMsg : Int -> Int
	sends a written message to a given socket to server 
	who will relay it towards a second client
	returns the value returned by sender
*/
int writeMsg(int socket){
	char message[MSG];
	fgets(message, MSG, stdin);
	char *pos = strchr(message, '\n');
	*pos = '\0';
	ssize_t sender = send(socket, message, strlen(message)+1, 0);
	return sender;
}

/*
	readMsg : Int x String
	reads a written message from the server 
	sent by a second client which is stored in msg
*/
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

	/*check if port and ip were given in arguments*/
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

	/*
		fetches port value given in argument call
	*/
	char* port = argv[1];

	/*
		fetches ip value given in argument call
	*/
	char* ip = argv[2];

	/*
		socket intialisation with port and ip given in argument call
	*/
	int socket = initSocket(atoi(port), ip);
	
	/*
		checks if connection has been made
	*/
	if (connection(socket) == 0){
		printf("Connexion réussie \n");
	} else{
		printf("Connexion échouée \n");exit(0);
	}

	/*
		reading ID order sent by the server
		if 0 client is the first sender, if 1 client is the first receiver
	*/
	char msg[256];
	readMsg(socket, msg);
	

	if (msg[0] == '0'){
		/* 
			first client to send
			loops until "fin" has not been written or received
		*/
		while(strcmp(msg, "fin") != 0){
			printf("Ecrivez votre message : ");
			writeMsg(socket);
			printf("Attente d'un message\n");
			readMsg(socket, msg);
			printf("Message recu : %s\n", msg);
		}
	}else{
		/* 
			first client to read
			loops until "fin" has not been written or received 
		*/
		while(strcmp(msg, "fin") != 0){
			printf("Attente d'un message\n");
			readMsg(socket, msg);
			printf("Message recu : %s\n", msg);
			/* 
				checks is received message has not been "fin" to avoid sending another message
			*/
			if(strcmp(msg, "fin") != 0){
				printf("Ecrivez votre message : ");
				writeMsg(socket);
			}
		}
	}
	/*  
		closes the client's socket	
	*/
	close(socket);
}
