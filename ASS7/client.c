// TO COMPILE: gcc file_client.c -o client 
// TO RUN: ./client localhost <portno.>
#include <wait.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define FNAME "client.txt"
#define PORT 8080
#define MAX_CHAR 100
#define BUFFER_SIZE 20


int get_filesize(int *num, int fd)
{
    int32_t rt;
    char *data = (char *)&rt;
    int left = sizeof(rt);
    int cr;
    do
    {
        cr = read(fd, data, left);
        if (cr <= 0)
        { /* instead of rt */
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
             
            }
            else if (errno != EINTR)
            {
                return -1;
            }
        }
        else
        {
            data += cr;
            left -= cr;
        }
    } while (left > 0);
    *num = ntohl(rt);
    return 0;
}

 
int getFile(int sock_fd, char * filename, int file_size,int buffer_size, int *X, int *Y)
{
    int cnt = file_size/buffer_size, last_size = file_size - buffer_size*cnt,total_bytes=0;
    int fd = open(filename,O_WRONLY|O_CREAT|O_TRUNC,0644), i=0;
    char buffer[buffer_size+1];
    for(i=0;i<cnt;i++)
    {
        *Y = recv(sock_fd,buffer,buffer_size,MSG_WAITALL);        
        buffer[buffer_size]='\0';
        write(fd,buffer,buffer_size);
        *X +=1;
	total_bytes += *Y;
    }
    if(last_size)
    {
        *Y = recv(sock_fd, buffer, last_size, MSG_WAITALL);
        buffer[last_size] = '\0';
        write(fd, buffer, last_size);
        *X+=1;
	total_bytes += *Y;
    }
    close(fd);
    return total_bytes;

}
int main(int argc, char *argv[])
{
    int sock_fd,status,portno;
    struct sockaddr_in serv_addr;
    struct hostent *server; 		
    if (argc != 3) { printf("Please give the following arguments: localhost portno.\n");
                    exit(1);
    }
    portno = atoi(argv[2]);
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) 
        {perror("ERROR opening socket");exit(0);}
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
    if (connect(sock_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        {perror("ERROR connecting");exit(0);}
    printf("Enter filename : ");
    char filename[MAX_CHAR];
    scanf("%s",filename);
    send(sock_fd,filename,strlen(filename)+1,0);
    char temp[2];
    recv(sock_fd,temp,1,0);
    temp[1]='\0';
    if(!strcmp(temp,"E"))
    {
        printf("File Can't be found\n");
        close(sock_fd);
        return 0;
    }
    else
    {
        int n;
        get_filesize(&n,sock_fd);
        printf("File Size: %d\n",n);
        int total_blocks = 0,final_block_size = 0, total_bytes = 0;
        total_bytes =  getFile(sock_fd,FNAME,n,BUFFER_SIZE,
                                &total_blocks,&final_block_size);
        printf("\nFile Received Successfully!\n");
        printf("Buffer Size: %d\n",BUFFER_SIZE);
        printf("Total Blocks: %d\nFinal BLock Size: %d\nTotal Bytes Received: %d\n",
                total_blocks,final_block_size, total_bytes);
    }
    close(sock_fd);
    return 0;
}
