#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT "9034" //port we are listening to
#define MAX_NICK 32 

char nicknames[FD_SETSIZE][MAX_NICK];

//coverting socket to IP address string

const char *inet_ntop2(void *addr, char *buf, size_t size)
{
	struct sockaddr_storage *sas = addr;
	struct sockaddr_in *sa4;
	struct sockaddr_in6 *sa6;
	void *src;

	switch (sas->ss_family) {
		case AF_INET:
			sa4 = addr;
			src = &(sa4->sin_addr);
			break;
		case AF_INET6:
			sa6 = addr;
			src = &(sa6->sin6_addr);
			break;
		default:
			return NULL;
		}

	return inet_ntop(sas->ss_family, src, buf, size);
}   

// returning a listening socket 

int get_listener_socket(void)
{
	struct addrinfo hints, *ai, *p;
	int yes = 1;
	int rv;
	int listener;

	//get a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, PORT, &hints , &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}

	for(p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p ->ai_socktype, p->ai_protocol);
		if(listener < 0 ) {
			continue;
		}

		//lose "address already in use" error msg
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen)< 0){
			close(listener);
			continue;
		}

		break;
	}

	//error handling
	if (p == NULL) {
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}


	freeaddrinfo(ai); //freeing the linked list
	
	//listen
	if(listen(listener, 10) == -1) {
		perror("listen");
		exit(3);
	}

        printf("selectserver: waiting for connections on port %s...\n", PORT);

	return listener;
}


//adding new incoming connectinos to proper sets

void handle_new_connection(int listener, fd_set *master, int *fdmax)
{
	socklen_t addrlen;
	int newfd;
	struct sockaddr_storage remoteaddr;
	char remoteIP[INET6_ADDRSTRLEN];

	addrlen = sizeof remoteaddr;
	newfd = accept(listener,
	        (struct sockaddr *)&remoteaddr,
		&addrlen);
	
	if (newfd == -1) {
		perror("accept");
	}
	else{
		FD_SET(newfd, master);	//add to master set
                snprintf(nicknames[newfd], sizeof(nicknames[newfd]), "User%d", newfd);
		if (newfd > *fdmax) { 
		    *fdmax = newfd;
		}
		printf("selectserver: new connection from %s on""socket %d\n",
                        inet_ntop2(&remoteaddr, remoteIP, sizeof remoteIP),
			newfd);

                char *welcome_msg = "Welcome to the chat server! Use /nick <name> to change your name.\n";
                  if (send(newfd, welcome_msg, strlen(welcome_msg), 0) == -1) {
                      perror("send welcome");
                  }

	}
}

//broadcasting a msg to all clients

void broadcast( char* buf, int nbytes, int listener, int s, 
		fd_set *master, int fdmax)
{
	for(int j =0;j<=fdmax; j++){
		//send to everyone
		if (FD_ISSET(j,master)){
			//except to the listener and oneself
			if(j != listener && j != s) {
				if (send(j, buf, nbytes, 0) == -1) {
					perror("send");
				}
			}
		}
	}
}

//handling client data and hangups

void handle_client_data(int s, int listener, fd_set *master, int fdmax)
{
	char buf[256];
	int nbytes;

	if((nbytes = recv(s,buf, sizeof buf, 0)) <= 0){
		//connection closed or error
		if(nbytes == 0){
			//connection closed
			printf("selectserver: socket %d hung up\n", s);
		} else {
		   perror("recv");
		}
		close(s);
		FD_CLR(s, master);
	} else {
    //if client typed /nick change their name
    if (strncmp(buf, "/nick ", 6) == 0) {
        sscanf(buf + 6, "%31s", nicknames[s]);
    } else {
        //otherwise prepend their nickname and broadcast
        char out_buf[300];
        int out_len = snprintf(out_buf, sizeof(out_buf), "<%s>: %.*s", nicknames[s], nbytes, buf);
        broadcast(out_buf, out_len, listener, s, master, fdmax);
    }
  }
}


//main

int main(void)
{
	fd_set master; //master file descriptor list
	fd_set read_fds; //temp file descriptor list
	int fdmax;       //max file descriptor number

	int listener;

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	listener = get_listener_socket();

	//add listener to master set
	FD_SET(listener, &master);
	fdmax = listener;

	//main loop
	for(;;) {
		read_fds = master;
		if (select(fdmax + 1, &read_fds,NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}

		for(int i = 0; i <= fdmax; i++ ) {
			if (FD_ISSET(i, &read_fds)) {
				if ( i == listener){
					handle_new_connection(i, &master, &fdmax);
				} else {
				        handle_client_data(i, listener, &master, fdmax);
				}
			}
		}
	}

	return 0;
}


