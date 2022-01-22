// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>

#define MAXLINE 1024
#define POKENAME_MAX_SIZE 7
#define TOTAL_PORT 4
#define FALSE 0
#define TRUE 1
#define SUCESS 0
#define FAILED 1
#define TOTAL_DEFENSIVE_POKEMONS 6
#define POKENAME_1 "Zubat"
#define POKENAME_2 "Lugia"
#define POKENAME_3 "Mewtwo"

typedef struct{
	int path;
	int serverLocation;	
} Pokemon;

typedef struct{
	Pokemon pokemon;
	int id;
	char name[POKENAME_MAX_SIZE];
	int hits;
	int hitsToDie;
} EnemyPokemon;

typedef struct{
	struct sockaddr_in sockaddr_in;
	Pokemon pokemonsInBase[TOTAL_DEFENSIVE_POKEMONS];
	int totalPokemons;
} Server;

int sockfd[4], len, n, totalDefensivePokemonsWasCreated = 0,totalEnemyPokemonsWasCreated, total = 0, actualTurn = 0, actualID = 0;
int totalEnemiesKilled = 0, enemiesThatReachedPokedex = 0, totalSeconds = 0;
Server servers[4];
struct sockaddr_in cliaddr;
char buffer[MAXLINE], receivedCommands[5][10], returnedMessage[MAXLINE],returnedMessage2[MAXLINE], status[2] = "1";
Pokemon defensivePokemons[TOTAL_DEFENSIVE_POKEMONS];
EnemyPokemon enemyPokemons[20];
time_t start;

// Funções para limpar conteudo
void clearMessage();
void clearEnemyPokemonsInTable();
void clearEnemyPokemonsInTableByID(int);
void clearDefensivePokemonsInTable();

//Funções de conversão de inteiro para string
void reverse(char*);
void itoa(int, char*);

//Funções referente a criação, movimentação e ação dos pokemons
int createNewTurn();
void createRandomPositionForDefensivePokemon(int);
Pokemon createRandomPositionForEnemyPokemon();
void insertDefensivePokemonsInTable();
int generateID();
void makesEnemyWalkFoward(int);
void makesActionWhenEnemyReachPokedex(int);
void shotInEnemyPokemonByID(int id);

//Funções referente a posição dos pokemons
void generateNewRandomEnemyPokemon();
char* getPokemonsPosition(int, char*);
Pokemon returnValidPositionToEnemyPokemon();
EnemyPokemon getEnemyInPosition(int, int);
EnemyPokemon getEnemyByID(int id);
void addPokemonInServer(Pokemon, int);

//Funções de verificação
int verifyIfHasSpaceToCreateNewEnemyPokemon();
int verifyExistsEnemyInPosition(int, int);
int verifyCommandIsStart(char*);
int verifyMessageIsEmpty(char*);
int verifyExistsEnemyByID(int id);
int verifyIfEnemyCanBeTargeted(int, int);
int verifyPositionInTableIsFill(Pokemon);

//Funções de configuração do servidor
void createInitialConfigurations(char**);
void createSocketsFileDescriptors();
void makeMemsetInAllAddress();
void fillServersInformation(int);
void bindAllSocks();

//Funções de comunicação com o Client
void openCommunicationWithClient();
void printGameStarted();
int receiveMessageFromClient();
void sendMessageToClient(char*);
void closeCommunication();
void waitCommandAfterGameOver();
void sendGameOverMessage(char*);

//Funções de montagem de respostas para o client
void makeActionWhenCommandEqualsGetDefenders();
void makeActionWhenCommandEqualsGetTurn();
void makeActionWhenCommandEqualsShot();
void makeActionWhenInvalidTurn();
char* mountTurnMessagePerServer(int);
char* mountShotrespMessage();
char* defensivePokemonToString(Pokemon, char*);
char* enemyPokemonToString(EnemyPokemon);
int countHowManyWordsAreInTheMessage(char*);
void fillReceivedCommands(int);

int main(int argc, char **argv) {

	createInitialConfigurations(argv);
	while (strcmp(status,"1") == 0){
		openCommunicationWithClient();
		if (strcmp(status,"3") != 0)
			waitCommandAfterGameOver();
	}
		closeCommunication();

	return 0;
}

void createInitialConfigurations(char** argv){
	start = time(NULL); 
	createSocketsFileDescriptors();
	makeMemsetInAllAddress();
	fillServersInformation(atoi(argv[2]));
	bindAllSocks();
}
void createSocketsFileDescriptors(){
	for (int i = 0; i < TOTAL_PORT; i++){
		if ((sockfd[i] = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
		}
	}
}

void makeMemsetInAllAddress(){
	for (int i = 0; i < TOTAL_PORT; i++){
		memset(&servers[i], 0, sizeof(servers[i]));
	}
	memset(&cliaddr, 0, sizeof(cliaddr));
}

void fillServersInformation(int firstPort){
	for (int i = 0; i < TOTAL_PORT; i++){
		servers[i].sockaddr_in.sin_family = AF_INET; // IPv4
		servers[i].sockaddr_in.sin_addr.s_addr = INADDR_ANY;
		servers[i].sockaddr_in.sin_port = htons(firstPort+i);
	}
}

void bindAllSocks(){
	for (int i = 0; i < TOTAL_PORT; i++){
		if (bind(sockfd[i], (const struct sockaddr *)&(servers[i].sockaddr_in),
				sizeof(servers[i].sockaddr_in)) < 0 ){
			perror("bind failed");
			exit(EXIT_FAILURE);
		}
	}
}

void createRandomPositionForDefensivePokemon(int chosenServer){
	defensivePokemons[chosenServer].path = rand() % 5;
	defensivePokemons[chosenServer].serverLocation = rand() % 4;
}

Pokemon createRandomPositionForEnemyPokemon(){
	Pokemon pokemon;
	pokemon.path = rand() % 4;
	pokemon.serverLocation = 0;
	return pokemon;
}

void insertDefensivePokemonsInTable(){
	clearDefensivePokemonsInTable();
	srand(time(NULL));
	for (int i=0;i<TOTAL_DEFENSIVE_POKEMONS;i++){
		do{
			createRandomPositionForDefensivePokemon(i);
		}while(verifyPositionInTableIsFill(defensivePokemons[i]) == TRUE);
		addPokemonInServer(defensivePokemons[i], defensivePokemons[i].serverLocation);
		totalDefensivePokemonsWasCreated++;
	}
}

int verifyCommandIsStart(char* comand){
	return strcmp("start\0", comand);
}

void printGameStarted(){
	char message[128] = "game started: path ";
	for (int i = 0; i < TOTAL_PORT; i++){
		char aux[10];
		itoa(i+1, aux);
		strcat(message,aux);
		sendMessageToClient(message);
		strcpy(message,"game started: path ");
	}
	printf("Game started message sent...\n");
}

void reverse(char s[]){
	int i, j;
	char c;

	for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}  

void itoa(int n, char s[])
 {
     int i, sign;

     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
}  

int receiveMessageFromClient(){
	len = sizeof(cliaddr); 
	printf("\nWaiting for message from client...\n");
	n = recvfrom(sockfd[0], (char *)buffer, MAXLINE,
				MSG_WAITALL, ( struct sockaddr *) &cliaddr,
				&len);
	buffer[n] = '\0';
	if (!verifyMessageIsEmpty(buffer))
		return FALSE;
	printf("Message received from client!\nContent: [%s]\n", buffer);
	return TRUE;
}

int verifyMessageIsEmpty(char message[]){
	if (strlen(buffer) == 0){
		printf("Received an empty message. ");
		return FALSE;
	}
	return TRUE;
}

void openCommunicationWithClient(){
	insertDefensivePokemonsInTable();
	clearEnemyPokemonsInTable();
	actualTurn = 0;
	actualID = 0;
	int currentRound;
	for (currentRound = 1;receiveMessageFromClient() && currentRound <= 50; currentRound++){
		if (verifyCommandIsStart(buffer) == 0){
			printGameStarted();
		}
		else{
			int totalSpaces = countHowManyWordsAreInTheMessage(buffer);
			if (totalSpaces == 0){
				if (strcmp(buffer,"getdefenders") == 0){
					makeActionWhenCommandEqualsGetDefenders();
				}
				else if (strcmp(buffer,"quit") == 0){
					sendGameOverMessage("2");
					break;
				}
				else{
					printf("Invalid command\n");
					sendGameOverMessage("1");
					break;
				}
			}
			else{
				fillReceivedCommands(totalSpaces);
				if (strcmp(receivedCommands[0],"getturn") == 0){
					if (createNewTurn()){
						makeActionWhenCommandEqualsGetTurn();
					}
					else{
						makeActionWhenInvalidTurn();
					}
				}
				else if (strcmp(receivedCommands[0],"shot") == 0){
					makeActionWhenCommandEqualsShot();
				}
				else{
					printf("Invalid command\n");
					sendGameOverMessage("1");
					break;
				}
			}	
		}
	}
	if (currentRound >= 50){
		sendGameOverMessage("0");
		printf("Server closed!\n");
		strcpy(status,"3");
	}

}

void sendMessageToClient(char message[]){
	sendto(sockfd[0], (const char *)message, strlen(message),
			MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
				len);
}

void makeActionWhenCommandEqualsGetDefenders(){
	char message[128] = "defender [";
	total = 0;
	for (int i=0; i<4;i++)
		getPokemonsPosition(i, message);
	strcat(message, "]");
	sendMessageToClient(message);
	printf("Pokemon position message was sent successfully!\n");
}

void makeActionWhenCommandEqualsGetTurn(){
	for (int i=0; i<4;i++){
		sendMessageToClient(mountTurnMessagePerServer(i));
	}
	printf("Message about turn %s was sent successfully!\n", receivedCommands[1]);
}

void makeActionWhenCommandEqualsShot(){
	sendMessageToClient(mountShotrespMessage());
	printf("Shotresp message was sent successfully!\n");
}

int countHowManyWordsAreInTheMessage(char message[]){
	int totalSpaces = 0;
	for (int character = 0; character < strlen(message); character++)
		if (message[character] == ' ')
			totalSpaces++;
	return totalSpaces;
}

void fillReceivedCommands(int totalSpaces){
	char buf_aux[MAXLINE];
	strcpy(buf_aux,buffer);
	char space[] = " ";
	char *auxreceivedCommands[5];
	auxreceivedCommands[0] = strtok(buf_aux,space);
	strcpy(receivedCommands[0], auxreceivedCommands[0]);
	for(int c=1 ; c < totalSpaces+1;c++){
		auxreceivedCommands[c] = strtok(NULL,space);
		strcpy(receivedCommands[c], auxreceivedCommands[c]);
	}
}

void addPokemonInServer(Pokemon pokemon, int chosenServer){
	int actualPosition = servers[chosenServer].totalPokemons;
	servers[chosenServer].pokemonsInBase[actualPosition] = pokemon;
	servers[chosenServer].totalPokemons++;
}

int verifyPositionInTableIsFill(Pokemon pokemon){
	for (int actualPokemon = 0; actualPokemon < totalDefensivePokemonsWasCreated; actualPokemon++){
		if ((defensivePokemons[actualPokemon].path == pokemon.path) && 
			(defensivePokemons[actualPokemon].serverLocation == pokemon.serverLocation)){
				return TRUE;
			}
	}
	return FALSE;
}

char* defensivePokemonToString(Pokemon pokemon, char message[]){
	char aux[10], aux2[10];
	itoa(pokemon.serverLocation+1, aux);
	strcat(message,"[");
	strcat(message,aux);
	strcat(message,", ");
	itoa(pokemon.path, aux2);
	strcat(message,aux2);
	strcat(message,"]");
	return message;
}

char* enemyPokemonToString(EnemyPokemon enemyPokemon){
	clearMessage(returnedMessage);
	char aux[10], aux2[10];
	itoa(enemyPokemon.id,aux);
	strcpy(returnedMessage, aux);
	strcat(returnedMessage, " ");
	strcat(returnedMessage, enemyPokemon.name);
	strcat(returnedMessage, " ");
	itoa(enemyPokemon.hits,aux2);
	strcat(returnedMessage, aux2);
	return returnedMessage;
}

char* getPokemonsPosition(int chosenServer, char m[]){
	char auxPokemon[128] = "[";
	for (int actualPokemon = 0; actualPokemon < servers[chosenServer].totalPokemons; actualPokemon++){
		strcpy(auxPokemon,"");
		strcat(m, defensivePokemonToString(servers[chosenServer].pokemonsInBase[actualPokemon],auxPokemon));
		if (total != 5){
			strcat(m,", ");
			total++;
		}
	}
	return m;
}

void clearMessage(char m[]){
	strcpy(m,"");
}

char* mountTurnMessagePerServer(int chosenServer){
	clearMessage(returnedMessage2);
	strcpy(returnedMessage2, "base ");
	char aux[10], aux2[10];
	itoa(chosenServer+1, aux);
	strcat(returnedMessage2, aux);
	strcat(returnedMessage2, "\n");
	for (int i=0;i<4;i++){
		strcat(returnedMessage2, "turn ");
		strcat(returnedMessage2, receivedCommands[1]);
		strcat(returnedMessage2, "\n");
		strcat(returnedMessage2, "fixedLocation ");
		itoa(i+1, aux2);
		strcat(returnedMessage2, aux2);
		if (i != 3)
			strcat(returnedMessage2, "\n");
		if (!verifyExistsEnemyInPosition(i,chosenServer)){
			if (i != 3){
				strcat(returnedMessage2, "\n");
			}
		}
		else{
			if (i == 3)
				strcat(returnedMessage2, "\n");
			strcat(returnedMessage2, enemyPokemonToString(getEnemyInPosition(i,chosenServer)));
			if (i != 3)
				strcat(returnedMessage2, "\n\n");
		}
	}
	return returnedMessage2;
}

int generateID(){
	return actualID++;
}

void generateNewRandomEnemyPokemon(){
	enemyPokemons[totalEnemyPokemonsWasCreated].id = generateID();
	enemyPokemons[totalEnemyPokemonsWasCreated].hits = 0;
	int randomNumber = rand() % 6;
	char name[POKENAME_MAX_SIZE] = "";
	if (randomNumber < 3){
		strcpy(enemyPokemons[totalEnemyPokemonsWasCreated].name, POKENAME_1);
		enemyPokemons[totalEnemyPokemonsWasCreated].hitsToDie = 1;
	}
	else if (randomNumber < 5){
		strcpy(enemyPokemons[totalEnemyPokemonsWasCreated].name, POKENAME_2);
		enemyPokemons[totalEnemyPokemonsWasCreated].hitsToDie = 2;
	}
	else{
		strcpy(enemyPokemons[totalEnemyPokemonsWasCreated].name, POKENAME_3);
		enemyPokemons[totalEnemyPokemonsWasCreated].hitsToDie = 3;
	}
	enemyPokemons[totalEnemyPokemonsWasCreated].pokemon = returnValidPositionToEnemyPokemon();
	printf("Enemy created!\n");
	totalEnemyPokemonsWasCreated++;
}

Pokemon returnValidPositionToEnemyPokemon(){
	Pokemon pokemon = createRandomPositionForEnemyPokemon();
	for (int actualPokemon = 0; actualPokemon < totalEnemyPokemonsWasCreated; actualPokemon++){
		if ((enemyPokemons[actualPokemon].pokemon.path == pokemon.path) && enemyPokemons[actualPokemon].pokemon.serverLocation == 0){
			pokemon = createRandomPositionForEnemyPokemon();
			actualPokemon = 0;
		}
	}
	return pokemon;
}

int verifyIfHasSpaceToCreateNewEnemyPokemon(){
	int totalEnemiesInFirstLocation = 0;
	for (int actualPokemon = 0; actualPokemon < totalEnemyPokemonsWasCreated; actualPokemon++){
		if (enemyPokemons[actualPokemon].pokemon.serverLocation == 0){
			totalEnemiesInFirstLocation++;
		}
	}
	if (totalEnemiesInFirstLocation < 4){
		return TRUE;
	}
	else{
		return FALSE;
	}
}

int verifyExistsEnemyInPosition(int path,int location){
	for (int actualPokemon = 0; actualPokemon < totalEnemyPokemonsWasCreated; actualPokemon++){
		if ((enemyPokemons[actualPokemon].pokemon.path == path) && enemyPokemons[actualPokemon].pokemon.serverLocation == location){
			return TRUE;
		}
	}
	return FALSE;
}

EnemyPokemon getEnemyInPosition(int path,int location){
	EnemyPokemon enemyPokemonReturned;
	for (int actualPokemon = 0; actualPokemon < totalEnemyPokemonsWasCreated; actualPokemon++){
		if ((enemyPokemons[actualPokemon].pokemon.path == path) && enemyPokemons[actualPokemon].pokemon.serverLocation == location){
			enemyPokemonReturned = enemyPokemons[actualPokemon];
		}
	}
	return enemyPokemonReturned;
}

char* mountShotrespMessage(){
	clearMessage(returnedMessage2);
	strcpy(returnedMessage2, "shotresp ");
	strcat(returnedMessage2,receivedCommands[1]);
	strcat(returnedMessage2, " ");
	strcat(returnedMessage2,receivedCommands[2]);
	strcat(returnedMessage2, " ");
	strcat(returnedMessage2,receivedCommands[3]);
	strcat(returnedMessage2, " ");
	char aux[2];
	itoa(verifyIfEnemyCanBeTargeted(atoi(receivedCommands[1]),atoi(receivedCommands[2])),aux);
	strcat(returnedMessage2, aux);

	return returnedMessage2;
}

int verifyIfEnemyCanBeTargeted(int location, int path){
	Pokemon pokemon;
	pokemon.path = path;
	pokemon.serverLocation = location-1;
	int id = atoi(receivedCommands[3]);
	if (verifyPositionInTableIsFill(pokemon) && verifyExistsEnemyByID(id)){
		EnemyPokemon enemyPokemon = getEnemyByID(id);
		if (enemyPokemon.pokemon.serverLocation == pokemon.serverLocation){
			if (pokemon.path == 0 && enemyPokemon.pokemon.path == 0){
				shotInEnemyPokemonByID(id);
				return SUCESS;
			}
			else if (pokemon.path == 4 && enemyPokemon.pokemon.path == 3){
				shotInEnemyPokemonByID(id);
				return SUCESS;
			}
			else if (pokemon.path > 0 && pokemon.path < 4){
				if (pokemon.path == enemyPokemon.pokemon.path || pokemon.path == enemyPokemon.pokemon.path+1){
					shotInEnemyPokemonByID(id);
					return SUCESS;
				}
			}
		}
	}
	return FAILED;
}

int verifyExistsEnemyByID(int id){
	for (int actualPokemon = 0; actualPokemon < totalEnemyPokemonsWasCreated; actualPokemon++){
		if (enemyPokemons[actualPokemon].id == id){
			return TRUE;
		}
	}
	return FALSE;
}

EnemyPokemon getEnemyByID(int id){
	EnemyPokemon returnedPokemon;
	for (int actualPokemon = 0; actualPokemon < totalEnemyPokemonsWasCreated; actualPokemon++){
		if (enemyPokemons[actualPokemon].id == id){
			returnedPokemon = enemyPokemons[actualPokemon];
		}
	}
	return returnedPokemon;
}

void shotInEnemyPokemonByID(int id){
	for (int actualPokemon = 0; actualPokemon < totalEnemyPokemonsWasCreated; actualPokemon++){
		if (enemyPokemons[actualPokemon].id == id){
			printf("%s received a shot\n", enemyPokemons[actualPokemon].name);
			enemyPokemons[actualPokemon].hits++;
			if (enemyPokemons[actualPokemon].hits >= enemyPokemons[actualPokemon].hitsToDie){
				clearEnemyPokemonsInTableByID(id);
			}

		}
	}
}

void clearEnemyPokemonsInTableByID(int id){
	EnemyPokemon auxEnemyPokemons[20];
	int x = 0;
	for (int actualPokemon = 0; actualPokemon < totalEnemyPokemonsWasCreated; actualPokemon++){
		if (enemyPokemons[actualPokemon].id != id){
			auxEnemyPokemons[actualPokemon-x] = enemyPokemons[actualPokemon];
		}
		else{
			x++;
			printf("%s was killed\n", enemyPokemons[actualPokemon].name);
			totalEnemiesKilled++;
		}
	}
	clearEnemyPokemonsInTable();
	totalEnemyPokemonsWasCreated--;
	for (int actualPokemon = 0; actualPokemon < totalEnemyPokemonsWasCreated; actualPokemon++){
		enemyPokemons[actualPokemon] = auxEnemyPokemons[actualPokemon];
	}	
}

void clearEnemy(int address){
	enemyPokemons[address].hits = -1;
	enemyPokemons[address].hitsToDie = -1;
	enemyPokemons[address].id = -1;
	strcpy(enemyPokemons[address].name , "");
	enemyPokemons[address].pokemon.path = -1;
	enemyPokemons[address].pokemon.serverLocation = -1;
}

int createNewTurn(){
	if (actualTurn == atoi(receivedCommands[1])){
		for (int actualPokemon = 0; actualPokemon < totalEnemyPokemonsWasCreated; actualPokemon++){
		makesEnemyWalkFoward(actualPokemon);
		enemyPokemons[actualPokemon].hits = 0;		
		}
		for (int i = 0; i < 4; i++){
			int createEnemy = rand() % 100;
			if (verifyIfHasSpaceToCreateNewEnemyPokemon() && createEnemy < 40){
				generateNewRandomEnemyPokemon();
			}
		}
		actualTurn++;
		printf("New turn!\n");
		return TRUE;
	}
	printf("Invalid turn!\n");
	return FALSE;
}

void sendGameOverMessage(char s[]){
	time_t diff = time(NULL) - start;
	
	clearMessage(returnedMessage);
	strcpy(returnedMessage,"gameover ");
	if (strcmp(s,"2") == 0){
		strcat(returnedMessage, "1");
	}
	else{
		strcat(returnedMessage, s);
	}
	strcat(returnedMessage," ");
	char aux[10],aux2[10],aux3[10];
	itoa(totalEnemiesKilled,aux);
	strcat(returnedMessage,aux);
	strcat(returnedMessage," ");
	itoa(enemiesThatReachedPokedex,aux2);
	strcat(returnedMessage,aux2);
	strcat(returnedMessage," ");
	itoa(diff,aux3);
	strcat(returnedMessage,aux3);
	
	sendMessageToClient(returnedMessage);
	strcpy(status,s);
}

void makeActionWhenInvalidTurn(){
	sendMessageToClient("Turn invalid, try to keep sequential values.");
}

void clearEnemyPokemonsInTable(){
	for (int i=0;i<20;i++){
		clearEnemy(i);
	}
}

void makesEnemyWalkFoward(int position){
	if (enemyPokemons[position].pokemon.serverLocation == 3){
		makesActionWhenEnemyReachPokedex(enemyPokemons[position].id);
	}
	else{
		enemyPokemons[position].pokemon.serverLocation++;
	}
}

void makesActionWhenEnemyReachPokedex(int id){
	EnemyPokemon auxEnemyPokemons[20];
	int x = 0;
	for (int actualPokemon = 0; actualPokemon < totalEnemyPokemonsWasCreated; actualPokemon++){
		if (enemyPokemons[actualPokemon].id != id){
			auxEnemyPokemons[actualPokemon-x] = enemyPokemons[actualPokemon];
		}
		else{
			x++;
			printf("%s reached the pokedex\n", enemyPokemons[actualPokemon].name);
			enemiesThatReachedPokedex++;
		}
	}
	clearEnemyPokemonsInTable();
	totalEnemyPokemonsWasCreated--;
	for (int actualPokemon = 0; actualPokemon < totalEnemyPokemonsWasCreated; actualPokemon++){
		enemyPokemons[actualPokemon] = auxEnemyPokemons[actualPokemon];
	}	
}

void closeCommunication(){
	for (int i=0;i<4;i++){
		close(sockfd[i]);
	}
}

void clearDefensivePokemonsInTable(){
	for (int i=0;i<6;i++){
		defensivePokemons[i].path = -1;
		defensivePokemons[i].serverLocation = -1;
	}
	for (int i=0;i<4;i++){
		for (int j=0;j<6;j++){
			servers[i].pokemonsInBase[j].path = -1;
			servers[i].pokemonsInBase[j].serverLocation = -1;
		}
		servers[i].totalPokemons = 0;
	}
	totalDefensivePokemonsWasCreated = 0;
}

void waitCommandAfterGameOver(){
	if (strcmp(buffer,"quit") == 0){
		return;
	}
	receiveMessageFromClient();
	if (strcmp(buffer,"start") == 0){
		strcpy(status,"1");
		printGameStarted();
	}
	else{
		strcpy(status,"0");
		sendMessageToClient("end");
	}
}
