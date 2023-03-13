#include "headsock.h"

#define BACKLOG 10 //maximum length which queue of pending connections may grow

void str_ser(int sockfd); 			//transmitting and receiving function

int main(void)
{
	int sockfd, con_fd, ret;
	struct sockaddr_in my_addr; 	//server receives from this address
	struct sockaddr_in their_addr; 	//server responds to client at this address
	int sin_size;

	pid_t pid; //process ID

	//Create Socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd <0)
	{
		printf("Error in socket");
		exit(1);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYTCP_PORT); //port that the server receives from will have this TCP_PORT number
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); //specify a specific IP that the server receives from (All IPs accepted in this case)
	bzero(&(my_addr.sin_zero), 8);

	//Associate Socket to a port so that server knows which IP-Port pair to listen to 
	ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)); //(socket, sockaddr, addrlen)
	if (ret <0)
	{
		printf("Error in binding");
		exit(1);
	}
	
	//Check that Server able to listen
	ret = listen(sockfd, BACKLOG);
	if (ret <0) {
		printf("Error in listening");
		exit(1);
	}

	while (1)
	{
		printf("Waiting for data\n");
		sin_size = sizeof (struct sockaddr_in);
		con_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size); //accept the packet
		if (con_fd <0)
		{
			printf("Error in accepting data\n");
			exit(1);
		}

		if ((pid = fork())==0)			// create accept process
		{
			close(sockfd);
			str_ser(con_fd); 			//receive packet and response
			close(con_fd);
			exit(0);
		}
		else close(con_fd);				//parent process
	}
	close(sockfd);
	exit(0);
}

void str_ser(int sockfd)
{
	char buf[BUFSIZE];
	FILE *fp;
	char receive_buf[DATALEN]; //incoming data from client is stored here
	struct ack_so ack;
	int end_flag = 0; 
	int num_bytes_received = 0; //number of bytes server received from client in each recv() call
	int ack_bytes_sent = 0; 	//number of ACK bytes server sends to client in end send() call
	long buf_tracker = 0; 		//keeps track of position in buffer next packet from client should be stored
	int packet_count = 0;
	ack.num = 0;

	int du_received = 0;
	
	printf("Receiving data!\n\n");

	while(!end_flag)
	{
		if ((num_bytes_received = recv(sockfd, &receive_buf, DATALEN, 0))==-1)                                   //receive the packet
		{
			printf("Error when receiving packet\n");
			exit(1);
		} else {
			packet_count++;
			du_received++;
		}

		if (receive_buf[num_bytes_received-1] == '\0')		//checking for EOF
		{
			end_flag = 1;
			num_bytes_received --;	//means we don't copy the end byte over to buffer, although it was received
		}

		memcpy((buf+buf_tracker), receive_buf, num_bytes_received); 	//memory copy from (dest, source, no. bytes)
		buf_tracker += num_bytes_received;
		printf("Received packets: %d (%d bytes)\n", packet_count, num_bytes_received);
		
		if (du_received == NSIZE || end_flag == 1) {
			ack.num = 1;
			ack.len = 0;
			ack_bytes_sent = send(sockfd, &ack, 2, 0); 		//sends ACK; (socket, buffer, length, flags)
			if (ack_bytes_sent == -1) {
				printf("Send error!");		
				exit(1);
			}
			du_received = 0; //reset DU received size to 0
			printf("ACK sent\n");
			printf("Received %d bytes so far\n\n", (int) buf_tracker);
		}
	}
	if ((fp = fopen ("myTCPreceive.txt","wt")) == NULL)
	{
		printf("File doesn't exist\n");
		exit(0);
	}
	fwrite (buf , 1 , buf_tracker , fp);	//write data into file (ptr, size of each element, no. elements, output stream)
	fclose(fp);
	printf("A file has been successfully received!\nThe total data received is %d bytes\n", (int)buf_tracker);
}