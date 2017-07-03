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
#define DIREITA 'D'
#define ESQUERDA 'A'
#define CIMA 'W'
#define BAIXO 'S'


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
	
	campo[1] [1] = 'o';
	campo[1] [2] = 'o';
	campo[1] [3] = 'o';
	campo[1] [4] = '@';
	
	campo[TAM_CAMPO-2] [TAM_CAMPO-2] = 'O';
	
	campo[0] [0] = '╔';
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

void atualizaCampo(char *direcao, char campo[TAM_CAMPO] [TAM_CAMPO], int *score){
	int x, y;
	int pontuou=0;

	for(y=1; y < TAM_CAMPO-1; ++y){
		for(x=1; x<TAM_CAMPO-1; ++x){
		
			if(campo[y][x] == '@'){
				campo[y][x] = 'o';
			
				if(*direcao == DIREITA){
					if(x+2 == TAM_CAMPO){
						if(campo[y][1] == 'O'){
							++(*score);
							pontuou=1;
						}
					
						campo[y][1] = '@';
					} else {
						if(campo[y][x+1] == 'O'){
							++(*score);
							pontuou=1;
						}
						
						campo[y][x+1] = '@';
					}
				} else if(*direcao == ESQUERDA){
					if(x-1 == 0){
						if(campo[y] [TAM_CAMPO-2] == 'O'){
							++(*score);
							pontuou=1;
						}
						
						campo[y] [TAM_CAMPO-2] = '@';
					} else {
						if(campo[y][x-1] == 'O'){
							++(*score);
							pontuou=1;
						}
						
						campo[y][x-1] = '@';
					}
				} else if(*direcao == BAIXO){
					if(y+2 == TAM_CAMPO){
						if(campo[1][x] == 'O'){
							++(*score);
							pontuou=1;
						}
						
						campo[1][x] = '@';
					} else {
						if(campo[y+1][x] == 'O'){
							++(*score);
							pontuou=1;
						}
						
						campo[y+1][x] = '@';
					}
				} else if(*direcao == CIMA){
					if(y-1 == 0){
						if(campo[TAM_CAMPO-2] [x] == 'O'){
							++(*score);
							pontuou=1;
						}
						
						campo[TAM_CAMPO-2] [x] = '@';
					} else {
						if(campo[y-1] [x] == 'O'){
							++(*score);
							pontuou=1;
						}
						
						campo[y-1] [x] = '@';
					}
				}
				
				if(pontuou){
					adicionaComida(campo);
				} else {
					moveCauda(campo);
				}
			
				return;
			}
		}
	}
}

void loopJogo(char *direcao, char campo[TAM_CAMPO][TAM_CAMPO]){
	int x, y;
	int score=0;
	
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
	char campo[TAM_CAMPO] [TAM_CAMPO];
	char direcao = DIREITA;
	
	pthread_t threadLeitura;
	int retPthread_create;
	
	
	geraCampo(campo);
	
	
	retPthread_create = pthread_create(&threadLeitura, NULL, leituraTeclado, (void*) &direcao);
	
	if(retPthread_create){
		printf("Erro em pthred_create() retornando %d\n", retPthread_create);
		exit(1);
	}


	loopJogo(&direcao, campo);
	
	pthread_join(threadLeitura, NULL);

	return 0;
}
