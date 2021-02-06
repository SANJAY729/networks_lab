/* Creates a datagram server.  The port 
   number is passed as an argument.  This
   server runs forever */

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#define MAX_FILE_LINES 100
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
 int sock, length, n;
 socklen_t fromlen;
 struct sockaddr_in server;
 struct sockaddr_in from;
 char *buf=(char*)malloc(1024*sizeof(char));
 char * line = NULL,*mes = NULL;
 char **lines=(char**)malloc(sizeof(char*)*MAX_FILE_LINES);
 for(int i=0;i<MAX_FILE_LINES;i++){
     lines[i]=(char*)malloc(sizeof(char)*200);
 }
 FILE * fp;
 size_t len=0;
 ssize_t read;
 const char* word="WORD";

 if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(0);
 }
 
 sock=socket(AF_INET, SOCK_DGRAM, 0);
 if (sock < 0) error("Opening socket");
 length = sizeof(server);
 bzero(&server,length);
 server.sin_family=AF_INET;
 server.sin_addr.s_addr=INADDR_ANY;
 server.sin_port=htons(atoi(argv[1]));
 if (bind(sock,(struct sockaddr *)&server,length)<0) 
     error("binding");
 fromlen = sizeof(struct sockaddr_in);
 bzero(buf,1024);
 n = recvfrom(sock,buf,1024,0,(struct sockaddr *)&from,&fromlen);

 if (n < 0) error("recvfrom");
 //write(1,"Received filename: ",21);
 //write(1,buf,n);printf("%s",buf);
 buf[strlen(buf)-1]='\0';
 if( access( buf, F_OK ) == 0 ) {
    //printf("%s",buf);
    if((fp=fopen(buf,"r"))==NULL){
      error("fopen");
    }
    else{
      read=getline(&line,&len,fp);
      if(read==-1){
        n = sendto(sock,"WRONG_FILE_FORMAT",17,
            0,(struct sockaddr *)&from,fromlen);
        if (n  < 0) error("sendto");
      }
      else if(strcmp("HELLO\n",line)!=0){
        n = sendto(sock,"WRONG_FILE_FORMAT",17,
            0,(struct sockaddr *)&from,fromlen);
        if (n  < 0) error("sendto");
      }
      else{
        strcpy(lines[0],line);;
        n = sendto(sock,"HELLO",5,
            0,(struct sockaddr *)&from,fromlen);
        if (n  < 0) error("sendto");
        int i=1;
        while ((read = getline(&line, &len, fp)) != -1) {          
          strcpy(lines[i],line);
          i++;
        }
      }
    }
  } else {
    n = sendto(sock,"FILE_NOT_FOUND",15,
            0,(struct sockaddr *)&from,fromlen);
    if (n  < 0) error("sendto");
  } 
  
 while (1) {
     bzero(buf,1024);
     n = recvfrom(sock,buf,1024,0,(struct sockaddr *)&from,&fromlen);
     if (n < 0) error("recvfrom");
     write(1,"Received a datagram: ",21);
     //write(1,buf,n);
     int i=0;
     
     if(strncmp(buf,word,4)==0){
      
      mes=lines[atoi(buf+4)-1];
      
      printf("%d %s\n",atoi(buf+4)-1,mes);
      n = sendto(sock,mes,strlen(mes),
                0,(struct sockaddr *)&from,fromlen);
      if (n  < 0) error("sendto");
     }     
 }
 return 0;
 }

