#include <sys/types.h>

#include "network.h"

pkt_t *buf[WINDOW_SIZE];
FILE *output;
//int count = 0;


/*
 * receives and decodes data. Then if truncated, sends a NACK packet. If not,
 * adds it to the reception buffer, if it is in sequence, writes all
 * in-sequence packets and finally sends an ACK packet.
 * @return DATA_CORRUPTED, DATA_TRUNCATED, DATA_OK.
 */
int receive_data(char* data, size_t len, int sfd);

/*
 * send an appropriate respond for the reception of a packet
 */
 int response(uint8_t tr, uint32_t seqnum, int sfd);

/* Waits for some data on the socket and writes it in a file or on stdout
 * @sfd: socket descriptor
 */
void read_write_loop(int sfd);
