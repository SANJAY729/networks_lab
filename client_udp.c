/* UDP client in the internet domain */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
void error(const char *);
int main(int argc, char *argv[])
{
   int sock, n,i=0;
   unsigned int length;
   struct sockaddr_in server, from;
   struct hostent *hp;
   char buffer[256];
   FILE *fp;
   char* fname="clientfile.txt";
   
   if (argc != 3) { printf("Usage: server port\n");
                    exit(1);
   }
   sock= socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) error("socket");

   server.sin_family = AF_INET;
   hp = gethostbyname(argv[1]);
   if (hp==0) error("Unknown host");

   bcopy((char *)hp->h_addr, 
        (char *)&server.sin_addr,
         hp->h_length);
   server.sin_port = htons(atoi(argv[2]));
   length=sizeof(struct sockaddr_in);
   printf("Please enter file name: ");
   bzero(buffer,256);
   fgets(buffer,255,stdin);
   //printf("%s",buffer);
   n=sendto(sock,buffer,
            strlen(buffer),0,(const struct sockaddr *)&server,length);
   if (n < 0) error("Sendto");bzero(buffer,256);
   n = recvfrom(sock,buffer,256,0,(struct sockaddr *)&from, &length);
   if (n < 0) error("recvfrom");
   write(1,"Got an ack: ",12);
   write(1,buffer,n);
   if(strcmp(buffer,"FILE_NOT_FOUND")==0){
       printf("\nFile Not Found\n");
       exit(1);
   }
   if(strcmp(buffer,"WRONG_FILE_FORMAT")==0){
       printf("\nWrong File Format\n");
       exit(1);
   }
   //printf("%s",buffer);
   if(strcmp(buffer,"HELLO")==0){
       if((fp=fopen(fname,"w"))==NULL){
           perror("fopen");
       }
   }
   while(1){
   char word[100]="WORD";
   char *num=(char*)malloc((int)((ceil(log10(i+1))+1)*sizeof(char)));printf("HI\n");
   sprintf(num,"%d",i+1);  
   //printf("Please enter the message: ");
   bzero(buffer,256);
   strcat(word,num);
   n=sendto(sock,word,
            strlen(word),0,(const struct sockaddr *)&server,length);
   if (n < 0) error("Sendto");
   n = recvfrom(sock,buffer,256,0,(struct sockaddr *)&from, &length);
   if (n < 0) error("recvfrom");
   write(1,"Got an ack: ",12);printf("Ha\n");
   fprintf(fp,buffer);printf("%s",buffer);
   if(strcmp(buffer,"END")==0){
       fclose(fp);
       break;
   }i++;   
   }
   close(sock);
   return 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
