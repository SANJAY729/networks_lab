// TO COMPILE: gcc file_server.c -o server
// TO RUN: sudo ./server <portno.>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#define MAX_BYTES 100 //Maximum buffer size for server (can be changed)
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     int fd;
     socklen_t clilen;
     char buffer[MAX_BYTES];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 2) {
         printf("ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
             { perror("binding");printf("\nTry using 'sudo'.\n");exit(0);}
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     printf("Server Started!\n");
     while(1){
        printf("Waiting for new connection ....\n");
        newsockfd = accept(sockfd, 
                    (struct sockaddr *) &cli_addr, 
                    &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");
        bzero(buffer,MAX_BYTES);
        printf("Expecting Filename ....\n");
        n = recv(newsockfd,buffer,MAX_BYTES,0);
        if (n < 0) error("ERROR reading from socket");
        
        //printf("%s\n",buffer);
        buffer[n-1]='\0';
        if( access( buffer, F_OK ) == 0 ) {
            //printf("%s\n",buffer);
            if((fd=open(buffer,O_RDONLY))==-1){
            error("open");
            }bzero(buffer,MAX_BYTES);
            buffer[0]='\0';
            n = send(newsockfd,buffer,1,0);
            printf("Sending file data ......");bzero(buffer,MAX_BYTES);
            while((n=read(fd,buffer,MAX_BYTES-1))>0){
                //printf("ALLO\n");
                // write(1,buffer,n);
                n = send(newsockfd,buffer,n,0);
                if (n < 0) error("ERROR writing to socket");
                bzero(buffer,MAX_BYTES);
            }
        printf("Done!\n");}
        else{printf("File not found\n");}
        close(newsockfd);
        printf("Closed connection\n");
        }
     close(sockfd);
     return 0; 
}
