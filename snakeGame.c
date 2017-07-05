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
#define TAM_MAX_COBRA (TAM_CAMPO-1) * (TAM_CAMPO-1)

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

void moveCauda(char campo[TAM_CAMPO][TAM_CAMPO]){
	int x, y;
	int somaLaterais;

	for(y=1; y < TAM_CAMPO-1; ++y){
		for(x=1; x<TAM_CAMPO-1; ++x){
		
			if(campo[y][x] == 'o'){
			
				somaLaterais=0;
			
				if(campo[y+1][x] == 'o' || campo[y+1][x] == '@'){
					++somaLaterais;
				}
				
				if(campo[y][x+1] == 'o' || campo[y][x+1] == '@'){
					++somaLaterais;
				}
				
				if(campo[y-1][x] == 'o' || campo[y-1][x] == '@'){
					++somaLaterais;
				}
				
				if(campo[y][x-1] == 'o' || campo[y][x-1] == '@'){
					++somaLaterais;
				}
				
				if(somaLaterais == 1){
					campo[y][x] = ' ';
					
					return;
				}
			
			}
		}
	}
}

void obtemPosicao(int *x, int *y, char campo[TAM_CAMPO] [TAM_CAMPO]){

	for(*y=1; *y < TAM_CAMPO-1; ++(*y)){
		for(*x=1; *x < TAM_CAMPO-1; ++(*x)){
		
			if(campo[*y][*x] == '@'){
				return;
			}
		}
	}
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

void atualizaCampo(char *direcao, char campo[TAM_CAMPO] [TAM_CAMPO], int *score){
	int x, y, newX, newY;
	int pontuou=0;
	
	obtemPosicao(&x, &y, campo);
	
	obtemNovaPosicao(x, y, direcao, &newX, &newY);
	
	if(campo[newY][newX] == 'O'){
		pontuou=1;
	}
	
	campo[y][x] = 'o';
	
	x = newX;
	y = newY;
	
	campo[y][x] = '@';
	
	if(pontuou){
		++(*score);
		adicionaComida(campo);
	} else {
		moveCauda(campo);
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
		cobra[i].x = posX;
		cobra[i].y = posY-i;
	}
}

void adicionaCobra(Ponto cobra[TAM_MAX_COBRA], int tamCobra, char campo[TAM_CAMPO] [TAM_CAMPO]){
	int i;
	
	campo[cobra[0].x] [cobra[0].y] = '@';
	
	for(i=1; i<tamCobra; ++i){
		campo[cobra[i].x] [cobra[i].y] = 'o';
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
		atualizaCampo(direcao, campo, &score);
	
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
	char aux, *direcao;
	
	direcao = (char *) ptr;
	
	while(1){
		aux = getch_echo(0);
		
		aux = toupper(aux);
	
		if(aux == DIREITA || aux == ESQUERDA || aux == CIMA || aux == BAIXO){
			*direcao = aux;
		}
	}
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


	loopJogo(&direcao);
	
	pthread_join(threadLeitura, NULL);

	return 0;
}
