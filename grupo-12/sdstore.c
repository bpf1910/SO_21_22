#include "auxClient.h"

int parser(int argc, char * argv[]){

    //./sdstored pro old new bla bla bla
    if(strcmp(argv[1],"proc-file")!=0){
        printf("Opção inválida: %s\n[Usage] ./sdstore proc-file samples/file-a outputs/file-a-output [transform]...\n",argv[1]);
        return -1;
    }

    for (int i = 4; i < argc; i++){
        if (strcmp(argv[i],"nop")!=0 && strcmp(argv[i],"bcompress")!=0 && strcmp(argv[i],"bdecompress")!=0 
                    && strcmp(argv[i],"gcompress")!=0 && strcmp(argv[i],"gdecompress")!=0 
                    && strcmp(argv[i],"encrypt")!=0 && strcmp(argv[i],"decrypt")!=0){
            
            printf("Comando invalido: %s\n[Usage] nop bcompress bdecompress gcompress gdecompress encrypt decrypt\n",argv[i]);
            return -1;
        }
    }
    return 0;   
}


int main(int argc, char *argv[]){
    int status;
    
    if (argc<2){
        printf("[Usage] ./sdstore proc-file samples/file-a outputs/file-a-output [transform]...\n");
        printf("[Usage] ./sdstore status\n");
        return -1;
    }
    else if (argc == 2){
        if(strcmp("status",argv[1])!=0){
            printf("[Usage] ./sdstore status\n");
            return -1;
        }
    }

    else{
        if (parser(argc, argv)<0){
            return -1;
        }
    }
    
    if (argc==2){
        sendToServerStatus(argc,argv);
        receiveFromServer();
                
    }
    else{
        sendToServerPF(argc,argv);
        receiveFromServer();
    }
        
    return 0;
}