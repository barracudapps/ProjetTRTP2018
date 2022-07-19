#include <netinet/in.h>
#include <sys/select.h>


#include "network.h"

#define BUF_SIZE 1024
#define STDIN 0
#define STDOUT 1

int input;
int connected = 1;
int finished = 0;
pkt_t* buf[WINDOW_SIZE];
uint8_t seqnum = 0;
uint16_t end = 256;
uint16_t old = 255;

int read_input(pkt_t *pkt);
int send_data(pkt_t *pkt, int sfd);
int receive_data(const char* data, size_t len, pkt_t* pkt);
void read_write_loop(int sfd);
