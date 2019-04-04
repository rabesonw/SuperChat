#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MSG 124


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
int forwardMsg(int socketSender, int socketReceiver) {
	char msgCli[MSG];
	ssize_t rcv = recv(socketSender, msgCli, MSG, 0);
	ssize_t sender = send(socketReceiver, msgCli, strlen(msgCli)+1, 0);
	return rcv;
}

/*
	sendID : Int x Char* -> Int
	sends id message to a client given in first parameter
	message contains a value to inform order of speech to client
	returns the value returned by send
*/
int sendID(int socket, char* id) {
	ssize_t sender = send(socket, id, 2, 0);
	return sender;
}

/*
	validIP : Char* -> Int
	checks if IP given in parameter is a correct IP
	returns 1 if correct IP, 0 if not
*/
int validIP(char *ip) {
	//TODO validate IP adress
	return 0;
}

int main(int argc, char* argv[]) {

	/*
		check if port and ip were given in arguments
	*/
	if(argc != 2) { 
		printf("Pas le bon nombre d'arguments\n");
		exit(0);
	} else if(strlen(argv[1]) <= 4 || atoi(argv[1]) <= 1024) {
		printf("Mauvais port\n");
		exit(0);
	} else {
		printf("ok\n");
		//TODO insert validIP verification here
	}

	/*
		fetches port value given in argument call
	*/
	char* port = argv[1];

	int socket = initServer(atoi(port), 2);

	/*
		creates two slots for the 2 clients who will communicate
	*/
	struct sockaddr_in adCli1;
	struct sockaddr_in adCli2;
	
	while(1) {
		/*
			value to check if messages are still being sent
		*/
		int rcvStatus = 1;

		/*
			accepting the connection of the two clients
		*/
		int sCli1 = acceptClient(socket, adCli1);
		printf("client 1 connecté\n");
		int sCli2 = acceptClient(socket, adCli2);
		printf("client 2 connecté\n");

		/*
			sending ID to clients
		*/
		int confirm1 = sendID(sCli1, "0");
		int confirm2 = sendID(sCli2, "1");

		if(confirm1 || confirm2) {
			printf("order sent successfully\n");
		} else {
			printf("failed to send order\n");
		}

		/*fork to tend to multiple clients at a time*/
		pid_t pid = fork();
		if(pid == 0) {
			while(rcvStatus) {
				/*receiving message*/
				/*from client 1 to client 2*/
				rcvStatus = forwardMsg(sCli1, sCli2);
				/*from client 2 to client 1*/
				rcvStatus = forwardMsg(sCli2, sCli1);
			}
			/*
				closes the sockets for each client
			*/
			close(sCli1);
			close(sCli2);
			/*pourquoi on fait ça ?*/
			exit(0);
		} 
	}
	close(socket);
}