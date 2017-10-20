/* @author: Gustavo Henrique de Oliveira Santos
 *
 * Implementacao do jogo Snake em C usando a Pthreds
 *
 * Para compilar em linha de comando:
 * gcc -pthread snakeGame.c
 */


#include <stdio_ext.h>
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

void atualizaCampo(char campo[TAM_CAMPO] [TAM_CAMPO], Ponto cobra[TAM_MAX_COBRA], int *tamCobra, int *score, int velocidade, int newX, int newY){
	int pontuou=0;
	int i;
	Ponto aux1, aux2;
	
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
		(*score) += velocidade;
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


void exibeCampo(char campo[TAM_CAMPO] [TAM_CAMPO], int score){
	int x, y;

	printf("\e[H\e[2J");
		
	printf("Score: %d\n\n", score);

	for(x=0; x < TAM_CAMPO; ++x){
		for(y=0; y<TAM_CAMPO; ++y){
			printf("%c ", campo[x][y]);
		}
		printf("\n");
	}
}


void terminaJogo(char campo[TAM_CAMPO] [TAM_CAMPO], int score){
	int x, y;
	
	for(x=0; x < TAM_CAMPO; ++x){
		for(y=0; y<TAM_CAMPO; ++y){
			campo[x][y] = '*';
			
			exibeCampo(campo, score);
			
			usleep(11000);
		}
	}
}


void loopJogo(int velocidade){
	int score=0;
	int newX, newY;
	char direcao = DIREITA;;
	char campo[TAM_CAMPO] [TAM_CAMPO];
	
	int tamCobra = 4;
	
	pthread_t threadLeitura;
	int retPthread_create;
	
	
	retPthread_create = pthread_create(&threadLeitura, NULL, leituraTeclado, (void*) &direcao);
	
	if(retPthread_create){
		printf("Erro em pthred_create() retornando %d\n", retPthread_create);
		exit(1);
	}
	
	Ponto cobra[TAM_MAX_COBRA];
	
	geraCampo(campo);
	
	geraCobra(cobra);
	
	adicionaCobra(cobra, tamCobra, campo);
	
	adicionaComida(campo);
	
	while(1){
		obtemNovaPosicao(cobra[0].x, cobra[0].y, &direcao, &newX, &newY);
	
		if(campo[newY][newX] == 'o'){
			pthread_cancel(threadLeitura);
			
			pthread_join(threadLeitura, NULL);
			
			terminaJogo(campo, score);
			
			break;
		}
	
		atualizaCampo(campo, cobra, &tamCobra, &score, velocidade, newX, newY);
	
		exibeCampo(campo, score);
	
		// Espera 0.45 segundos para atualizar o campo de jogo
		usleep(600000 - (velocidade * 100000));
	}
}

void opcoes(int *velocidade){
	do{
		printf("\n\n\n   Velocidade\n");
		printf("   1 - 2 - 3 - 4 - 5\n\n");
	
		scanf("%d", velocidade);
	}while(*velocidade < 1 || *velocidade > 5);
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

int menu(){
	int op;
	
	printf("\n\n\n   1 - Novo jogo\n");
	printf("   2 - Opcoes\n");
	//printf("   3 - Recordes\n");
	printf("   0 - Sair\n");
	printf("\n   Opcao: ");
	
	scanf("%d", &op);
	
	return op;
}

int main(){
	struct termios oldt;
	int op, velocidade = 3;

	tcgetattr( STDIN_FILENO, &oldt );

	inicio();

	do{
		printf("\e[H\e[2J");
		
		__fpurge(stdin);
		op = menu();
		
		printf("\e[H\e[2J");
		
		switch(op){
			case 1:
				loopJogo(velocidade);
				
				tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
				
				
				break;
			
			case 2:
				opcoes(&velocidade);
				break;
		}
	
	}while(op);

	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );

	return 0;
}
