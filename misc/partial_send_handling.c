//a little function to handle Partial send() s and make sure no data is left in the buffer

#include<sys/types.h>
#include<sys/socket.h>

int sendall(int s, char *buf, int *len)
{
	int total = 0;  //how many bytes are sent
	int bytesleft = *len; //bytes left to send
	int n;

	while(total < *len) {
		n = send(s, buf+total, bytesleft, 0);
		if (n==-1) {break;}
		total += n;
		bytesleft -= n;
	}

	*len = total; //returning number actually sent
		      
	return n==-1?-1:0; //-1 if fail, 0 if success
} 
