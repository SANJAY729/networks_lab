/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

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
     char buffer[100];
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
        newsockfd = accept(sockfd, 
                    (struct sockaddr *) &cli_addr, 
                    &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");
        bzero(buffer,100);
        n = recv(newsockfd,buffer,100,MSG_BATCH);
        if (n < 0) error("ERROR reading from socket");
        printf("Expecting Filename ....\n");
        //printf("%s\n",buffer);
        buffer[n-1]='\0';
        if( access( buffer, F_OK ) == 0 ) {
            //printf("%s\n",buffer);
            if((fd=open(buffer,O_RDONLY))==-1){
            error("open");
            }bzero(buffer,100);
            printf("Sending file data ......");
            while((n=read(fd,buffer,99))>0){
                //printf("ALLO\n");
                // write(1,buffer,n);
                n = send(newsockfd,buffer,n,MSG_BATCH);
                if (n < 0) error("ERROR writing to socket");
                bzero(buffer,100);
            }
        }printf("Done!\n");
        //  printf("Here is the message: %s\n",buffer);
        //  n = write(newsockfd,"I got your message",18);
        //  if (n < 0) error("ERROR writing to socket");
        close(newsockfd);
        printf("Closed connection\n");
        }
     close(sockfd);
     return 0; 
}
