

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
int search(int arr[5],int fd){
    for(int i=0;i<5;i++){
        if(arr[i]==fd){
            return i;
        }
    }
    return -1;
}
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int parse(char* buf,int n,char* dst, char* msg,int cr){
    int i=0,nb=0,j=0;
    while (buf[i]!='/'){
        dst[i++]=buf[i];
    }i++;
    for(;i<n;i++){
        msg[j++]=buf[i];nb++;
    }msg[j]=(char)(cr+48);
    //printf("MSg %c\n",msg[j]);
    return nb+1;    
}


int main(int argc, char const *argv[])
{
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    char buf[256],dst[10],msg[256];    // buffer for client data
    int nbytes;
	char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;
    int user[5],user_fd[5],port[5],curr_user;
    int u = atoi(argv[1]);
    curr_user=u;
    char ports[5][10]={"100","101","102","103","104"};
    for(i=0;i<5;i++){
        user[i]=0;port[i]=100+i;
    }user[u]=1;user_fd[u]=STDIN_FILENO;

	struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, ports[u], &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}
	
	for(p = ai; p != NULL; p = p->ai_next) {
    	listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) { 
			continue;
		}
		
		// lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}

		break;
	}

	// if we got here, it means we didn't get bound
	if (p == NULL) {
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);
    FD_SET(STDIN_FILENO, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one
    printf("%d\n",listener);
    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                //printf("fd = %d listner = %d\n",i,listener);
                if (i == listener) {
                    // handle new connections
                    //printf("ALLO\n");
                    addrlen = sizeof remoteaddr;
					newfd = accept(listener,
						(struct sockaddr *)&remoteaddr,
						&addrlen);
                    
					if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        char hoststr[100],portstr[100];
                        getnameinfo((struct sockaddr *)&remoteaddr,(socklen_t)sizeof(struct sockaddr_storage),hoststr, sizeof(hoststr), portstr, sizeof(portstr), NI_NUMERICHOST | NI_NUMERICSERV);
                        printf("selectserver: new connection from %s on "
                            "socket %d\n",
							inet_ntop(remoteaddr.ss_family,
								get_in_addr((struct sockaddr*)&remoteaddr),
								remoteIP, INET6_ADDRSTRLEN),
							newfd);
                        //printf("NEWFD = %d\n",newfd);
                        // u=atoi(portstr)-100;printf("%s %s\n",hoststr,portstr);
                        // user[u]=1;
                        // user_fd[u]=newfd;printf("YALLO\n");
                    }
                } else {
                    if(i!=STDIN_FILENO){//printf("listner1 = %d\n",listener);
                        // handle data from a client
                        if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                            // got error or connection closed by client
                            if (nbytes == 0) {
                                // connection closed
                                u=search(user_fd,i);
                                printf("selectserver:  user %d hung up\n", u);
                            } else {
                                perror("recv");
                            }
                            
                            close(i); // bye!
                            
                            FD_CLR(i, &master); // remove from master set
                        } else {
                            // we got some data from a client
                            
                            u=buf[nbytes-1]-48;
                            //printf("%c\n",buf[nbytes-1]);
                            user[u]=listener;//printf("listner1.5 = %d\n",listener);
                            user_fd[u]=i;
                            buf[nbytes-1]='\0';
                            // for (int j = 0; j < nbytes; j++)
                            // {
                            //     printf("%c ",buf[j]);
                            // }printf("\n");
                            printf("User %d: %s\n",u,buf);
                            //write(STDOUT_FILENO,buf,nbytes-1);
                        }
                    }
                    else{//printf("listner2 = %d\n",listener);
                        // handle data from a stdin
                        if((nbytes = read(i, buf, sizeof(buf))) <= 0) {
                            // got error or connection closed by client
                            if (nbytes == 0) {
                                // connection closed
                                printf("selectserver: socket %d hung up\n", i);
                            } else {
                                perror("recv");
                            }
                            user[u]=0;
                            close(i); // bye!
                            FD_CLR(i, &master); // remove from master set
                        } else {
                            // we got some data from a stdin
                            nbytes=parse(buf,nbytes,dst,msg,curr_user);
                            u=atoi(dst);
                            if(user[u]){
                                send(user_fd[u],msg,nbytes,0);
                            }
                            else{
                                int sock_fd,status,portno;
                                struct sockaddr_in serv_addr;
                                struct hostent *server; 		
                                portno = port[u];
                                sock_fd = socket(AF_INET, SOCK_STREAM, 0);
                                if (sock_fd < 0) 
                                    {perror("ERROR opening socket");exit(0);}
                                server = gethostbyname("localhost");
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
                                user[u]=listener;
                                user_fd[u]=sock_fd;
                                FD_SET(sock_fd, &master); // add to master set
                                if (sock_fd > fdmax) {    // keep track of the max
                                    fdmax = sock_fd;
                                }
                                send(user_fd[u],msg,nbytes,0);printf("NEW\n");
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    return 0;
}
