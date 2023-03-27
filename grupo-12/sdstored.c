#include "auxServer.h"

/*----------------------------------- EXECUÇÃO DE OPÇÕES -----------------------------------*/

//funçao que envia ao cliente informação sobre as tasks a correr naquele momento
int executeStatus(char pid[]){
	char path[11];
	for (int i = 0;i<11;i++){
		path[i] = '\0';
	}

    sprintf(path,"/tmp/%s",pid);
    mkfifo(path, 0666);

    int fifo_client = open(path,O_WRONLY);

    int f_log1, f_log2;

	if ((f_log2 = open("log.bin", O_RDONLY))==-1){
		perror("[execute status] Erro ao abrir o ficheiro log");
		return -1;
	}

	int j;
	char buffer[4096] = "";

	Processamento aux;

	while((read(f_log2, &aux, sizeof(aux))) > 0){

		if(aux.completed==1){
			
			j = snprintf(buffer, sizeof(buffer), "task #%d: proc-file %s\n", aux.task, aux.comandos);
	
			write(fifo_client,buffer,j);
		}	       
    }

    close(f_log2);

    if ((f_log1 = open("log_limites.bin", O_RDONLY))==-1){
		perror("[execute status] Erro ao abrir o ficheiro log");
		return -1;

	}

	Limites aux1;

	if(read(f_log1, &aux1, sizeof(aux1))<0){
		perror("exec status ler limites");
		return -1;
	}

	j = snprintf(buffer, sizeof(buffer), "transf nop: %d\\%d (running\\max)\n", aux1.nop_atual,nop_max);
	write(fifo_client,buffer,j);

	j = snprintf(buffer, sizeof(buffer), "transf bcompress: %d\\%d (running\\max)\n", aux1.bcompress_atual,bcompress_max);
	write(fifo_client,buffer,j);

	j = snprintf(buffer, sizeof(buffer), "transf bdecompress: %d\\%d (running\\max)\n", aux1.bdecompress_atual,bdecompress_max);
	write(fifo_client,buffer,j);

	j = snprintf(buffer, sizeof(buffer), "transf gcompress: %d\\%d (running\\max)\n", aux1.gcompress_atual,gcompress_max);
	write(fifo_client,buffer,j);

	j = snprintf(buffer, sizeof(buffer), "transf gdecompress: %d\\%d (running\\max)\n", aux1.gdecompress_atual,gdecompress_max);
	write(fifo_client,buffer,j);

	j = snprintf(buffer, sizeof(buffer), "transf encrypt: %d\\%d (running\\max)\n", aux1.encrypt_atual,encrypt_max);
	write(fifo_client,buffer,j);

	j = snprintf(buffer, sizeof(buffer), "transf decrypt: %d\\%d (running\\max)\n", aux1.decrypt_atual,decrypt_max);
	write(fifo_client,buffer,j);

	close(f_log1);

    write(fifo_client,"stop\n",5);
    close(fifo_client);

    return 0;
}


//funçao que executa os comandos enviados pelo cliente
void executeServer(char pid[], char line[], int numeroArg) {

	answerClient(pid, "pending\n");

	char delim[] = " ";

	char* cmds[50];

	int j = 0, status;

	char* token = strtok(line, delim);
	cmds[j] = token;
	j++;

	while (token != NULL) {
		//printf(" %s\n",token);
		token = strtok(NULL, delim);
		cmds[j] = token;
		j++;
	}

	cmds[j] = NULL;

	if (verificaLimites(cmds, numeroArg, pid) == 0) {

		int forigin, fdestination, status, i;

		int fstdout = dup(1);

		while (verificaDisp(cmds, numeroArg) != 1) {
			sleep(2);
		}

		updateLog(pid, 1);

		updateLogConfig(cmds, numeroArg, 0);

		answerClient(pid, "processing\n");

		if (numeroArg == 1) {

			switch (fork()) {
				case -1:
					perror("criação de fork");
					_exit(-1);

				case 0:

					forigin = open(cmds[0], O_RDONLY);
					if (forigin < 0) {
						perror("[execute proc-file] open file f_origin");
						_exit(-1);
					}

					fdestination = open(cmds[1], O_CREAT | O_WRONLY | O_TRUNC, 0666);
					if (fdestination < 0) {
						perror("[execute proc-file] open file f_destination");
						_exit(-1);
					}

					dup2(forigin, 0);
					dup2(fdestination, 1);
					close(forigin);
					close(fdestination);

					strcat(path, cmds[2]);
					//write(fstdout,path,sizeof(path));
					execl(path, cmds[2], NULL);
					perror("exec numeroarg == 1");
					_exit(0);

				default:
					wait(&status);
			}
			
			updateLog(pid, 2);
			updateLogConfig(cmds, numeroArg, 1);
			answerClient(pid, "concluded\n");
			answerClient(pid, "stop\n");
		}
		else {

			//sleep(5);
			int NC = numeroArg;

			int p[NC - 1][2];

			i = 0;

			while (i < NC) {

				if (i == 0) {

					pipe(p[0]);

					switch (fork()) {
						case -1:
							perror("criação de fork");
							exit(-1);

						case 0:
							forigin = open(cmds[0], O_RDONLY);
							if (forigin < 0) {
								perror("[execute proc-file] open file f_origin");
								_exit(-1);
							}

							close(p[0][0]);

							dup2(p[0][1], 1);
							dup2(forigin, 0);
							close(forigin);
							close(p[0][1]);

							strcat(path, cmds[2 + i]);
							//write(fstdout,path,sizeof(path));
							execl(path, cmds[2 + i], NULL);
							perror("exec numeroarg != 1 and i==0");
							close(1);
							_exit(0);

						default:
							close(p[0][1]);
					}
				}

				else if (i == NC - 1) {

					switch (fork()) {
						case -1:
							perror("criação de fork");
							exit(-1);

						case 0:
							fdestination = open(cmds[1], O_CREAT | O_WRONLY | O_TRUNC, 0666);
							if (fdestination < 0) {
								perror("[execute proc-file] open file f_destination");
								_exit(-1);
							}

							dup2(p[i - 1][0], 0);
							dup2(fdestination, 1);

							close(p[i - 1][0]);
							close(fdestination);

							strcat(path, cmds[2 + i]);
							execl(path, cmds[2 + i], NULL);

							perror("exec numeroarg != 1 and i==NC-1");
							_exit(0);
						default:
							close(p[i - 1][0]);
					}
				}

				else {
					pipe(p[i]);

					switch (fork()) {
						case -1:
							perror("criação de fork");
							_exit(-1);

						case 0:
							close(p[i][0]);

							dup2(p[i - 1][0], 0);
							dup2(p[i][1], 1);
							close(p[i - 1][0]);
							close(p[i][1]);

							strcat(path, cmds[2 + i]);
							execl(path, cmds[2 + i], NULL);
							perror("exec numeroarg != 1 and i!=0 and i!=NC-1");
							_exit(0);

						default:
							close(p[i - 1][0]);
							close(p[i][1]);

					}
				}
				i++;
			}

			for (int k = 0; k < NC; k++) {
				//int status;
				wait(&status);
			}

			updateLog(pid, 2);
			updateLogConfig(cmds, numeroArg, 1);
			answerClient(pid, "concluded\n");
			answerClient(pid, "stop\n");
		}
	}
	else {
		updateLog(pid, 2);
	}
}

/*----------------------------------- PARSING -----------------------------------*/

// função que faz parse aos argumentos enviados pelo cliente e executa-os
void parseServer(char line[],int n){

	int length=0;
	char * n_argc;
	char * pid;

	pid = strtok(line, " ");
	length+=strlen(pid);
	//printf("%s\n",pid);

	n_argc = strtok(NULL," ");
	length+=strlen(n_argc)+2;
	
	int m = atoi(n_argc);
	//printf("%d\n",m);
	/*
	
	printf("%d\n",length);
	printf("%s\n",line+length);
	*/
	if(m==1){ //opçao status

		int pid_filho = fork();
		
		switch(pid_filho){
			case -1:
				perror("erro criação de fork (status)");
				_exit(-1);

			case 0:
				executeStatus(pid);
				_exit(0);

			default:
				printf("Filho %d a executar o comando status\n", pid_filho);
		}
	}

	else{ //opçao proc-file
	
	   	int pid_filho = fork();
	
		switch(pid_filho){
	   		case -1:
	   			perror("criação de fork");
	   			_exit(-1);
	
			case 0:
	   			writeToLog(pid,line+length,task);
				executeServer(pid,line+length,m-2);
				_exit(0);
	
			default:
				printf("Filho %d a executar o comando proc-file %s\n", pid_filho, line+length);
				task ++;
   		}
    }
}


//funçao que faz parse aos argumentos recebidos pelo servidor
int parseArgs(int argc, char* argv[]){

	int log;
	if (argc == 1 || argc > 3){
		write(1,"[Usage] ./sdtored <path_config_file> <path_tranformations>\n",59);
		return -1;
	}

	if(read_Config_File(argv[1])<0){
		return -1;
	}
	
	if((log = open("log.bin",O_WRONLY | O_CREAT | O_TRUNC, 0660))==-1){
		perror("Impossivel criar log file");
		return -1;
	}

	strcpy(path,argv[2]);

	int len = strlen(argv[2]);

	if (argv[2][len-1] != '/'){
		strcat(path,"/");
	}

	return 0;
}


/*----------------------------------- MAIN -----------------------------------*/

int main(int argc, char* argv[]) {

	if(parseArgs(argc,argv)<0){
		return -1;
	}
	
	int n, fd, fd2, task;
	char * path_fifo = "/tmp/fifo";
    mkfifo(path_fifo, 0666);

	char line[4096];

	fd = open(path_fifo, O_RDONLY);
	if (fd == -1) {
		perror("opening fifo");
	};

	fd2 = open(path_fifo, O_WRONLY);
	if (fd2 == -1) {
		perror("opening fifo");
	};
	
	task = 0;

	while((n = readln(fd, line, 4096)) > 0) {
		//write(1,line,n);
		//write(1,"\n",1);
		parseServer(line,n);
	}

	close(fd);
	
	return 0;
}