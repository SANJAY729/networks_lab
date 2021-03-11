// TO COMPILE: gcc file_server.c -o server
// TO RUN: sudo ./server <portno.>
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

#define PORT 8080
#define MAX_CHAR 100
#define BUFFER_SIZE 20

 
int sendInt(int num, int fd)
{
    int32_t cv = htonl(num);
    char *data = (char *)&cv;
    int left = sizeof(cv);
    int cr;
    do
    {
        cr = write(fd, data, left);
        if (cr < 0)
        {
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
    return 0;
}

int fileSize(int fd)
{
    struct stat f_info;
    fstat(fd, &f_info);
    return (int)f_info.st_size;
}

// Read bytes in block of buffer_size and send
void sendFile(int fd, int sock_fd, int buffer_size)
{
    int n = 0;
    char buffer[buffer_size];
    do
    {
        n = read(fd, buffer, buffer_size - 1);
        buffer[n] = '\0';
        send(sock_fd, buffer, strlen(buffer), 0);
    } while (n == buffer_size - 1);
    return;
}

int main(int argc, char *argv[])
{
    int sock_fd,newsock_fd,fd,portno;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
		int n;
		if (argc < 2) {
			 printf("ERROR, no port provided\n");
			 exit(1);
		}
		sock_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (sock_fd < 0) 
			{perror("ERROR opening socket");exit(0);}
		bzero((char *) &serv_addr, sizeof(serv_addr));
		portno = atoi(argv[1]);
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(portno);
		if (bind(sock_fd, (struct sockaddr *) &serv_addr,
				    sizeof(serv_addr)) < 0) 
				   { perror("binding");printf("\nTry using 'sudo'.\n");exit(0);}
		listen(sock_fd,5);
		cli_len = sizeof(cli_addr);
    listen(sock_fd,5);
    while(1)
    {
        printf("Waiting for new connection.....\n");
        cli_len = sizeof(cli_addr);
        newsock_fd = accept(sock_fd,(struct sockaddr*)&cli_addr,&cli_len);
        if (newsock_fd < 0)
        {
            perror("Connection Error\n");
            continue;
        }
        else
        {
            printf("Connected......Expecting filename....\n");
            char filename[MAX_CHAR];
						bzero(filename,MAX_CHAR);
            int temp =recv(newsock_fd, filename, MAX_CHAR, 0);
						filename[temp-1]='\0';
            printf("File name received: %s\n", filename);
 						if( access( filename, F_OK ) != 0 ){
								perror("[ERROR] File can't be found\n");
                send(newsock_fd, "E", 1, 0);
                close(newsock_fd);
                continue;
						}
            fd = open(filename, O_RDONLY);
						if(fd<0){
							perror("Error opening file");
							continue;
						}            
            send(newsock_fd, "L", 1, 0);
            int filesize = fileSize(fd);
						struct stat f_info;
						fstat(fd, &f_info);
            sendInt((int)f_info.st_size, newsock_fd);
            sendFile(fd, newsock_fd, BUFFER_SIZE);
            printf("File Sent Successfully\n");
            close(newsock_fd);
            close(fd);            
        }        
    }
    return 0;
}
