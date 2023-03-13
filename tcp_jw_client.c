#include "headsock.h"

float str_cli(FILE *fp, int sockfd, long *transmit_size);                       //transmission function
void calc_transmission_interval(struct  timeval *out, struct timeval *in);	    //calculate the time interval between out and in

int main(int argc, char **argv) {
	int sockfd, ret;
	float transmit_time, transmit_rate;
	long transmit_size; 			//size of file to be transmitted
	struct sockaddr_in ser_addr; 	//client sends to this address
	char ** pptr;
	struct hostent *sh; 			//store information about host
	struct in_addr **addrs;
	FILE *fp;

	if (argc != 2) {
		printf("Parameters do not match");
	}

	sh = gethostbyname(argv[1]);	//get host's information
	if (sh == NULL) {
		printf("Error when gethostby name");
		exit(0);
	}

	//Print the remote host's information
	printf("Canonical name: %s\n", sh->h_name);
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++)
		printf("Host aliases name is: %s\n", *pptr);
	switch(sh->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("Unknown addrtype\n");
		break;
	}
        
	addrs = (struct in_addr **)sh->h_addr_list;

	//Create socket for client
	sockfd = socket(AF_INET, SOCK_STREAM, 0); //SOCK_STREAM for TCP
	if (sockfd <0)
	{
		printf("Error in socket");
		exit(1);
	}

	//Update parameters for destination address
	ser_addr.sin_family = AF_INET;                                                      
	ser_addr.sin_port = htons(MYTCP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8);

	//Connect the socket with the host
	ret = connect(sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr)); //(socket, addr, addrlen)
	if (ret != 0) {
		printf ("Connection failed\n"); 
		close(sockfd); 
		exit(1);
	}
	
	//Attempt to open local file to read
	if((fp = fopen ("myfile.txt","r+t")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}

	//Perform the transmission and receiving
	transmit_time = str_cli(fp, sockfd, &transmit_size); 
	transmit_rate = (transmit_size/(float)transmit_time);    //caculate the average transmission rate
	printf("Time(ms) : %.3f\nData Sent (bytes) with end byte: %d\nData Rate: %f (Kbytes/s)\n", transmit_time, (int)transmit_size, transmit_rate);

	close(sockfd);
	fclose(fp);
	exit(0);
}

//Handles client processing loop
float str_cli(FILE *fp, int sockfd, long *transmit_size) { //client: sends string

	struct ack_so ack; 				//packet structure for acknowledgement packet
	struct timeval sendt, recvt;
	char *buf; 						//buffer allocated for the entire file that client wishes to transmit
	long file_size; 				//file size that client wishes to transmit
	long sent_bytes_counter = 0; 	//bytes sent by client so far
	char packet_to_send[DATALEN]; 	//base packet used by client for transmission
	int num_bytes_sent; 			//number of bytes client sends to server in one send()
	int ack_bytes_received;			//number of ACK bytes received
	int send_packet_size; 			//size of packet client will use for transmission
	float transmission_time = 0.0;
	int du_transmitted  = 0; 		//initialise a counter for the number of DUs transmitted, when it reaches N, reset it
	int packet_count  = 0; 			//keep count of number of packets received
	int end_flag = 0; 				//this flag is just used to minus one byte at the end

	fseek (fp , 0 , SEEK_END);
	file_size = ftell (fp);
	rewind (fp);
	printf("The file length is %d bytes\n", (int)file_size);
	printf("The packet length is %d bytes\n\n",DATALEN);

	//Allocate memory to contain the whole file
	buf = (char *) malloc (file_size);
	if (buf == NULL) exit (2);

	//Load the whole the file into the buffer.
	fread (buf,1,file_size,fp);

	buf[file_size] ='\0';	//append end byte (null terminating character which signals to stop) (EOF)
	gettimeofday(&sendt, NULL); 	//get the current time
	while(sent_bytes_counter<= file_size)	//while entire file hasn't been transmited
	{
		//printf("Initial ACK num is %d, ACK length is %d\n", ack.num, ack.len);
		if ((file_size+1-sent_bytes_counter) <= DATALEN) {//if file size - sent bytes less than packet size, assign packet size to remaining bytes
			send_packet_size = file_size+1-sent_bytes_counter;
			end_flag = 1; //this flag is just used to minus one byte at the end
		} else {
			send_packet_size = DATALEN; //assign packet size as specifed in headsock
		}

		memcpy(packet_to_send, (buf+sent_bytes_counter), send_packet_size); 	//memory copy from source to dest
		num_bytes_sent = send(sockfd, &packet_to_send, send_packet_size, 0); 	//send function (socket, buffer, length, flags)

		if(num_bytes_sent == -1) {
			printf("Error in transmitting from Client to Server");
			exit(1); 
		} else {
			packet_count++;
			du_transmitted++;
			printf("Sent packets: %d (%d bytes)\n", packet_count, num_bytes_sent);
		}

		if (du_transmitted == NSIZE || end_flag == 1) {
			ack_bytes_received = recv(sockfd, &ack, 2, 0); //receive ACK
			if (ack_bytes_received == -1) {
				printf("Error when receiving ACK from server\n");
				exit(1);
			}
			if (ack.num == 1 && ack.len == 0) { //checks if ACK correctly received
				printf("ACK received\n");
				//printf("Number of bytes received (ACK) is %d\n", ack_bytes_received);
				sent_bytes_counter += send_packet_size;
				if (!end_flag) {
					printf("File bytes sent so far: %ld", (long)sent_bytes_counter);
				} else {
					printf("File bytes sent so far: %ld", (long)sent_bytes_counter-1);
				}
				printf("/");
				printf("%ld bytes\n\n", (long)file_size);
			} else {
				printf("Error in transmission of ACK\n");
				exit(1); 
			}
			du_transmitted = 0; //resetting DU size to 0
		} else { //DUs transmitted haven't reached N size
			sent_bytes_counter += send_packet_size; 
		}
		//printf("ACK num is %d, ACK length is %d\n", ack.num, ack.len);
	}

	gettimeofday(&recvt, NULL); //get current time
	*transmit_size= sent_bytes_counter; 
	calc_transmission_interval(&recvt, &sendt);
	transmission_time += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
	return(transmission_time);
}

void calc_transmission_interval(struct  timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) <0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}