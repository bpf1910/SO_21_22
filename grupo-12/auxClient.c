#include "auxClient.h"

char read_buffer[BUFFER_SIZE];
int read_buffer_pos = 0;
int read_buffer_end = 0;

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


/*--------------------------- SEND TO SERVER PROC-FILE ---------------------------*/

void sendToServerPF(int argc, char *argv[]){
    char * path = "/tmp/fifo";

    int fd = open(path,O_WRONLY), j;

    char buffer[4096]="";

    sprintf(buffer,"%d %d",getpid(),argc-2);
    
    for(int i = 2; i < argc ; i++){
        sprintf(buffer,"%s %s",buffer,argv[i]);
        
    }

    j = sprintf(buffer,"%s \n",buffer);
    write(fd,buffer,j);
    //write(1,buffer,j);

    close(fd);
}


/*--------------------------- SEND TO SERVER STATUS ---------------------------*/

void sendToServerStatus(int argc, char *argv[]){
    char * path = "/tmp/fifo";

    int fd = open(path,O_WRONLY), j;

    char buffer[4096]="";

    j = sprintf(buffer,"%d %d %s \n",getpid(),argc-1,argv[1]);
    write(fd,buffer,j);

    //write(1,buffer,j);

    close(fd);
}


/*--------------------------- RECEIVE FROM SERVER ---------------------------*/

void receiveFromServer(){
    int fifo_client_r,fifo_client_w,bytes_read;
    char path[12];
    char buffer[4096];

    for (int i = 0;i<11;i++){
        path[i] = '\0';
    }

    sprintf(path,"/tmp/%d",getpid());
    mkfifo(path, 0666);

    if ((fifo_client_r = open(path, O_RDONLY)) == -1) {

        perror("rd open");
    }

    if((fifo_client_w = open(path, O_WRONLY)) == -1){
        perror("wr open");
    }

    while((bytes_read = readln(fifo_client_r, buffer, 4096)) > 0) {
        char * str = (char*) malloc(bytes_read*sizeof(char));
        strcpy(str,buffer);
        
        if(strcmp(str,"stop")==0){
            close(fifo_client_w);
        }
        else{
            write(1,buffer,bytes_read);
            write(1,"\n",1);
        }    
    }
    
    close(fifo_client_r);
}