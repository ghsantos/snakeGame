/* @author: Gustavo Henrique de Oliveira Santos
 *
 * Implementacao do jogo Snake em C usando a Pthreds
 *
 * Para compilar em linha de comando:
 * gcc -pthread snakeGame.c
 */


#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>

#define TAM_CAMPO 16
#define TAM_MAX_COBRA (TAM_CAMPO) * (TAM_CAMPO)

#define DIREITA 'D'
#define ESQUERDA 'A'
#define CIMA 'W'
#define BAIXO 'S'

typedef struct Ponto {
	int x;
	int y;
} Ponto;


int getch_echo(int echo){
	struct termios oldt, newt;
	int ch;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~ICANON;
	
	if(echo){
		newt.c_lflag &=  ECHO;
	} else{
		newt.c_lflag &= ~ECHO;
	}
	
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}


void geraCampo(char campo[TAM_CAMPO][TAM_CAMPO]){

	int x, y;
	
	for(x=0; x < TAM_CAMPO; ++x){
		for(y=0; y<TAM_CAMPO; ++y){
			if(x == 0 || y == 0 || x == TAM_CAMPO-1 || y == TAM_CAMPO-1){
				campo[x][y] = '*';
			} else {
				campo[x] [y] = ' ';
			}
		}
	}
}

void adicionaComida(char campo[TAM_CAMPO][TAM_CAMPO]){
	int posX, posY;
	
	srand((unsigned)time(NULL));
	
	do{
		posX = rand()%(TAM_CAMPO-2) + 1;
		usleep(100);
		posY = rand()%(TAM_CAMPO-2) + 1;
	
		if(campo[posY][posX] == ' '){
			campo[posY][posX] = 'O';
		}
	}while(campo[posY][posX] != 'O');
}

void obtemNovaPosicao(int x, int y, char *direcao, int *newX, int *newY){
	*newX = x;
	*newY = y;
	
	if(*direcao == DIREITA){
		++(*newX);
	} else if(*direcao == ESQUERDA){
		--(*newX);
	} else if(*direcao == BAIXO){
		++(*newY);
	} else if(*direcao == CIMA){
		--(*newY);
	}
	
	if((*newX)+1 == TAM_CAMPO){
		*newX = 1;
	} else if((*newY)+1 == TAM_CAMPO){
		*newY = 1;
	} else if(*newX == 0){
		*newX = TAM_CAMPO -2;
	} else if(*newY == 0){
		*newY = TAM_CAMPO -2;
	}
}

void atualizaCampo(char *direcao, char campo[TAM_CAMPO] [TAM_CAMPO], Ponto cobra[TAM_MAX_COBRA], int *tamCobra, int *score){
	int newX, newY;
	int pontuou=0;
	int i;
	Ponto aux1, aux2;
	
	obtemNovaPosicao(cobra[0].x, cobra[0].y, direcao, &newX, &newY);
	
	if(campo[newY][newX] == 'O'){
		pontuou=1;
	}
	
	campo[cobra[0].y][cobra[0].x] = 'o';
	
	
	aux2 = cobra[0];
	
	for(i=1; i<(*tamCobra); ++i){
		aux1 = cobra[i];
		
		cobra[i] = aux2;
		
		aux2 = aux1;
	}
	
	cobra[0].x = newX;
	cobra[0].y = newY;
	
	campo[cobra[0].y][cobra[0].x] = '@';
	
	if(pontuou){
		++(*score);
		++(*tamCobra);
		adicionaComida(campo);
	} else {
		campo[cobra[(*tamCobra)-1].y][cobra[(*tamCobra)-1].x] = ' ';
	}
}

void geraCobra(Ponto cobra[TAM_MAX_COBRA]){
	int posX, posY, i;
	
	srand((unsigned)time(NULL));
	
	posX = rand()%(TAM_CAMPO-7) + 4;
	usleep(100);
	posY = rand()%(TAM_CAMPO-7) + 4;
	
	cobra[0].x = posX;
	cobra[0].y = posY;
	
	for(i=0; i < 4; ++i){
		cobra[i].x = posX-i;
		cobra[i].y = posY;
	}
}

void adicionaCobra(Ponto cobra[TAM_MAX_COBRA], int tamCobra, char campo[TAM_CAMPO] [TAM_CAMPO]){
	int i;
	
	campo [cobra[0].y][cobra[0].x] = '@';
	
	for(i=1; i<tamCobra-1; ++i){
		campo [cobra[i].y][cobra[i].x] = 'o';
	}
}

void loopJogo(char *direcao){
	int x, y;
	int score=0;
	
	char campo[TAM_CAMPO] [TAM_CAMPO];
	
	int tamCobra = 4;
	Ponto cobra[TAM_MAX_COBRA];
	
	geraCampo(campo);
	
	geraCobra(cobra);
	
	adicionaCobra(cobra, tamCobra, campo);
	
	adicionaComida(campo);
	
	while(1){
		atualizaCampo(direcao, campo, cobra, &tamCobra, &score);
	
		printf("\e[H\e[2J");
		
		printf("Score: %d\n\n", score);
	
		for(x=0; x < TAM_CAMPO; ++x){
			for(y=0; y<TAM_CAMPO; ++y){
				printf("%c ", campo[x][y]);
			}
			printf("\n");
		}
	
		// Espera 0.5 segundos para atualizar o campo de jogo
		usleep(500000);
	}
}

void *leituraTeclado(void *ptr){
	char aux, *direcao, direcaoAnterior;
	
	direcao = (char *) ptr;
	
	direcaoAnterior = *direcao;
	
	while(1){
		aux = getch_echo(0);
		
		aux = toupper(aux);
	
		if((aux == DIREITA && aux != *direcao && direcaoAnterior != ESQUERDA) ||
		   (aux == ESQUERDA && aux != *direcao && direcaoAnterior != DIREITA) ||
		   (aux == CIMA && aux != *direcao && direcaoAnterior != BAIXO) ||
		   (aux == BAIXO && aux != *direcao && direcaoAnterior != CIMA)){
			*direcao = aux;
			direcaoAnterior = *direcao;
		}
	}
}

void inicio(){

	printf("\e[H\e[2J");
	
	printf("\n\n\n\n");
	
	printf("     .oooooo..o                       oooo                  \n");
	printf("    d8P'    `Y8                       `888                  \n");
	printf("    Y88bo.      ooo. .oo.    .oooo.    888  oooo   .ooooo.  \n");
	printf("     `\"Y8888o.  `888P\"Y88b  `P  )88b   888 .8P'   d88' `88b \n");
	printf("         `\"Y88b  888   888   .oP\"888   888888.    888ooo888 \n");
	printf("    oo     .d8P  888   888  d8(  888   888 `88b.  888    .o \n");
	printf("    8\"\"88888P'  o888o o888o `Y888\"\"8o o888o o888o `Y8bod8P'\n");

	sleep(2);
}

int main(){
	char direcao = DIREITA;
	
	pthread_t threadLeitura;
	int retPthread_create;
	
	
	retPthread_create = pthread_create(&threadLeitura, NULL, leituraTeclado, (void*) &direcao);
	
	if(retPthread_create){
		printf("Erro em pthred_create() retornando %d\n", retPthread_create);
		exit(1);
	}

	inicio();

	loopJogo(&direcao);
	
	pthread_join(threadLeitura, NULL);

	return 0;
}
