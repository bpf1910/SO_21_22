#include "auxServer.h"

char read_buffer[BUFFER_SIZE];
int read_buffer_pos = 0;
int read_buffer_end = 0;

/*--------------------------- ANSWER CLIENT ---------------------------*/

//função que envia ao cliente informação sobre o estado de um processamento de um ficheiro
void answerClient(char pid[], char mens[]){
    char path[11];
    
    for (int i = 0;i<11;i++){
        path[i] = '\0';
    }

    sprintf(path,"/tmp/%s",pid);
    mkfifo(path, 0666);

    int fifo_client = open(path,O_WRONLY);

    write(fifo_client,mens,strlen(mens));
    close(fifo_client);
}


/*--------------------------- READ LINE ---------------------------*/

int readc(int fd, char *c)
{
    if (read_buffer_pos == read_buffer_end)
    {
        read_buffer_end = read(fd, read_buffer, BUFFER_SIZE);
        switch (read_buffer_end)
        {
        case -1:
            perror("read_line");
            break;
        case 0:
            return 0;
            break;
        default:
            read_buffer_pos = 0;
        }
    }
    *c = read_buffer[read_buffer_pos++];
    return 1;
}

ssize_t readln(int fd, char *line, size_t size)
{
    int res = 0;
    int i = 0;
    while (i < size && (res = readc(fd, line + i) > 0))
    {
        i++;
        if ((line)[i - 1] == '\n') {
            line[i-1] = '\0';
            return i;
        }
    }
    return i;
}


/*--------------------------- READ CONFIG FILE ---------------------------*/

//funçao que lê o ficheiro de configurações
int read_Config_File(char * path_config){
    int config_file, log;

    if((config_file = open(path_config,O_RDONLY))==-1){
        perror("Impossivel abrir config file");
        return -1;
    }
    
    char buffer[200];
    int i = 0, bytes;

    if((bytes = read(config_file,buffer,200))<0){
        perror("leitura do config file");
        return -1;
    }

    close(config_file);

    char * limites[7];
    char delim[] = "\n";

    int j = 0; 
    
    char * token = strtok(buffer, delim);
    limites[j] = token;
    j++;
    
    while( token != NULL ) {
        token = strtok(NULL, delim);
        limites[j] = token;
        j++;
    }
    
    Limites aux;

    for (j = 0; j<7; j++){
        if(strstr(limites[j],"nop")!=NULL){
            int n = strlen(limites[j]);
            nop_max = limites[j][n-1] - '0';
            aux.nop_atual = 0;
        }
        else if(strstr(limites[j],"bcompress")!=NULL){
            int n = strlen(limites[j]);
            bcompress_max = limites[j][n-1] - '0';
            aux.bcompress_atual = 0;
        }
        else if(strstr(limites[j],"bdecompress")!=NULL){
            int n = strlen(limites[j]);
            bdecompress_max = limites[j][n-1] - '0';
            aux.bdecompress_atual = 0;
        }
        else if(strstr(limites[j],"gcompress")!=NULL){
            int n = strlen(limites[j]);
            gcompress_max = limites[j][n-1] - '0';
            aux.gcompress_atual = 0;
        }
        else if(strstr(limites[j],"gdecompress")!=NULL){
            int n = strlen(limites[j]);
            gdecompress_max = limites[j][n-1] - '0';
            aux.gdecompress_atual = 0;
        }
        else if(strstr(limites[j],"encrypt")!=NULL){
            int n = strlen(limites[j]);
            encrypt_max = limites[j][n-1] - '0';
            aux.encrypt_atual = 0;
        }
        else if(strstr(limites[j],"decrypt")!=NULL){
            int n = strlen(limites[j]);
            decrypt_max = limites[j][n-1] - '0';
            aux.decrypt_atual = 0;
        }
    }

    int f_log;

    if((f_log = open("log_limites.bin",O_WRONLY | O_CREAT | O_TRUNC, 0660))==-1){
        perror("Impossivel criar log file");
        return -1;
    }

    if(write(f_log,&aux,sizeof(aux))<0){
        perror("Write para o log");
        return -1;
    }

    close(f_log);

    return 0;
}


/*--------------------------- UPDATE LOG ---------------------------*/

//funçao que atualiza certo pedido feito pelo cliente
int updateLog(char pid[], int flag){
    int f_log;
    
    if ((f_log = open("log.bin", O_RDWR))==-1){
        perror("[update log] Erro ao abrir o ficheiro log");
        return -1;
    }

    Processamento aux;
    int w;

    while((w = read(f_log,&aux,sizeof(aux)))>0){
        
        if (strcmp(aux.pid,pid)==0){
            
            aux.completed = flag;
            
            off_t o = lseek(f_log,-sizeof(aux),SEEK_CUR);

            write(f_log,&aux,sizeof(aux));

            break;
        }
    }

    if(w<0){
        perror("Write");
        close(f_log);
        return -1;
    }

    close(f_log);

    return 0;
}


/*--------------------------- UPDATE LOG CONFIG ---------------------------*/

int updateLogConfig(char * cmds[], int numeroArg, int flag){
    int log;

    if((log = open("log_limites.bin", O_RDONLY))==-1){
        perror("[update config file] Erro ao abir o ficheiro log");
        return -1;
    }

    Limites aux;

    if(read(log,&aux,sizeof(aux))==-1){
        perror("[update config file] Erro a ler do ficheiro");
        return -1;
    }

    close(log);

    if(flag==0){

        for (int i = 0; i < numeroArg; i++){
            if(strcmp(cmds[i+2],"nop")==0){
                aux.nop_atual += 1;
            }

            else if(strcmp(cmds[i+2],"bcompress")==0){
                aux.bcompress_atual = aux.bcompress_atual+1;
            }

            else if(strcmp(cmds[i+2],"bdecompress")==0){
                aux.bdecompress_atual += 1;
            }

            else if(strcmp(cmds[i+2],"gcompress")==0){
                aux.gcompress_atual += 1;
            }

            else if(strcmp(cmds[i+2],"gdecompress")==0){
                aux.gdecompress_atual += 1;
            }

            else if(strcmp(cmds[i+2],"encrypt")==0){
                aux.encrypt_atual += 1;
            }

            else if(strcmp(cmds[i+2],"decrypt")==0){
                aux.decrypt_atual += 1;
            }
        }
    }

    else{

        for (int i = 0; i < numeroArg; i++){
            if(strcmp(cmds[i+2],"nop")==0){
                aux.nop_atual -= 1;
            }

            else if(strcmp(cmds[i+2],"bcompress")==0){
                aux.bcompress_atual -= 1;
            }

            else if(strcmp(cmds[i+2],"bdecompress")==0){
                aux.bdecompress_atual -= 1;
            }

            else if(strcmp(cmds[i+2],"gcompress")==0){
                aux.gcompress_atual -= 1;
            }

            else if(strcmp(cmds[i+2],"gdecompress")==0){
                aux.gdecompress_atual -= 1;
            }

            else if(strcmp(cmds[i+2],"encrypt")==0){
                aux.encrypt_atual -= 1;
            }

            else if(strcmp(cmds[i+2],"decrypt")==0){
                aux.decrypt_atual -= 1;
            }
        }
    }

    if((log = open("log_limites.bin", O_WRONLY ))==-1){
        perror("[update config file] Erro ao abir o ficheiro log");
        return -1;
    }

    if(write(log,&aux,sizeof(aux))<0){
        perror("[log update] Escrita do ficheiro!");
        return -1;
    }

    return 0;
}


/*--------------------------- WRITE LOG ---------------------------*/

//funçao que escreve num ficheiro log os pedidos feitos ao servidor
int writeToLog(char pid[], char cmds[], int task){
    int f_log;
    
    if ((f_log = open("log.bin", O_WRONLY | O_APPEND, 0644))==-1){
        //printf("Msg: %s, Nr: %d\n", sterror(errno),errno);
        perror("Erro ao abrir o ficheiro log");
        return -1;

    }

    Processamento aux;

    strcpy(aux.comandos,cmds);
    strcpy(aux.pid,pid);
    aux.completed = 0;
    aux.task = task;
    
    if(write(f_log,&aux,sizeof(aux))<0){
        perror("Write para o log");
        close(f_log);
        return -1;
    }

    close(f_log);

    return 0;
}


/*--------------------------- VERIFICA LIMITES ---------------------------*/

int verificaLimites(char * cmds[], int numeroArg, char pid[]){
    int nop = 0, bcom = 0, bdecom = 0, gcom = 0, gdecom = 0, encrypt = 0, decrypt = 0;

    for (int i = 0; i<numeroArg; i++){
        if (strcmp("nop",cmds[2+i])==0){
            nop += 1;
            if(nop>nop_max){
                answerClient(pid,"O número de utilizações do comando nop excede o limite suportado pelo servidor\n");
                answerClient(pid,"stop\n");
                return -1;
            }
        }
        else if (strcmp("bcompress",cmds[2+i])==0){
            bcom += 1;
            if(bcom>bcompress_max){
                answerClient(pid,"O número de utilizações do comando bcompress excede o limite suportado pelo servidor\n");
                answerClient(pid,"stop\n");
                return -1;
            }
        }
        else if (strcmp("bdecompress",cmds[2+i])==0){
            bdecom += 1;
            if(bdecom>bdecompress_max){
                answerClient(pid,"O número de utilizações do comando bdecompress excede o limite suportado pelo servidor\n");
                answerClient(pid,"stop\n");
                return -1;
            }
        }
        else if (strcmp("gcompress",cmds[2+i])==0){
            gcom += 1;
            if(gcom>gcompress_max){
                answerClient(pid,"O número de utilizações do comando gcompress excede o limite suportado pelo servidor\n");
                answerClient(pid,"stop\n");
                return -1;
            }
        }
        else if (strcmp("gdecompress",cmds[2+i])==0){
            gdecom += 1;
            if(gdecom>gdecompress_max){
                answerClient(pid,"O número de utilizações do comando gdecompress excede o limite suportado pelo servidor\n");
                answerClient(pid,"stop\n");
                return -1;
            }
        }
        else if (strcmp("encrypt",cmds[2+i])==0){
            encrypt += 1;
            if(encrypt>encrypt_max){
                answerClient(pid,"O número de utilizações do comando encrypt excede o limite suportado pelo servidor\n");
                answerClient(pid,"stop\n");
                return -1;
            }
        }

        else{
            decrypt += 1;
            if(decrypt>decrypt_max){
                answerClient(pid,"O número de utilizações do comando decrypt excede o limite suportado pelo servidor\n");
                answerClient(pid,"stop\n");
                return -1;
            }
        }
    }
    return 0;
}

/*--------------------------- VERIFICA DISPONIBILIDADE ---------------------------*/

int verificaDisp(char * cmds[], int numeroArg){
    int f_log;
    
    if ((f_log = open("log_limites.bin", O_RDONLY))==-1){
        perror("[verifica disp] Erro ao abrir o ficheiro log_limites");
    }

    Limites aux;

    if(read(f_log,&aux,sizeof(aux))==-1){
        perror("[verificaDisp] Erro a ler do ficheiro log_limites");
    }

    close(f_log);

    int nop = aux.nop_atual, bcom = aux.bcompress_atual, bdecom = aux.bdecompress_atual;
    int gcom = aux.gcompress_atual, gdecom = aux.gdecompress_atual;
    int encrypt = aux.encrypt_atual, decrypt = aux.decrypt_atual;
    
    for (int i = 0; i<numeroArg; i++){
        if (strcmp("nop",cmds[2+i])==0){
            nop += 1;
            if(nop>nop_max){
                return 0;
            }
        }
        else if (strcmp("bcompress",cmds[2+i])==0){
            bcom += 1;
            if(bcom>bcompress_max){
                return 0;
            }
        }
        else if (strcmp("bdecompress",cmds[2+i])==0){
            bdecom += 1;
            if(bdecom>bdecompress_max){
                return 0;
            }
        }
        else if (strcmp("gcompress",cmds[2+i])==0){
            gcom += 1;
            if(gcom>gcompress_max){
                return 0;
            }
        }
        else if (strcmp("gdecompress",cmds[2+i])==0){
            gdecom += 1;
            if(gdecom>gdecompress_max){
                return 0;
            }
        }
        else if (strcmp("encrypt",cmds[2+i])==0){
            encrypt += 1;
            if(encrypt>encrypt_max){
                return 0;
            }
        }

        else{
            decrypt += 1;
            if(decrypt>decrypt_max){
                return 0;
            }
        }
    }

    return 1;
}