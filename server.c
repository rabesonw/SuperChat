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

typedef struct forward_args {
	int client1;
	int client2;
	int order;
} forward_args;

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
	int res = bind(sServ, (struct sockaddr*)&ad, sizeof(ad));

	/*wait for clients*/
	int ear = listen(sServ, nbClients);
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
void* forwardMsg(void *args) {
	int ack = 0;
	forward_args *arguments = (forward_args *) args;
	printf("(forward 1) ordre = %d\n", arguments->order);
	char msgCli[MSG];
	while(ack == 0) {
		if (arguments->order == 0) {
			printf("(forward 2) ordre = %d\n", arguments->order);
			ssize_t rcv = recv(arguments->client1, msgCli, MSG, 0);
			ssize_t sender = send(arguments->client2, msgCli, strlen(msgCli)+1, 0);
			ack = rcv;
		} else {
			printf("(forward 3) ordre = %d\n", arguments->order);
			ssize_t rcv = recv(arguments->client2, msgCli, MSG, 0);
			ssize_t sender = send(arguments->client1, msgCli, strlen(msgCli)+1, 0);
			ack = rcv;
		}	
	}
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
		printf("Pas le bon nombre d'arguments\n");
		exit(0);
	} else if(strlen(argv[1]) <= 4 || atoi(argv[1]) <= 1024) {
		printf("Mauvais port\n");
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
	
	while(1) {

		int sCli1 = acceptClient(socket, adCli1);
		printf("Client 1 connecté\n");
		int sCli2 = acceptClient(socket, adCli2);
		printf("Client 2 connecté\n");
		/*
			accepting the connection of the two clients
		*/

		forward_args *arguments = malloc (sizeof(forward_args));
		arguments->client1 = sCli1;
		arguments->client2 = sCli2;

		/*receiving message*/
		printf("creation des thread\n");
		arguments->order = 0;
		printf("(main 1) ordre = %d\n", arguments->order);
		if (pthread_create(&forward1, NULL, forwardMsg, arguments) == 0) {
			/*from client 1 to client 2*/
			printf("forward 1\n");
		} else {
			printf("création du thread 1 échouée\n");
		}
		printf("(main 2) ordre = %d\n", arguments->order);
		arguments->order = 1;
		printf("(main 3) ordre = %d\n", arguments->order);
		if (pthread_create(&forward2, NULL, forwardMsg, arguments) == 0) {
			/*from client 2 to client 1*/
			printf("forward 2\n");
		} else {
			printf("création du thread 2 échouée\n");
		}
		printf("(main 4) ordre = %d\n", arguments->order);

		/*
			closes the sockets for each client
		*/
		close(sCli1);
		close(sCli2);
		// exit(0);
	} 
	close(socket);
}