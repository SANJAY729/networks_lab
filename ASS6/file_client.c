// TO COMPILE: gcc file_client.c -o client 
// TO RUN: ./client localhost <portno.>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>
#include <poll.h>
#define CFNAME "output.txt"
#define MAX_BYTES 50 //Maximum buffer size for client (can be changed)
void error(const char *msg)
{
    perror(msg);
    exit(0);
}
int isdelimiter(char c){
    if(c==' '||c=='.'||c==','||c==';'||c==':'||c=='\t'||c=='\0'||c=='\n'){
        return 1;
    }
    return 0;
}
int countWords(char* buf,int n,char prev){
    int i=0,cw=0;
    if(!isdelimiter(prev)){
        while(!isdelimiter(buf[i])&&i<n){
            i++;
        }
    }
    while(isdelimiter(buf[i])&&i<n){
                i++;
    }
    while(i<n){
        if(isdelimiter(buf[i])){
            cw++;
            while(isdelimiter(buf[i])&&i<n){
                i++;
            }
        }
        else{
            i++;
        }        
    }
    if(!isdelimiter(buf[n-1])){
        cw++;
    }
    return cw;
}
int main(int argc, char *argv[])
{
    int sockfd, portno, n,fd,cw=0,cb=0;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char prev;
    char buffer[MAX_BYTES];
    if (argc != 3) { printf("Please give the following arguments: localhost portno.\n");
                    exit(1);
   }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    printf("Please enter filename: ");
    bzero(buffer,MAX_BYTES);
    fgets(buffer,MAX_BYTES-1,stdin);
    n = send(sockfd,buffer,strlen(buffer),0);
    if (n < 0) 
         error("ERROR writing to socket");
    n = recv(sockfd,buffer,1,0);
    bzero(buffer,MAX_BYTES-1);
    if(n==0){
        printf("ERR 01: File Not Found");exit(0);
    }
    else{
        fd=open(CFNAME,O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    }
    // cw+=countWords(buffer,n,' ');cb+=n;prev = buffer[n-1];
    // n = write(fd,buffer,n);

    if (n < 0) error("ERROR writing to socket");
    bzero(buffer,MAX_BYTES-1);
    
    while((n = recv(sockfd,buffer,MAX_BYTES-1,0))>0){
        // printf("%d\n",n);
        // write(1,buffer,n);
        //printf("ALLO\n");
        cw+=countWords(buffer,n,prev);cb+=n;prev = buffer[n-1];
        n = write(fd,buffer,n);
        if (n < 0) error("ERROR writing to socket");
        bzero(buffer,MAX_BYTES-1);
    }
    printf("The file transfer is successful. Size of the file = %d bytes, no. of words = %d",cb,cw);
    close(sockfd);
    return 0;
}
