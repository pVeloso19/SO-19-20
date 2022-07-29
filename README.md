#  Serviço de monitorização, execução e comunicação entre processos
## SO-19-20

> Este trabalho realizado no âmbito da disciplina de Sistemas Operativos (2019/2020) na Universidade do Minho.
>
> Neste trabalho elaborou-se um projeto onde se implementou um serviço de monitorização, execução e de comunicação entre processos.
>
> O serviço deveria ser capaz de receber de um utilizador tarefas, cada uma delas sendo uma sequência de comandos encadeados por pipes anónimos, ou comandos simples sem qualquer tipo de encadeamento. Além de iniciar a execução das tarefas, o servidor deveria ser capaz de identificar as tarefas em execução, bem como a conclusão da sua execução. Deveria também ser capaz de terminar tarefas em execução, caso não se verifique qualquer comunicação através de pipes anónimos ao fim do tempo máximo de inatividade, que por sua vez podia ser alterado pelo utilizador. Por fim o servidor teria um tempo máximo especificado, onde qualquer execução que ultrapasse esse tempo seria abortada. 
>
> Também foi pedido que a interface por trás do utilizador, implementa-se duas possibilidades diferentes, ou seja, fosse capaz de receber comandos através da linha de comados ou de apresentar uma interface textual interpretada (tipo Shell).
>
> Para realizar este trabalho recorreu-se à linguagem C, sem qualquer utilização das funções da biblioteca de C para operações sobre ficheiros.
>
> Para executar o serviço foi realizada uma [Makefile](https://github.com/pVeloso19/SO-19-20/blob/main/Makefile) que realiza a compilação dos arquivos .c, bem como apaga todos os arquivos gerados por execuções anteriores.
>
> Existe o ficheiro [argus.c](https://github.com/pVeloso19/SO-19-20/blob/main/argus.c) que reperesenta o programa elaborado para o cliente. Para perceber o seu funcionamento existe uma flag (-h no modo de argumentos do programa) que apresenta os comandos disponiveis.
>
> Por fim o ficheiro [argusd.c](https://github.com/pVeloso19/SO-19-20/blob/main/argusd.c) representa o programa elaborado para o serviço.

###### Nota Final: 16/20
