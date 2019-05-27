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
#define MAX_CHANNELS 20

int sCli1;
int sCli2;

int ack = 1;

typedef struct clientDesc {
	int socketClient;
	struct sockaddr_in clientData;
} clientDesc;

typedef struct Client {
	int idClient;
	int idChannel;
	clientDesc dataClient;
	char* nickname;
} Client;

typedef struct Channel {
	int idChannel;
	char name[50];
	int sizeMax;
	int connectedClients;
	Client* clients;
} Channel;

Channel channels[MAX_CHANNELS];
// char msgCli[MSG];

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
	forwardMsg : Int x Int -> Pointer
	forwards a message received from a first client given in first parameter
	towards clients in the same channel as client given in parameter
	returns the value returned by recv
*/
void* broadcastMsg(Client client) {
	char msg[MSG];
	int res = recv(client.idClient, msg, strlen(msg)+1, MSG);

	Channel channel = channels[client.idChannel];

	for (int i = 0; i<channel.sizeMax; i++) {
		if (i != client.idClient && channel.clients[i].idClient != -1) {
			sendMSG(channel.clients[i].dataClient.socketClient, msg);
		}
	}
}

void sendMSG(int socket, char* msg) {
	send(socket, msg, strlen(msg)+1, 0);
}

/*
	sendID : Int x Char* -> Int
	//TODO
*/
int sendID(int socket, char* id) {
	ssize_t sender = send(socket, id, 2, 0);
	return sender;
}

int selectChannel(int socket) {
	int chanId = 0;
	char channel[MSG];

	while (chanId <= 0) {
		char msgServ[MSG];
		sprintf(msgServ, "Available channels : \n");

		char channelsDesc[MSG];
		for(int i = 0; i<MAX_CHANNELS; i++) {
			sprintf(channelsDesc, channels[i].name);
		}

		char response[MSG];
		sendChanInfo(socket, channelsDesc);
		int res = recv(socket, response, strlen(response)+1, MSG);

		if (res == 0) {
			return -1;
		} else {
			chanId = atoi(response);
			if(chanId > 0 && chanId < MAX_CHANNELS) {
				if(channels[chanId-1].connectedClients > channels[chanId-1].sizeMax) {
					chanId = 0;
				}
			}
		}
	}
	return chanId-1;
}

void *chat() {
	Client client;

	int chan = selectChannel(client.dataClient.socket);

	if (chan >= 0) {
		client.idChannel = chan;
	}
}

int main(int argc, char* argv[]) {

	/*
		check if port is given in arguments
	*/
	if(argc != 2) { 
		printf("Pas le bon nombre d'arguments :\n");
		printf("./server [port]\n");
		exit(0);
	} else if(strlen(argv[1]) < 4 || atoi(argv[1]) <= 1024) {
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
	struct sockaddr_in adCli;

	printf("déclaration des thread\n");
	pthread_t chat;
	
	while(1) {

		sCli1 = acceptClient(socket, adCli);
		printf("Client 1 connecté\n");

		if (pthread_create(&chat, NULL, chat, 0) == 0) {
			/*from client 1 to client 2*/
			printf("Création du thread forward1 ok\n");
		} else {
			printf("Création du thread forward1 échouée\n");
		}

		/*
			closes the sockets for each client
		*/
		pthread_join(chat, NULL);
		pthread_cancel(chat);
		close(sCli1);
		ack = 1;
		// exit(0);
	} 
	close(socket);
}