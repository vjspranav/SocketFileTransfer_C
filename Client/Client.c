// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define PORT 8000

typedef struct fRecieve{
    int numLoops;
    int progress;
    char filename[1024];
}f;

int rFile(int sock){
    int i;
    f file;
    int valread;
    char *nsd="Next";
    send(sock, nsd, 4, 0);
    valread = read(sock , &file, sizeof(struct fRecieve));
    printf("Recieving %s\n", file.filename);
    send(sock, nsd, 4, 0);
    for(i=0;i<file.numLoops;i++){
        memset(&file, 0, sizeof(struct fRecieve));
        valread = read(sock , &file, sizeof(struct fRecieve));
        printf("Progress: %d\n", file.progress);
        send(sock, nsd, 4, 0);
    }
    return 0;
}

int recieveFiles(int sock){
    char buffer[1024] = {0};
    int valread;
    printf("Awaiting Signal from server\n");
    while(1){
        memset(buffer, 0, 1024);
        valread = read( sock , buffer, 1024);
        if(strcmp(buffer, "Ready")==0){
            printf("Reciving File\n");
            rFile(sock);
            send(sock , "done", 4, 0 );    
        }else if(strcmp(buffer, "Done")==0){
            printf("Recived Files\n");
            send(sock , "Thank you\n", 10, 0 );    
            return 0;
        }else if(strcmp(buffer, "Failed")==0){
            printf("This File doesn't exist or is missing permission\n");
            send(sock , "done", 4, 0 );    
        }else{
            printf("buffer=%s\n", buffer);
            break;
        }
    }
    printf("Something went Wrong\n");
    return -1;
}

int startShell(int sock){
    char hello[200];
    char buffer[1024] = {0};
    int valread;
    while(1){
        memset(buffer, 0, 1024);
        printf("> ");
        scanf(" %[^\n]s", hello);
        if(strcmp("get", hello)!=0)
            send(sock , hello , strlen(hello) , 0 );  // send the message.
        printf("%s message sent\n", hello);
        if(strncmp("get", hello, 3)==0){
            if(strcmp("get", hello)!=0)
                recieveFiles(sock);
            else{
                printf("No filename provided\n");
                continue;
            }
        }
        if(strcmp(hello, "exit")==0)
            break;
        valread = read( sock , buffer, 1024);  // receive message back from server, into the buffer
        printf("%s\n",buffer);
    }
    printf("Transaction ended successfully\n");
    return 0;
}

int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
                                                // which is meant to be, and rest is defined below

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converts an IP address in numbers-and-dots notation into either a 
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    printf("** Connection Successfull **\nWelcome to inter device file transfer mechanism\n1. ls to view available files\n2. get <filename> to download file from the dir of server\n3. exit to exit\n");
    startShell(sock);
    return 0;
}
