#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>
#include <time.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "window.h"
#include "packet_interface.h"

#define BUF_LEN 528		// Header: 12; Payload: 512; CRC2: 4;
#define RTT_MAX 4100	// Latence: 2*2000ms; Security: 100ms
#define ALPHA 1.15		// Security ratio for minimal RTT
#define STDIN 0       // Standard input
#define RECEIVER 0    // Client id
#define SENDER 1      // Server id

#define DATA_CORRUPTED -1
#define DATA_TRUNCATED 0
#define DATA_OK 1
#define DATA_ABRUPT 2
#define DATA_GRACEFUL 3
#define WRITE_ERROR -2

// Useful variables
struct sockaddr_in6 realAddr;  // IPv6 version of addresses
int sfd;                        // Result of create_socket function
win_t* win;                     // Pointer to window
int port;                       // Port used
int curTime;                    // Current time

/* Get the binary IPv6 address from current name
 * @address: Name to solve
 * @rval: Storage of the input IPv6 address descriptor
 * Returns NULL
 * Error: returns a pointer to the error string descrpitor
 */
const char * real_address(const char *address, struct sockaddr_in6 *rval);

/* Creates a socket and initializes it
 * @src_addr: address bound to the socket if !NULL
 * @src_port: port on which the socket listens if >0
 * @dst_addr: address to which the sockets sends data if !NULL
 * @dst_port: port to which the socket is connected if >0
 * Returns a file descriptor representing the socket
 * Error: returns -1
 */
int create_socket(struct sockaddr_in6* source_addr, int src_port, struct sockaddr_in6* dest_addr, int dst_port);

/* Stops the caller until a message arrives on the socket
 * and connects the socket to the source address of the
 * received message
 * @sfd: file descriptor to the socket to connect
 * Returns 0 in case of success
 * Error: returns -1
 * @POST: function does not erase data and could be repeated
 */
int wait_for_client(int sfd);

/* Starts connection for client or server
 * @hostname: domain to be traduced
 * @port: port on/to which the socket bust be connnected
 * @addr: structure remembering the traduced address
 * @sorc: indicator of server (1) or client (0)
 * Returns 0 if the connection is established
 * Error: returns -1
 */
int create_connection(char* hostname, int port, struct sockaddr_in6* addr, int sorc);


/* Returns current time in ms
 */
uint32_t gettime();
