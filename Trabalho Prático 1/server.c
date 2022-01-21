#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 500
#define TAMANHO_POKEDEX 40
#define TAMANHO_NOME 11
#define EXISTE 1
#define NAO_EXISTE -1
#define STRING_VALIDA 0
#define STRING_INVALIDA -2
#define ADD "add"
#define REMOVE "remove"
#define LIST "list"
#define EXCHANGE "exchange"

char* limpaString(char string_temporaria[]){
    for (int c=0; c < TAMANHO_NOME; c++){
        if (string_temporaria[c] == '\n' || string_temporaria[c] == '\0'){
            string_temporaria[c] = '\0';
            return string_temporaria;
        }
    }
    return "";
}

int verificaExisteEntrada(char pokemon[TAMANHO_NOME], char pokedex[TAMANHO_POKEDEX][TAMANHO_NOME]){
    for (int c = 0; c < TAMANHO_POKEDEX; c++){
        if (strcmp(limpaString(pokemon),limpaString(pokedex[c])) == 0)
            return EXISTE;
    }
    return NAO_EXISTE;
}

int contaNumeroEntradas(char string_temporaria[]){
    int retorno = 1, contador = 0;

    while(string_temporaria[contador] != '\n'){
        if (string_temporaria[contador] == ' ')
            retorno += 1;
        contador += 1;
    }

    return retorno;
}

int validaString(char nome_pokemon[TAMANHO_NOME]){
    for (int c2 = 0; nome_pokemon[c2] != '\0' && nome_pokemon[c2] != '\n';c2++){
        if (!((nome_pokemon[c2] >= 'a' && nome_pokemon[c2] <= 'z') || (nome_pokemon[c2] >= '0' && nome_pokemon[c2] <= '9')))
            return STRING_INVALIDA;
    }
    return STRING_VALIDA;
}

int adicionaEntrada(char nome_pokemon[TAMANHO_NOME], char pokedex[TAMANHO_POKEDEX][TAMANHO_NOME]){
    if (verificaExisteEntrada(nome_pokemon,pokedex) != NAO_EXISTE){
        return NAO_EXISTE;
    }
    else {
        for (int c = 0; c < TAMANHO_POKEDEX; c++){
            if (pokedex[c][0] == '\0'){
                if (validaString(nome_pokemon) == STRING_INVALIDA)
                    return STRING_INVALIDA;
                strcpy(pokedex[c], nome_pokemon);
                return 1;
            }
        }
    }
        
    return 0;
}

void listaEntradas(char buff[], char pokedex[TAMANHO_POKEDEX][TAMANHO_NOME]){
    char saida[BUFSZ] = "";
    int quantEntradas = 0;
    for (int c = 0; c < TAMANHO_POKEDEX; c++){
        if (pokedex[c][0] != '\0'){
            strcat(saida, limpaString(pokedex[c]));
            strcat(saida, " ");
            quantEntradas++;
        }
    }
    int tamanhoSaida = 0;
    for (int c = 0; saida[c] != '\0';c++){
        tamanhoSaida++;
    }
    saida[tamanhoSaida-1] = '\0';    
    if (quantEntradas != 0)
        sprintf(buff,"%s\n", saida);
    else
        sprintf(buff,"none\n");
}

int removeEntrada(char nome_pokemon[TAMANHO_NOME], char pokedex[TAMANHO_POKEDEX][TAMANHO_NOME]){
    if (validaString(nome_pokemon) == STRING_VALIDA){
        if (verificaExisteEntrada(nome_pokemon,pokedex) == NAO_EXISTE){
            return NAO_EXISTE;
        }
        else {
            for (int c = 0; c < TAMANHO_POKEDEX; c++){
                if (strcmp(limpaString(nome_pokemon),limpaString(pokedex[c])) == 0){
                    strcpy(pokedex[c], "");
                    pokedex[c][0] = '\0';
                    return 1;
                }
            }
        }
    }
    else{
        return -2;
    }
        
    return 0;
}

int trocaPokemon(char nome_pokemon[TAMANHO_NOME],char nome_pokemon2[TAMANHO_NOME], char pokedex[TAMANHO_POKEDEX][TAMANHO_NOME]){
    if (validaString(nome_pokemon) == STRING_VALIDA && validaString(nome_pokemon2) == STRING_VALIDA){
        if (verificaExisteEntrada(nome_pokemon,pokedex) == NAO_EXISTE){
            return NAO_EXISTE;
        }else if (verificaExisteEntrada(nome_pokemon2,pokedex) == EXISTE){
            return -2;
        }else{
            for (int c = 0; c < TAMANHO_POKEDEX; c++){
                if (strcmp(limpaString(nome_pokemon),limpaString(pokedex[c])) == 0){
                    strcpy(pokedex[c], nome_pokemon2);
                    return 1;
                }
            }
        }
    }
    else{
        if (validaString(nome_pokemon) == STRING_INVALIDA){
            return -3;
        }else{
            return -4;
        }
    }
        
    return 0;
}

void handle(char buf[],char pokedex[TAMANHO_POKEDEX][TAMANHO_NOME]){
    char buf_temporario[BUFSZ];
    strcpy(buf_temporario, buf);
    memset(buf,0, BUFSZ);
    int posAtual = 0;
    int numeroPalavras = contaNumeroEntradas(buf_temporario);
    char *entradas[numeroPalavras];
    char delim[] = " ";
    
    entradas[0] = strtok(buf_temporario,delim);
    for (posAtual = 1; posAtual < numeroPalavras; posAtual++){
        entradas[posAtual] = strtok(NULL,delim);
    }

    if (strcmp(ADD,entradas[0]) == 0){
        char saida[BUFSZ] = "";
        for (int c = 1; c < numeroPalavras; c++){
            if (c > 1)
                strcat(saida, " ");
            int auxAdiciona = adicionaEntrada(entradas[c],pokedex);
            if (auxAdiciona == 1){
                strcat(saida, limpaString(entradas[c]));
                strcat(saida, " added");
            }
            else if (auxAdiciona == -1){
                strcat(saida, limpaString(entradas[c]));
                strcat(saida, " already exists");
            } else if (auxAdiciona == -2){
                strcat(saida, "invalid message");
            } else{
                strcat(saida, "limit exceeded");
            }
        }
        sprintf(buf,"%s\n", saida);
    } else if(strcmp(REMOVE,entradas[0]) == 0){
        char saida[BUFSZ] = "";
        int auxRemove = removeEntrada(entradas[1],pokedex);
        if (auxRemove == 1){
            strcat(saida, limpaString(entradas[1]));
            strcat(saida, " removed");
        } else if (auxRemove == -1){
            strcat(saida, limpaString(entradas[1]));
            strcat(saida, " does not exist");
        }else if (auxRemove == -2){
            strcat(saida, "invalid message");
        }
        sprintf(buf,"%s\n", saida);
    } else if (!strcmp("list\n",entradas[0]) || !strcmp(LIST,entradas[0])){
        listaEntradas(buf, pokedex);
    } else if (!strcmp(EXCHANGE,entradas[0])){
        char saida[BUFSZ] = "";
        int auxTroca = trocaPokemon(entradas[1],entradas[2],pokedex);
        if (auxTroca == 1){
            strcat(saida, limpaString(entradas[1]));
            strcat(saida, " exchanged\n");
        }else if (auxTroca == -1){
            strcat(saida, limpaString(entradas[1]));
            strcat(saida, " does not exist\n");
        }else if (auxTroca == -2){
            strcat(saida, limpaString(entradas[2]));
            strcat(saida, " already exists\n");
        }else if (auxTroca == -3 || auxTroca == -4){
            strcat(saida, "invalid message\n");
        }
        sprintf(buf,"%s", saida);
    }else{
        exit(EXIT_SUCCESS);
    }
}

int multiplasMensagens(int csock, char *buf){
    size_t count = 0;
    memset(buf, 0, BUFSZ);
    while (buf[strlen(buf) - 1] != '\n') 
        count += recv(csock, buf + count, BUFSZ - count, 0);
    return count;
}

void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    char MAIN_DATA[TAMANHO_POKEDEX][TAMANHO_NOME];
    for (int c = 0; c < 40; c++){
        for (int c2 = 0; c2 < 10; c2++){
            MAIN_DATA[c][c2] = '\0';
        }
    }

    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("[log] connection from %s\n", caddrstr);

        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);
        size_t count;

        while(1){
            memset(buf, 0, BUFSZ);  
            count = multiplasMensagens(csock, buf);
            printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);    

            handle(buf, MAIN_DATA);
            count = send(csock, buf, strlen(buf), 0);
            if (count != strlen(buf)) {
                logexit("send");
            }
        }
        close(csock);   
    }

    exit(EXIT_SUCCESS);
}
