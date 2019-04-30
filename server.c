#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MSG 124

int sCli1;
int sCli2;

int ack = 1;

/*
	initServer : Int x Int -> Int
	Initializes the socket for the server with port and 
	available spots for clients
	Initializes the struct of type sockaddr_in with port
	Names the socket at given port
	Starts to wait for clients for the socket created
	returns the socket created.
*/
int initServer(int port, int nbClients) {
	/*declaring server socket as ipv4 and tcp*/
	int sServ = socket(PF_INET, SOCK_STREAM, 0);

	/*declaring API for adresses*/
	struct sockaddr_in ad;

	/*IPV4*/
	ad.sin_family = AF_INET;
	ad.sin_addr.s_addr = INADDR_ANY;
	/*port*/
	ad.sin_port = htons((short)port);

	/*naming the server*/
	bind(sServ, (struct sockaddr*)&ad, sizeof(ad));

	/*wait for clients*/
	listen(sServ, nbClients);
	return sServ;
}

/*
	acceptClient : Int x Struct sockaddr_in -> Int
	accepts client in given server for the given client's adress family
	returns the socket for the new client connected
*/
int acceptClient(int socketServ, struct sockaddr_in adCli) {
	socklen_t lgA = sizeof(struct sockaddr_in);
	int sCli = accept(socketServ, (struct sockaddr *)&adCli, &lgA);
	return sCli;
}

/*
	forwardMsg : Int x Int -> Int
	forwards a message received from a first client given in first parameter
	towards a second client given in second parameter
	returns the value returned by recv
*/
void* forwardMsg(int ordre) {
	char msgCli[MSG];
	while(ack != 0) {
		if (ordre == 0) {
			recv(sCli1, msgCli, MSG, 0);
			if(strcmp(msgCli, "fin") == 0) {
				send(sCli2, "fin", strlen("fin")+1, 0);
				ack = 0;
			}else if(strcmp(msgCli, "file") == 0) {
				//Envoi le mot file
				send(sCli2, msgCli, strlen(msgCli), 0);
				printf("envoie du mot : %s\n", msgCli);
				//Envoi le nom du fichier
				recv(sCli1, msgCli, MSG, 0);
				printf("nomfichier reçu : %s\n", msgCli);
				send(sCli2, msgCli, strlen(msgCli), 0);
				printf("nomfichier envoyé : %s\n", msgCli);
				//Envoi le contenu du fichier
				recv(sCli1, msgCli, MSG, 0);
				printf("contenu reçu : %s\n", msgCli);
				send(sCli2, msgCli, strlen(msgCli), 0);
				printf("contenu envoyé : %s\n", msgCli);
			}else {
				send(sCli2, msgCli, strlen(msgCli)+1, 0);
			}
		} else {
			recv(sCli2, msgCli, MSG, 0);
			if(strcmp(msgCli, "fin") == 0) {
				send(sCli1, "fin", strlen("fin")+1, 0);
				ack = 0;
			}else if(strcmp(msgCli, "file") == 0) {
				//Envoi le mot file
				send(sCli1, msgCli, strlen(msgCli), 0);
				printf("envoie du mot : %s\n", msgCli);
				//Envoi le nom du fichier
				recv(sCli2, msgCli, MSG, 0);
				printf("nomfichier reçu : %s\n", msgCli);
				send(sCli1, msgCli, strlen(msgCli), 0);
				printf("nomfichier envoyé : %s\n", msgCli);
				//Envoi le contenu du fichier
				recv(sCli2, msgCli, MSG, 0);
				printf("contenu reçu : %s\n", msgCli);
				send(sCli1, msgCli, strlen(msgCli), 0);
				printf("contenu envoyé : %s\n", msgCli);
			}else {
				send(sCli1, msgCli, strlen(msgCli)+1, 0);
			}
		}
	}
	pthread_exit(0);
}

/*
	sendID : Int x Char* -> Int
	//TODO
*/
int sendID(int socket, char* id) {
	ssize_t sender = send(socket, id, 2, 0);
	return sender;
}

int main(int argc, char* argv[]) {

	/*
		check if port is given in arguments
	*/
	if(argc != 2) { 
		printf("Pas le bon nombre d'arguments :\n");
		printf("./server [port]\n");
		exit(0);
	} else if(strlen(argv[1]) <= 4 || atoi(argv[1]) <= 1024) {
		printf("Mauvais port :\n");
		printf("./server [port]\n");
		exit(0);
	}

	/*
		fetches port value given in argument call
	*/
	char* port = argv[1];

	/*
		socket server intialisation with port given in argument call
	*/
	int socket = initServer(atoi(port), 2);

	/*
		creates two slots for the 2 clients who will communicate
	*/
	struct sockaddr_in adCli1;
	struct sockaddr_in adCli2;

	printf("déclaration des thread\n");
	pthread_t forward1;
	pthread_t forward2;
	pthread_t t_file;
	
	while(1) {

		sCli1 = acceptClient(socket, adCli1);
		printf("Client 1 connecté\n");
		sCli2 = acceptClient(socket, adCli2);
		printf("Client 2 connecté\n");
		/*
			accepting the connection of the two clients
		*/

		if (pthread_create(&forward1, NULL, forwardMsg, 0) == 0) {
			/*from client 1 to client 2*/
			printf("Création du thread forward1 ok\n");
		} else {
			printf("Création du thread forward1 échouée\n");
		}

		if (pthread_create(&forward2, NULL, forwardMsg, 1) == 0) {
			/*from client 2 to client 1*/
			printf("Création du thread forward2 ok\n");
		} else {
			printf("Création du thread forward2 échouée\n");
		}

		/*
			closes the sockets for each client
		*/
		pthread_join(forward2, NULL);
		pthread_join(forward1, NULL);
		pthread_cancel(forward1);
		pthread_cancel(forward2);
		close(sCli1);
		close(sCli2);
		ack = 1;
		// exit(0);
	} 
	close(socket);
}