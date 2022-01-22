// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 9000
#define MAXLINE 1024
#define MAX_IP_SIZE 16
#define MAX_PORT_SIZE 5
#define MAX_COMAND_SIZE 6
#define FALSE 0
#define TRUE 1

int sockfd;
int n, len, quit = 0;
char buffer[MAXLINE], host[MAX_IP_SIZE], firstPort[MAX_PORT_SIZE], comand[MAX_COMAND_SIZE];
struct sockaddr_in servaddr;

void createInitialConfigurations(char**);
void fillVariablesWithArgv(char**);
void createSocketFileDescriptor();
void fillServerInformation();
void sendStartMessage();
void receiveMessageFromServer();
void receiveMessagesFromServer(int);
void openCommunicationWithServer();
int sentMessage();
void verifyMessageNeedResponse(char message[]);

// Driver code
int main(int argc, char **argv) {
	
	createInitialConfigurations(argv);
	
	while(quit != 1){
		openCommunicationWithServer();
	}
	
	close(sockfd);

	return 0;
}

void createInitialConfigurations(char** argv){
	fillVariablesWithArgv(argv);
	createSocketFileDescriptor();
	fillServerInformation();
	sendStartMessage();
}

void fillVariablesWithArgv(char **argv){
	strcpy(host,argv[1]);
	strcpy(firstPort,argv[2]);
	strcpy(comand,argv[3]);
}

void createSocketFileDescriptor(){
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
}
void fillServerInformation(){
	memset(&servaddr, 0, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(firstPort));
	servaddr.sin_addr.s_addr = INADDR_ANY;
}

void sendStartMessage(){
	sendto(sockfd, (const char *)comand, strlen(comand),
		MSG_CONFIRM, (const struct sockaddr *) &servaddr,
			sizeof(servaddr));
	receiveMessagesFromServer(4);
}

void receiveMessageFromServer(){
	memset(buffer, 0, MAXLINE); //clears buffer 
	n = recvfrom(sockfd, (char *)buffer, MAXLINE,
				MSG_WAITALL, (struct sockaddr *) &servaddr,
				&len);
	buffer[n] = '\0';
	if (strncmp(buffer,"end",3) == 0){
		quit = 1;
	}
	else{
		printf("< %s\n", buffer);
	}
}

void receiveMessagesFromServer(int totalMessages){
	for(int currentMessage = 0; currentMessage < totalMessages; currentMessage++)
		receiveMessageFromServer();
}

void openCommunicationWithServer(){
	while(1){
		if (!sentMessage() || quit == 1)
			break;
	}
}

int sentMessage(){
	char messageToBeSent[MAXLINE];
	printf("> ");
	fgets(messageToBeSent, MAXLINE-1, stdin);
	messageToBeSent[strlen(messageToBeSent)-1] = '\0';
	sendto(sockfd, (const char *)messageToBeSent, strlen(messageToBeSent),
		MSG_CONFIRM, (const struct sockaddr *) &servaddr,
		sizeof(servaddr));
	verifyMessageNeedResponse(messageToBeSent);
	if (strlen(messageToBeSent) == 0) {
		return FALSE;
	}
	else if (strncmp(buffer,"gameover 0",10) == 0){
		quit = 1;
		return FALSE;
	}
	else{
		return TRUE;
	}
}

void verifyMessageNeedResponse(char message[]){
	if (strcmp(message,"getdefenders") == 0){
		receiveMessageFromServer();
	}
	else if (strncmp(message,"getturn",7) == 0){
		for (int i = 0; i < 4; i++){
			receiveMessageFromServer();
			if (strncmp(buffer,"base",3) != 0){
				break;
			}
		}
	}
	else if (strncmp(message,"shot",4) == 0){
		receiveMessageFromServer();
	}
	else if (strncmp(message,"quit",4) == 0){
		receiveMessageFromServer();
		quit = 1;
	}
	else if (strncmp(message,"start",4) == 0){
		receiveMessagesFromServer(4);
	}
	else{
		receiveMessageFromServer();
	}
}

