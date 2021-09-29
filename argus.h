#define MAX 4096

//Estrutura de dados utilizada para escrita no fifo responsavel pela comunicaçao entre pai e filho
typedef struct estruturaDadosFifo
{
	char opcao[MAX];
	char comando[MAX];
	char fifoResposta[MAX];
}EscritaFifo;