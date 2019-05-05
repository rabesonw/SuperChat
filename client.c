#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MSG 256
#define LIM "."

/* adServ API global variable */
struct sockaddr_in adServ;

int ack = 1;

int socketActif;

pthread_t readerT;
pthread_t writerT;

//pthread_t tFile;

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
	inet_pton(AF_INET, ip, &(adServ.sin_addr));
	return sCli;
}

/*
	connection : Int -> Int
	connects client to server by its socket
	returns 0 if connected, 1 if not
*/
int connection(){
	socklen_t lgA = sizeof(struct sockaddr_in);
	int res = connect(socketActif, (struct sockaddr*)&adServ, lgA);
	return res;
}

// char* fileHandler() {
// 	char* fileName = malloc(MSG*sizeof(char));
// 	printf("Choisissez un fichier :\n");
// 	system("cd Transfere/ && ls");
// 	fgets(fileName, MSG, stdin);
// 	char *pos = strchr(fileName, '\n');
// 	*pos = '\0';
// }

void* fileReceiver() {
	char fileName[MSG];
	char command[MSG];
	char fileContent[MSG];

	//recoit le nom du fichier
	recv(socketActif, fileName, MSG, 0);
	sprintf(command, "cd Telecharge && touch %s", fileName);
	system(command);
	char route[MSG];
	FILE *fd;
	sprintf(route, "Telecharge/%s", fileName);
	fd = fopen(route, "w");

	//recoit le contenu du fichier
	recv(socketActif, fileContent, MSG, 0);
	fprintf(fd, "%s", fileContent);
	fclose(fd);
	printf("Fichier : %s reçu\n", fileName);

	pthread_exit(0);
}

void* fileSender() {
	char* file = malloc(MSG*sizeof(char));
	printf("Choisissez un fichier :\n");
	system("cd Transfere/ && ls");
	fgets(file, MSG, stdin);
	char *pos = strchr(file, '\n');
	*pos = '\0';

	printf("file name : %s\n", file);

	FILE *fd;
	char route[MSG];
	sprintf(route, "Transfere/%s", file);
	fd = fopen(route, "r");

	while (fd == NULL) {
		printf("Mauvais nom de fichier\n");
		system("cd Transfere/ && ls");
		printf("%s\n", file);
		printf("Entrer un nom de fichier : \n");
		fgets(file, MSG, stdin);
		char *pos = strchr(file, '\n');
		*pos = '\0';
		sprintf(route, "Transfere/%s", file);
		fd = fopen(route, "r");
	}

	//Envoie le nom du fichier
	send(socketActif, file, strlen(file)+1, 0);
	fseek(fd, 0, SEEK_END);
	long fsize = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	char *content = malloc(fsize + 1);
	fread(content, 1, fsize, fd);

	//Envoie le contenu du fichier
	send(socketActif, content, strlen(content)+1, 0);
	fclose(fd);
	free(content);
	free(file);
	printf("(Fichier envoyé)\n");

	pthread_exit(0);
}


/*
	writeMsg : Int -> Int
	sends a written message to a given socket to server 
	who will relay it towards a second client
	returns the value returned by sender
*/
void *writeMsg(){
	char message[MSG];
	while(strcmp(message, "fin") != 0 && ack == 1) {
		fgets(message, MSG, stdin);
		char *pos = strchr(message, '\n');
		*pos = '\0';
		if(strcmp(message, "file") == 0) {
			//Envoi le mot "file"
			send(socketActif, message, strlen(message)+1, 0);
			if (pthread_create(&writerT, NULL, fileSender, 0) == 0) {
				printf("création thread File Write ok\n");
			} else {
				printf("création du thread File échouée\n");
			} 
		}
		send(socketActif, message, strlen(message)+1, 0);
	}
	ack = 0;
	pthread_exit(0);
}

/*
	readMsg : Int x String
	reads a written message from the server 
	sent by a second client which is stored in msg
*/
void *readMsg(){
	char *msg = malloc(256*sizeof(char));
	while(strcmp(msg, "fin") != 0 && ack == 1) {
		recv(socketActif, msg, 256, 0);
		if(strcmp(msg, "file") == 0) {
			if (pthread_create(&readerT, NULL, fileReceiver, 0) == 0) {
				printf("création thread File Read ok\n");
			} else {
				printf("création du thread File échouée\n");
			} 
		}else{
			printf("Message reçu : %s\n", msg);
		}
	}
	send(socketActif, "fin", strlen("fin")+1, 0);
	ack = 0;
	free(msg);
	pthread_exit(0);
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
		printf("Pas le bon nombre d'arguments :\n");
		printf("./client [port] [ip]\n");
		exit(0);
	} else if(strlen(argv[1]) <= 4 || atoi(argv[1]) <= 1024) {
		printf("Mauvais port :\n");
		printf("./client [port] [ip]\n");
		exit(0);
	} else if(!validIP(argv[2])) {
		printf("Mauvaise adresse IP :\n");
		printf("./client [port] [ip]\n");
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
	socketActif = initSocket(atoi(port), ip);
	
	/*
		checks if connection has been made
	*/
	if (connection() == 0){
		printf("Connexion réussie \n");
	} else{
		printf("Connexion échouée \n");exit(0);
	}

	pthread_t tWrite;
	pthread_t tRead;

	if (pthread_create(&tWrite, NULL, writeMsg, NULL) == 0) {
		printf("création thread Write ok\n");
	} else {
		printf("création du thread Write échouée\n");
	} 

	if(pthread_create(&tRead, NULL, readMsg, NULL) == 0) {
		printf("création thread Read ok\n");
	} else {
		printf("création du thread Read échouée\n");
	}
	/*  
		closes the client's socket	
	*/
	pthread_join(tWrite, NULL);
	pthread_join(tRead, NULL);
	pthread_cancel(tWrite);
	pthread_cancel(tRead);
	close(socketActif);
}