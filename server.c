#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MSG 124

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

int acceptClient(int socketServ, struct sockaddr_in adCli) {
	socklen_t lgA = sizeof(struct sockaddr_in);
	int sCli = accept(socketServ, (struct sockaddr *)&adCli, &lgA);
	return sCli;
}

int forwardMsg(int socketSender, int socketReceiver) {
	char msgCli[MSG];
	ssize_t rcv = recv(socketSender, msgCli, MSG, 0);
	ssize_t sender = send(socketReceiver, msgCli, strlen(msgCli)+1, 0);
	return rcv;
}

int sendID(int socket, char* id) {
	printf("id %s\n", id);
	ssize_t sender = send(socket, id, 2, 0);
	return sender;
}

int main(int argc, char* argv[]) {

	if(argc != 2) { 
		printf("Pas le bon nombre d'arguments\n");
		exit(0);
	}

	char* port = argv[1];

	int socket = initServer(atoi(port), 2);

	struct sockaddr_in adCli1;
	struct sockaddr_in adCli2;
	
	while(1) {
		int rcvStatus = 1;

		int sCli1 = acceptClient(socket, adCli1);
		printf("client 1 connecté\n");
		int sCli2 = acceptClient(socket, adCli2);
		printf("client 2 connecté\n");

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
				rcvStatus = forwardMsg(sCli1, sCli2);
				rcvStatus = forwardMsg(sCli2, sCli1);
			}
			close(sCli1);
			close(sCli2);
			exit(0);
		} 
	}
	close(socket);
}