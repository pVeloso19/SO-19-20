#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>   /*chamadas ao sistema: defs e decls essenciais*/
#include <fcntl.h>    /*O_RDONLY, O_WRONLY, O_CREAT, O_* */

#include "argus.h"

//Funcao responsavel por ignorar um control+C no menu do argus
void IGNORE(int x){
	write(1, "\nargus$ ", 8);
}

//Funcao responsavel por terminar o argus, quando recebido um SIGQUIT
void SAICLIENTE(int x){
	write(1, "\n", 1);
	_exit(0);
}

//Funcao responsavel por gerar um nome aleatorio para um fifo
void getNomeFifo(char* nome, int tam){
	srand(time(NULL));
	for (int i = 0; i < (tam-1); ++i)
	{
		nome[i]= (char)(rand()%26) + 'A';
	}
	nome[tam] = '\0';
}

//Funcao responsavel por ir recuperar o output, e apresentar no ecra
void recuperaOutPut(int pos){
	int d  = open("log.idx",O_RDONLY); 
	int d1 = open("log.txt",O_RDONLY);
	char buff[MAX];

	int numLinhas = 0;
	int encontrou = 0;
	char* temp;
	int n;

	while((n=read(d,buff,MAX))>0){
		temp = strtok(buff,"\n");
		numLinhas++;
		if(pos==0){
			encontrou = 1;
			break;
		}
		while(((temp = strtok(NULL,"\n"))!=NULL) && (numLinhas<pos)){
			numLinhas++;
		}

		if(numLinhas==pos && temp!=NULL){
			encontrou = 1;
			break;
		}
	}

	if(encontrou==1){
		int posFich = atoi(strtok(temp," "));
		int tamLer = atoi(strtok(NULL," "));

		int n2 = lseek(d1,posFich,SEEK_CUR);
		
		char buffTemp[tamLer];
		n = read(d1,buffTemp,tamLer);
		write(1,buffTemp,n);
	}
	else{
		printf("Nao existe Output\n");
	}

}

//Funcao responsavel por pedir ao servidor, para executar um comado
void executar(char* comando, int tam){

	char nomeFIFO[10];
	getNomeFifo(nomeFIFO,10);

	int opc = mkfifo(nomeFIFO,0600);
	
	struct estruturaDadosFifo e;
	strcpy(e.opcao, "executar");
	strcpy(e.fifoResposta, nomeFIFO);
	strcpy(e.comando, comando);

	int fifo = open("fifo1", O_WRONLY);

	write(fifo, &e, sizeof(struct estruturaDadosFifo));

	close(fifo);

	int d  = open("log.idx",O_WRONLY|O_CREAT|O_APPEND, 0600); 
	int d1 = open("log.txt",O_WRONLY|O_CREAT, 0600);
	int n2 = lseek(d1,0,SEEK_END);

	fifo = open(nomeFIFO, O_RDONLY);

	int n;
	int nT = 0;
	char buff[MAX];
	while((n=read(fifo,buff,MAX))>0){
		nT += n;
		write(1,buff,n);
		write(d1,buff,n);
	}

	char resp[MAX];
	sprintf(resp,"%d %d\n",n2,nT);
	write(d,resp,strlen(resp));
	close(d);

	close(fifo);
	close(d1);
}

//Funcao responsavel por mostar ajuda para a utilização do programa
void printaAjuda(){
	write(1, "tempo-inatividade(-i) segundos\n", 32);
	write(1, "tempo-execucao(-m) segundos\n", 29);
	write(1, "executar(-e) p1 | p2 ...\n", 26);
	write(1, "listar(-l)\n", 12);
	write(1, "terminar(-t) numero\n", 21);
	write(1, "historico(-r)\n", 15);
	write(1, "output(-o) numero\n", 19);
	write(1, "Crtrl+| (SIGQUIT) para sair\n", 29);
}

//Funcao responsavel por pedir ao servidor o historico de execuçoes
void printaHistorico(){

	char nomeFIFO[10];
	getNomeFifo(nomeFIFO,10);

	int opc = mkfifo(nomeFIFO,0600);
	
	struct estruturaDadosFifo e;
	strcpy(e.opcao, "historico");
	strcpy(e.fifoResposta, nomeFIFO);
	strcpy(e.comando, "Non");

	int fifo = open("fifo1", O_WRONLY);

	write(fifo, &e, sizeof(struct estruturaDadosFifo));

	close(fifo);

	fifo = open(nomeFIFO, O_RDONLY);

	int n;
	char buff[MAX];
	while((n=read(fifo,buff,MAX))>0){
		write(1,buff,n);
	}

	close(fifo);
}

//Funcao responsavel por defenir no servidor o tempo maximo de inatividade
void setTempoInatividade(char* tempo){
	struct estruturaDadosFifo e;
	strcpy(e.opcao, "tempo-inactividade");
	strcpy(e.fifoResposta, "Non");
	strcpy(e.comando, tempo);

	int fifo = open("fifo1", O_WRONLY);

	write(fifo, &e, sizeof(struct estruturaDadosFifo));

	close(fifo);
}

//Funcao responsavel pr defenir no servidor o tempo maximo de execução
void setTempoExecucao(char* tempo){
	struct estruturaDadosFifo e;
	strcpy(e.opcao, "tempo-execucao");
	strcpy(e.fifoResposta, "Non");
	strcpy(e.comando, tempo);

	int fifo = open("fifo1", O_WRONLY);

	write(fifo, &e, sizeof(struct estruturaDadosFifo));

	close(fifo);
}

//Funcao responsavel por pedir ao servidor a lista de programas que estao a executar 
void listar(){
	char nomeFIFO[10];
	getNomeFifo(nomeFIFO,10);

	int opc = mkfifo(nomeFIFO,0600);
	
	struct estruturaDadosFifo e;
	strcpy(e.opcao, "listar");
	strcpy(e.fifoResposta, nomeFIFO);
	strcpy(e.comando, "Non");

	int fifo = open("fifo1", O_WRONLY);

	write(fifo, &e, sizeof(struct estruturaDadosFifo));

	close(fifo);

	fifo = open(nomeFIFO, O_RDONLY);

	int n;
	char buff[MAX];
	while((n=read(fifo,buff,MAX))>0){
		write(1,buff,n);
	}

	close(fifo);
}

//Funcao responsavel por terminar uma instrucao a executar no servidor
void terminar(char* comando){
	struct estruturaDadosFifo e;
	strcpy(e.opcao, "terminar");
	strcpy(e.fifoResposta, "Non");
	strcpy(e.comando, comando);

	int fifo = open("fifo1", O_WRONLY);

	write(fifo, &e, sizeof(struct estruturaDadosFifo));

	close(fifo);
}

//Funcao main que vai receber os comandos e decidir o que fazer
int main(int argc, char const *argv[]){
	char* opcaoExecutar;
	char* comando;

    if(argc <= 1){
    	while(1){
    		char buffer[MAX];

    		signal(SIGINT,IGNORE);
    		signal(SIGQUIT,SAICLIENTE);
    		
    		write(1, "\nargus$ ", 9);
    		read(0, buffer, MAX);
    		
    		char opcaoExecutar[MAX];
    		strcpy(opcaoExecutar,strtok(buffer," "));
    		int i;
    		for ( i = 0; i<strlen(opcaoExecutar); i++){
    			if(opcaoExecutar[i]=='\n')
    				opcaoExecutar[i] = '\0';
    		}

    		comando = buffer + strlen(opcaoExecutar)+1;
    		
    		for ( i = 0; i<strlen(comando); i++){
    			if(comando[i]=='\n')
    				comando[i] = '\0';
    		}

    		signal(SIGINT,SIG_IGN);
    		signal(SIGQUIT,SIG_IGN);
    		
    		if(strcmp(opcaoExecutar,"tempo-inactividade")==0){
    			setTempoInatividade(comando);
    		}
    		
    		if(strcmp(opcaoExecutar,"tempo-execucao")==0){
    			setTempoExecucao(comando);
    		}
    		
    		if(strcmp(opcaoExecutar,"executar")==0){
    			executar(comando,strlen(comando));
    		}
    		
    		if(strcmp(opcaoExecutar,"listar")==0){
    			listar();
    		}
    		
    		if(strcmp(opcaoExecutar,"terminar")==0){
    			terminar(comando);
    		}
    		
    		if(strcmp(opcaoExecutar,"historico")==0){
    			printaHistorico();
    		}
    		
    		if(strcmp(opcaoExecutar,"ajuda")==0){
    			printaAjuda();
    		}
    		
    		if(strcmp(opcaoExecutar,"output")==0){
    			int pos = atoi(comando);
    			recuperaOutPut(pos);		
    		}
    	}	
	}
	else{

		opcaoExecutar = argv[1];
		
		signal(SIGINT,SIG_IGN);
    	signal(SIGQUIT,SIG_IGN);

		if(strcmp(opcaoExecutar,"-i")==0){
			setTempoInatividade(argv[2]);
		}

		if(strcmp(opcaoExecutar,"-m")==0){
			setTempoExecucao(argv[2]);
		}

		if(strcmp(opcaoExecutar,"-e")==0){
			comando = argv[2];
			executar(comando,strlen(comando)+1);
		}

		if(strcmp(opcaoExecutar,"-l")==0){
			listar();
		}

		if(strcmp(opcaoExecutar,"-t")==0){
			terminar(argv[2]);
		}

		if(strcmp(opcaoExecutar,"-r")==0){
			printaHistorico();
		}

		if(strcmp(opcaoExecutar,"-h")==0){
			printaAjuda();
		}

		if(strcmp(opcaoExecutar,"-o")==0){
			int pos = atoi(argv[2]);
			recuperaOutPut(pos);		
		}
	}

	return 0;
}
