# TCP-based-Jumping-Window-Protocol
A TCP-based client-server socket program for transferring large messages using a jumping window protocol. The message is split into short data-units (DUs) which are sent and acknowledged in batches of size N (can be tuned in headsock.c) The sender sends N DUs and then waits for an ACK before sending the next batch of N DUs. It repeats the above procedure until the entire file is sent and the acknowledgement for the last batch is received. The receiver sends an ACK after receiving N DUs. It repeats the above procedure, until the acknowledgement for the last batch is sent. 
*Note: The last batch may have less than N DUs.

## Usage 

### Compile both programs (linux-based terminal)

 ```
 gcc tcp_jw_ser.c
 gcc tcp_jw_client.c
 ```

#### Run Server side

```
./tcp_jw_ser
```

### Run Client side

```
./tcp_jw_client <IP Address of Client>
```
### If running client on same machine

```
./tcp_jw_client 127.0.0.1
