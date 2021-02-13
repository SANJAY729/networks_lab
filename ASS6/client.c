#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>
#define CFNAME "output.txt"
void error(const char *msg)
{
    perror(msg);
    exit(0);
}
int isdelimiter(char c){
    if(c==' '||c=='.'||c==','||c==';'||c==':'||c=='\t'||c=='\n'){
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

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
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
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if(n==0){
        printf("ERR 01: File Not Found");exit(0);
    }
    else{
        fd=open(CFNAME,O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    }
    cw+=countWords(buffer,n,' ');cb+=n;prev = buffer[n-1];
    n = write(fd,buffer,n);

    if (n < 0) error("ERROR writing to socket");
    bzero(buffer,255);
    
    while((n = read(sockfd,buffer,255))>0){
        // printf("%d\n",n);
        // write(1,buffer,n);
        cw+=countWords(buffer,n,prev);cb+=n;prev = buffer[n-1];
        n = write(fd,buffer,n);
        if (n < 0) error("ERROR writing to socket");
        bzero(buffer,255);
    }
    printf("The file transfer is successful. Size of the file = %d bytes, no. of words = %d",cb,cw);
    // if (n < 0) 
    //      error("ERROR reading from socket");
    close(sockfd);
    return 0;
}
