//multichat pollserver

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<poll.h>

#define PORT "9034"

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

//returning a listening socket

int get_listener_socket(void)
{
	int listener;
	int yes = 1;
	int rv;

	struct addrinfo hints, *ai, *p;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0){
		fprintf(stderr, "pollserver: %s\n", gai_strerror(rv));
		exit(1);
	}

	for(p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol);
		if (listener < 0) {
			continue;
		}

		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int));

		if(bind(listener, p->ai_addr, p->ai_addrlen) < 0){
			close(listener);
			continue;
		}

		break;
	}

	if(p == NULL) {
		return -1;
	}

	freeaddrinfo(ai);

	if (listen(listener, 10) == -1) {
		return -1;
	}

	return listener;
}

//adding a new file descriptor to the set

void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count,
		int *fd_size)
{
	if (*fd_count == *fd_size) {
	    *fd_size *= 2;
            *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
	}

        (*pfds)[*fd_count].fd = newfd;
        (*pfds)[*fd_count].events = POLLIN;
	(*pfds)[*fd_count].revents = 0;

	(*fd_count)++;
}

//removing a file descriptor at a given index from the set

void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
	pfds[i] = pfds[*fd_count-1];

	(*fd_count)--;
}

//handling incoming connections

void handle_new_connection(int listener, int *fd_count,
		int *fd_size, struct pollfd **pfds)
{
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen;
	int newfd;
	char remoteIP[INET6_ADDRSTRLEN];

	addrlen = sizeof remoteaddr;
	newfd = accept(listener, (struct sockaddr *)&remoteaddr,
			&addrlen);

	if(newfd == -1){
		perror("accept");
	}else {
		add_to_pfds(pfds, newfd, fd_count, fd_size);

		printf("pollserver: new connection from %s on socket %d\n",
				inet_ntop2(&remoteaddr, remoteIP, sizeof remoteIP),
				newfd);
	}
}

//handle regular client data or client hangups

void handle_client_data(int listener, int *fd_count,
		struct pollfd *pfds, int *pfd_i)
{
	char buf[256];

	int nbytes = recv(pfds[*pfd_i].fd, buf, sizeof buf, 0);

	int sender_fd = pfds[*pfd_i].fd;

	if (nbytes <= 0) {
		if(nbytes == 0){
			printf("pollserver: socket %d hung up\n", sender_fd);
		} else {
			perror("recv");
		}

		close(pfds[*pfd_i].fd);

		del_from_pfds(pfds, *pfd_i, fd_count);

		(*pfd_i)--;

	} else {
		printf("pollserver: recv from fd %d: %.*s", sender_fd,
				nbytes, buf);

		for(int j =0; j < *fd_count; j++) {
			int dest_fd = pfds[j].fd;
			
			if (dest_fd != listener && dest_fd != sender_fd) {
				if ( send(dest_fd, buf, nbytes, 0) == -1) {
					perror("send");
				}
			}
		}

	}
}


//processing all existing connections

void process_connections(int listener, int *fd_count, int *fd_size,
		struct pollfd **pfds)
{

	for(int i = 0;i < *fd_count ; i++) {
		//if someone is ready to read
		if ((*pfds)[i].revents & (POLLIN | POLLHUP)) {
			//we got one!
			if ((*pfds)[i].fd == listener){
				//if we are the listener, its a new connection
				handle_new_connection(listener, fd_count, fd_size,
						pfds);
			} else {
				//otherwise we are just a regular client
				handle_client_data(listener, fd_count, *pfds, &i);
			}
		}
	}
}

//main : creating a listener and connection set, and looping forever processing
//connections

int main(void)
{
	int listener;

	//starting off with room for 5 connections
	//can be realloced as necessary
	int fd_size = 5;
	int fd_count = 0;
	struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

	//set up and get a listening socket
	listener = get_listener_socket();

	if (listener == -1) {
		fprintf(stderr, "error getting listening socket\n");
		exit(1);
	}

	//adding listner to set;
	//report ready to read an incoming connection
	pfds[0].fd = listener;
	pfds[0].events = POLLIN;

	fd_count = 1;

	puts("pollserver: waiting for connections...");

	//main loop
	for(;;){
		int poll_count = poll(pfds, fd_count, -1);

		if(poll_count == -1) {
			perror("poll");
			exit(1);
		}

		//running thru the connections looking for data to read
		process_connections(listener, &fd_count, &fd_size, &pfds);
	}

	free(pfds);
}
