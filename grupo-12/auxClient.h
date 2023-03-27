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

int readc(int fd, char *c);
ssize_t readln(int fd, char *line, size_t size);

void sendToServerPF(int argc, char *argv[]);

void sendToServerStatus(int argc, char *argv[]);

void receiveFromServer();

#endif