#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h> 
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define PORT 8000
#define BYTE 0
#define KB 1
#define MB 2
#define GB 3

int isFile(const char *path){
    struct stat ps;
    stat(path, &ps);
    return S_ISREG(ps.st_mode);
}

float convert(float num){
    int type=BYTE;
    if(num>1024){
        type=KB;
        num=num/1024;
        if(num>1024){
            type=MB;
            num=num/1024;
            if(num>1024){
                type=GB;
                num=num/1024;
                return num;
            }else{
                return num;
            }
        }else{
            return num;
        }
    }else{
        return num;
    }   
}

char *mbgb(long long int num){
    int type=BYTE;
    if(num>1024){
        type=KB;
        num=num/1024;
        if(num>1024){
            type=MB;
            num=num/1024;
            if(num>1024){
                type=GB;
                num=num/124;
                return "GB";
            }else{
                return "MB";
            }
        }else{
            return "KB";
        }
    }else{
        return "B";
    }    
}

int ls(int new_socket){
    int empty=0;
    char lsd[1024]={0};
    struct dirent *de;  // Pointer for directory entry 
    struct stat fs;    
    DIR *dr = opendir(".");   
    if (dr == NULL) 
    { 
        printf("Could not open current directory" ); 
        return 0; 
    } 
    while ((de = readdir(dr)) != NULL){ 
        if((de->d_name[0]!='.') && (strncmp("Server", de->d_name, 6) && strcmp("a.out", de->d_name))){
            if(isFile(de->d_name)){
                empty=1;
                stat(de->d_name,&fs); 
                char str[1024];
                sprintf(str, "%s\t%.1f %s\n", de->d_name, convert(fs.st_size), mbgb(fs.st_size));
                strcat(lsd, str);   
            }
        }
    }
    closedir(dr);     
    if(!empty)
        strcat(lsd, "No Downloadable files found\n");
    send(new_socket , lsd, strlen(lsd) , 0 );  // send the message.
    return 0; 
}

int getFileNames(char* command, char** parsed){
    int i=0;    
    strtok(command, " ");
    parsed[i] = strtok(NULL, " ");
    i+=1;
    while(1){
        parsed[i] = strtok(NULL, " ");
        if(parsed[i]==NULL){
            return i;
        }
        i+=1;          
    }
    return -1;
}

int checkFiles(char **parsed, int numFiles[][2]){
    int i=0;
    printf("File\texists Read\n");
    while(parsed[i]!=NULL){
        printf("%s\t", parsed[i]);
        if( access( parsed[i], F_OK ) != -1 ){
            numFiles[i][0]=1;
            numFiles[i][1]=(access(parsed[i], F_OK)!=-1)?1:0;
        }else{
            numFiles[i][0]=0;
            numFiles[i][1]=0;
        }
        printf("%d\t%d\n", numFiles[i][0], numFiles[i][1]);
        i+=1;
    }
}

int checkandDownload(char* command, int new_socket){
    int valread;
    char buffer[1024] = {0};
    char *parsed[1000];
    char *rsd="Ready";
    char *lsd="Done";
    char *fsd="Failed";
    int i=0;
    printf("Sending\n");
    int numFiles=getFileNames(command, parsed);
    int FileS[numFiles][2];
    checkFiles(parsed, FileS);
    printf("\n");
    for(i=0;i<numFiles;i++){
        memset(buffer, 0, 1024);
        if(FileS[i][0] && FileS[i][1]){
            send(new_socket, rsd, strlen(rsd), 0);
            // Send File
            valread = read(new_socket , buffer, 1024);    
            if(strcmp(buffer, "done")==0)
                printf("File %s Sent successfully\nNext\n", parsed[i]);
        }else{
            send(new_socket, fsd, strlen(fsd), 0);
            valread = read(new_socket , buffer, 1024);    
            if(strcmp(buffer, "done")==0)
                printf("File %s could not be sent\nNext\n", parsed[i]);
        }
    }
    send(new_socket , lsd, strlen(lsd) , 0 );  // send the message.
    valread = read(new_socket , buffer, 1024);    
    send(new_socket , "Server: Transfer Successfull\n", 29, 0 );  // send the message.
    return 0;
}

int startListening(int new_socket){
    int valread;
    char buffer[1024] = {0};
    char *hello = "Command Not found... Please try again\n";
    while(strcmp(buffer, "exit")!=0){                
        memset(buffer, 0, 1024);
        valread = read(new_socket , buffer, 1024);  // read infromation received into the buffer
        printf("Clent sent: %s\n",buffer, strlen(buffer));
        if(strcmp(buffer, "ls")==0){
            ls(new_socket);
        }else if(strncmp("get", buffer, 3)==0){
            checkandDownload(buffer, new_socket);
        }else if(strcmp(buffer, "exit")!=0){
            send(new_socket , hello , strlen(hello) , 0 );  // send the message.
        }
        printf("Waiting for command\n");
    }
    printf("Transaction Successful\n");
}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;  
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // This is to lose the pesky "Address already in use" error message
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc. 
    address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from all interfaces.
    address.sin_port = htons( PORT );    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Port bind is done. You want to wait for incoming connections and handle them in some way.
    // The process is two step: first you listen(), then you accept()
    if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Listening for Clients\n");
    // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                       (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    
    }
    printf("Connection Successfull\n");
    startListening(new_socket);
    return 0;
}
