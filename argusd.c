#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>   /*chamadas ao sistema: defs e decls essenciais*/
#include <fcntl.h>    /*O_RDONLY, O_WRONLY, O_CREAT, O_* */

#include "argus.h"

#define MAXPROGEXEC 100

#define OCUPADO 1
#define LIVRE 0

////////////////////////////////////////////////////
//Variaveis globais utilizadas para o funcionamento do servidor
int tempoInactividade = 10;
int tempoExecucao = 20;

int vagasExecutar = MAXPROGEXEC;
	
char comando[MAXPROGEXEC][MAX];
char fifoResposta[MAX];

int estado[MAXPROGEXEC];
pid_t filhoComandaExecucao[MAXPROGEXEC];
pid_t filhoExecuta = -1;
pid_t filhospipes[MAX];
pid_t filhospipesAux = 0;

int terminacaoForcada = 0;
////////////////////////////////////////////////////

//Funcao responsavel por receber o alarme de que o tempo maximo de execuçao foi atingido
void tempoExecucaoAtingido(int x){
	kill(filhoExecuta, SIGINT);
}

//Funcao responsavel por matar os filhos que estao a executar, uma vez recebido a instrucao para paragem forçada
void terminaForcado(int x){
	terminacaoForcada = 1;
	kill(filhoExecuta, SIGINT);
}

//Funcao por matar todos os filhos a executar, uma vez recebido o que o pai recebeu o alarme de tempo maximo de execuçao atingido 
void maxExecucaoAtingido(int x){

	for (int k = 0; k <=MAX; k++)
	{
		if(filhospipes[k]!=0){
			kill(filhospipes[k],SIGINT);
		}
	}

	_exit(2);
}

//Funcao responsavel por receber o sinal de que um filho acabou a execuçao e libertar a sua possicao do array
void filhoAcabouExecucao(int x){
	pid_t pid = wait(NULL);

	for (int i = 0; i < MAXPROGEXEC; ++i)
	{
		if(filhoComandaExecucao[i]==pid){
			estado[i] = LIVRE;
			vagasExecutar++;
			break;
		}
	}
}

//Funcao responsavel por matar o filho que executa uma parte do comando
void matafilhos(int x){
	
	if(filhospipesAux!=0){
		kill(filhospipesAux, SIGINT);
	}

	_exit(1);
}

//Funcao responsavel por executar um comando pedido pelo cliente
void executa(int numFila){
	
	pid_t pid = fork(); 
	if(pid == 0){

		int fifo = open(fifoResposta,O_WRONLY);
		signal(SIGINT,terminaForcado);
		signal(SIGCHLD,SIG_DFL);

		int r = 1;
		int i = 0;
		int status, s;
		char* comandoTemp = strdup(comando[numFila]);

		int n = strlen(comandoTemp);
		
		while(i<n){
			if(comandoTemp[i] == ' ') r++;
				i++;
		}
		
		char* split[r+1];
		int j = 0;
		
		char* temp = strtok(comandoTemp," ");
		
		split[j++] = strdup(temp);
		
		while((temp = strtok(NULL," "))!=NULL){
			split[j++] = strdup(temp);
		}
		split[j++] = NULL;

		int p[2];
		int p2[2];

		pid_t tempPID = fork();

		if(tempPID==0){

			signal(SIGINT,maxExecucaoAtingido);
			signal(SIGCHLD,SIG_DFL);
			
			int j2 = 0;
			int i2 = 0;
			int posPipe = 0;
			while(1==1)
			{
				j2 = i2;
				for( ; i2<(j-1); i2++){
					if(strcmp(split[i2],"|")==0){
						split[i2] = NULL;
						i2++;
						break;
					}
				}
			
				if(i2==(j-1)){
					break;
				}
			
				pipe(p);
				pipe(p2);

				pid_t pid2 = fork();

				if(pid2 == 0){

					signal(SIGINT,matafilhos);

					pid_t pid4 = fork();

					if(pid4==0){
						dup2(p[1],1);
						close(p[1]);
						close(p[0]);
						execvp(split[j2],split+j2);
						_exit(1);
					}
					filhospipesAux = pid4;

					int n;
					alarm(tempoInactividade);
					signal(SIGALRM,matafilhos);
					
					char buff[MAX];
					while((n=read(p[0],buff,MAX))>0){
						signal(SIGALRM,SIG_IGN);
						write(p2[1],buff,n);
						break;
					}
					close(p2[1]);

					wait(NULL);

					_exit(0);
				}

				filhospipes[posPipe] = pid2;
				posPipe++;

				dup2(p2[0],0);
				close(p2[1]);
				close(p2[0]);
			}
			
			pid_t pid3 = fork();
			
			if(pid3==0){
				dup2(fifo,1);
			
				execvp(split[j2],split+j2);
			
				char historico[MAX];
				int d1 = open("historico.txt",O_WRONLY|O_CREAT|O_APPEND, 0600);
				strcpy(historico,"Erro ao executar o programa:");
				strcat(historico, comando[numFila]);
				strcat(historico, "\n");
				write(d1, historico, strlen(historico));
				close(d1);
							
				write(fifo,"Erro ao executar o programa\n",29);
				close(fifo);

				_exit(3);
			}

			filhospipes[posPipe] = pid3;
			posPipe++;

			for (int i = 0; i < posPipe; ++i)
			{
				pid_t pidFilhoAcabou = wait(&status);
				if(WIFEXITED(status)){
					s=WEXITSTATUS(status);
				}

				if(pidFilhoAcabou==pid3 && s==3){
					for (int k = 0; k <=MAX; k++)
					{
						if(filhospipes[k]!=0){
							kill(filhospipes[k],SIGINT);
						}
					}
						
					kill(getppid(),SIGUSR1);
					_exit(3);
				}
				else{
					if(s==1){
						kill(pid3,SIGINT);
						filhospipes[posPipe-1] = 0;
						for (int k = 0; k <= MAX; k++)
						{
							if(filhospipes[k]!= 0){
								kill(filhospipes[k],SIGINT);
							}
						}
						_exit(1);
					}
				}
				
			}

			_exit(0);
		}

		filhoExecuta = tempPID;

		alarm(tempoExecucao);
		signal(SIGALRM,tempoExecucaoAtingido);

		wait(&status);
		if(WIFEXITED(status)){
			s=WEXITSTATUS(status);
		}

		if(terminacaoForcada == 1){
			char historico[MAX];
			int d1 = open("historico.txt",O_WRONLY|O_CREAT|O_APPEND, 0600);
			strcpy(historico,"Paragem Forçada:");
			strcat(historico, comando[numFila]);
			strcat(historico, "\n");
			write(d1, historico, strlen(historico));

			write(fifo,"Paragem Forçada\n",17);
			close(fifo);
		}
		else{
			
			if(s==0){
				char historico[MAX];
				int d1 = open("historico.txt",O_WRONLY|O_CREAT|O_APPEND, 0600);
				strcpy(historico,"concluida:");
				strcat(historico, comando[numFila]);
				strcat(historico, "\n");
				write(d1, historico, strlen(historico));
				close(d1);
			}else{	
				if(s==1){
					char historico[MAX];
					int d1 = open("historico.txt",O_WRONLY|O_CREAT|O_APPEND, 0600);
					strcpy(historico,"max inactividade:");
					strcat(historico, comando[numFila]);
					strcat(historico, "\n");
					write(d1, historico, strlen(historico));
					close(d1);

					write(fifo,"max inactividade atingida\n",27);
				}
				else{
					char historico[MAX];
					int d1 = open("historico.txt",O_WRONLY|O_CREAT|O_APPEND, 0600);
					strcpy(historico,"max execução:");
					strcat(historico, comando[numFila]);
					strcat(historico, "\n");
					write(d1, historico, strlen(historico));
					close(d1);

					write(fifo,"max execução atingida\n",24);	
				}
			}
		}

		close(fifo);
		_exit(0);
	}

	filhoComandaExecucao[numFila] = pid;	
}

//Funcao main que vai perceber que instrucao realizar e se for para executar, cria um filho e mete-o a executar.
int main(int argc, char const *argv[]){

	int opc = mkfifo("fifo1",0600);
	struct estruturaDadosFifo e;
	signal(SIGCHLD,filhoAcabouExecucao);

	write(1,"####################################SERVIDOR####################################\n",82);
	write(1,"Crtl+C para terminar o servidor\n",33);

	while(1==1){

		int fifo = open("fifo1",O_RDONLY);
		int n = read(fifo,&e,sizeof(struct estruturaDadosFifo));
		close(fifo);
	
		if(strcmp(e.opcao,"tempo-inactividade")==0){
			int temp = atoi(e.comando);
			if(temp>0){
				tempoInactividade = temp;
			}
		}

		if(strcmp(e.opcao,"tempo-execucao")==0){
			int temp = atoi(e.comando);
			if(temp>0){
				tempoExecucao = temp;
			}
		}

		if(strcmp(e.opcao,"executar")==0){
			
			if(vagasExecutar>0){
				int i;
				for(i=0;i<MAXPROGEXEC;i++){
					
					if(estado[i] == LIVRE){
						
						estado[i] = OCUPADO;
						vagasExecutar--;

						strcpy(comando[i],e.comando);
						strcpy(fifoResposta,e.fifoResposta); 

						executa(i);
						break;
					}
				}
			}
			else{
				int fifo = open(e.fifoResposta,O_WRONLY);
				write(fifo,"SERVIDOR LOTADO\n",17);
				close(fifo);
			}
		}

		if(strcmp(e.opcao,"listar")==0){
			if(vagasExecutar != MAXPROGEXEC){
				
				int fifo = open(e.fifoResposta,O_WRONLY);
				
				for (int i = 0; i < MAXPROGEXEC; ++i)
				{
					if(estado[i]==OCUPADO){
						
						write(fifo,comando[i],strlen(comando[i]));
						write(fifo,"\n",1);
					}
				}

				close(fifo);
			}
			else{
				int fifo = open(e.fifoResposta,O_WRONLY);
				write(fifo,"Nenhum programa a executar\n",27);
				close(fifo);
			}
		}

		if(strcmp(e.opcao,"terminar")==0){
			int progMatar = atoi(e.comando);
			int r = 0;
			int matou = 1;
			for (int i = 0; i < MAXPROGEXEC && matou==1; ++i)
			{
				if(estado[i] == OCUPADO){
					if(r == progMatar){
						kill(filhoComandaExecucao[i],SIGINT);
						matou = 0;
					}
					r++;
				}
			}
		}

		if(strcmp(e.opcao,"historico")==0){
			
			int fifo = open(e.fifoResposta,O_WRONLY);
			
			char bufferLeitura[MAX];
			int d1 = open("historico.txt",O_RDONLY, 0600);
			int n;
			int leu = 0;
			
			while((n = read(d1, bufferLeitura, MAX)) > 0)
			{
				write(fifo, bufferLeitura, n);
				leu = 1;
			}

			if(leu==0){
				write(fifo, "Sem Historico\n", 15);
			}
			
			close(fifo);
			close(d1);
		}
	}

	return 1;
}