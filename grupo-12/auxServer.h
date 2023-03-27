#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

#ifndef MY_HEADER_H
#define MY_HEADER_H

#define BUFFER_SIZE 4096

int task, nop_max, bcompress_max, bdecompress_max, gcompress_max, gdecompress_max, encrypt_max, decrypt_max; 
char path[100];

typedef struct{
	char pid[10];
	char comandos[4096];
	int completed; // 0 -> pending, 1-> processing, 2-> completed
	int task;
}Processamento;

typedef struct{
	int nop_atual;
	int bcompress_atual;
	int bdecompress_atual;
	int gcompress_atual;
	int gdecompress_atual;
	int encrypt_atual;
	int decrypt_atual;
}Limites;

void answerClient(char pid[], char mens[]);

int readc(int fd, char *c);
ssize_t readln(int fd, char *line, size_t size);

int read_Config_File(char * path_config);

int updateLog(char pid[], int flag);

int updateLogConfig(char * cmds[], int numeroArg, int flag);

int writeToLog(char pid[], char cmds[], int task);

int verificaLimites(char * cmds[], int numeroArg, char pid[]);

int verificaDisp(char * cmds[], int numeroArg);

#endif