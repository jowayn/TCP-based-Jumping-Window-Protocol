# TCP-based-Jumping-Window-Protocol
A TCP-based client-server socket program for transferring large messages using a jumping window protocol. Done as part of an assignment in the NUS module EE4204 Computer Networking. 

The message is split into short data-units (DUs) which are sent and acknowledged in batches of size N (can be tuned in headsock.c) The sender sends N DUs and then waits for an ACK before sending the next batch of N DUs. It repeats the above procedure until the entire file is sent and the acknowledgement for the last batch is received. 

The receiver sends an ACK after receiving N DUs. It repeats the above procedure, until the acknowledgement for the last batch is sent. 

*Note: The last batch may have less than N DUs.

## Usage 

### Compile both programs (linux-based terminal)

 ```
 gcc tcp_jw_ser.c
 gcc tcp_jw_client.c
 ```

### Run Server side

```
./tcp_jw_ser
```

### Run Client side

```
./tcp_jw_client <IP Address of Server>
```
### If running client on same machine

```
./tcp_jw_client 127.0.0.1
```

## Features
The following are kept track of on the client side:
- Sent Packets
- ACK notifications
- Time
- Total Data Sent
- Data Rate

![](/sample.png) 

## Notes
By Default, sends the file myfile.txt
- This can be changed in tcp_jw_client.c line 65
       
By Default DU size is set to 100bytes and N size is set to 5
- This can be changed in headsock.h lines 18 and 23 respectively
