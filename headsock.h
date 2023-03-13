#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>

#define NEWFILE (O_WRONLY|O_CREAT|O_TRUNC)
#define MYTCP_PORT 4950
#define MYUDP_PORT 5350
#define DATALEN 100     //DU size
#define BUFSIZE 60000
#define PACKLEN 1008    //not used
#define HEADLEN 8       //not used

#define NSIZE 5         //size of N

struct pack_so			        //data packet structure
{
    uint32_t num;				//the sequence number (4 bytes)
    uint32_t len;				//the packet length (4 bytes)
    char data[DATALEN];	        //the packet data
};

struct ack_so
{
    uint8_t num;                //1 byte
    uint8_t len;                //1 byte
};