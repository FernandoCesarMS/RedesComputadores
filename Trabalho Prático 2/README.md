Para compilar o projeto, basta executar o comando make e os comandos referentes ao server e ao client,
como no exemplo abaixo. O server e o client deve ser executados em terminais diferentes.
Exemplo:
make
./server v4 9000
./client 127.0.0.1 9000 start

Após a conexão estabelecida, o trabalho inicia. 

O comando "getturn x" funciona de maneira sequencial, ou seja, não é possível começar utilizando o 
comando getturn 2, por exemplo. Deve-se sempre começar com getturn 0, depois getturn 1 e assim por diante.

Após a inserção de um comando invalido, é retornado gameover, porém é possível iniciar novamente digitando
o comando start