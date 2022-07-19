#include "receiver.h"

int main(int argc, char *argv[]){
  // Checking arguments
	if(argc < 3){
    fprintf(stderr,"[RECEIVER - MAIN] ERROR: too few arguments\n");
    return -1;
  }
	char *hostname;

	char c;
	if((c = getopt(argc, argv, "f:")) != -1){
		if(optarg == NULL){
			fprintf(stderr,"[RECEIVER - MAIN] ERROR: option format must be -f filename\n");
			return -1;
		}
		if(argc != 5){
	    fprintf(stderr,"[RECEIVER - MAIN] ERROR: bad number of arguments\n");
	    return -1;
	  }
		output = fopen(optarg, "w");
		hostname = argv[3];
    port = atoi(argv[4]);
	}
	else{
		if(argc != 3){
	    fprintf(stderr,"[RECEIVER - MAIN] ERROR: bad number of arguments\n");
	    return -1;
	  }
		output = stdout;
		hostname = argv[1];
    port = atoi(argv[2]);
	}

	sfd = create_connection(hostname, port, &realAddr, RECEIVER);
	if(sfd == -1)
		return -1;
	fprintf(stderr, "Waiting for data.\n\n");
	// Reception window
	win = win_new();
	if(win == NULL){
		fprintf(stderr,"[RECEIVER - MAIN] ERROR: failed to initialise the window\n");
		close(sfd);
		return -1;
	}

	read_write_loop(sfd);

	//fprintf(stderr, "%d packets received.\n", count);
	win_del(win);
	close(sfd);
	fclose(output);
	fprintf(stderr, "End of connection.\n\n");
	return 0;
}

void read_write_loop(int sfd){
	char* recBuf = (char*)malloc(BUF_LEN);
	int rd;
	int pollret;
	uint32_t waitTimer = gettime();
	struct pollfd spoll[] = {{sfd,POLLIN,0}};
	while(1){
		if((output == stdout && gettime()-waitTimer > (uint32_t)11000) || (output != stdout && gettime()-waitTimer > 2*(uint32_t)RTT_MAX)){
			fprintf(stderr, "[RECEIVER - READ_WRITE_LOOP] ERROR: running out of time\n");
			break;
		}
		pollret = poll(spoll,1,0);
		if(pollret == -1){
			fprintf(stderr, "[RECEIVER - READ_WRITE_LOOP] ERROR: error occured while waiting for data.\n");
			break;
		}
		if(spoll[0].revents & POLLIN){
			rd = read(sfd, recBuf, BUF_LEN);
			if(rd == -1){
				fprintf(stderr, "[RECEIVER - READ_WRITE_LOOP] ERROR: impossible to read on socket.\n");
				break;
			}
			int rdata = receive_data(recBuf,rd,sfd);
			if(rdata == DATA_ABRUPT){
				fprintf(stderr, "Transfer succeeded.\n");
				break;
			}
			waitTimer = gettime();
		}
	}
	free(recBuf);
}

// Receive some data
int receive_data(char* data, size_t len, int sfd){
	pkt_t *pkt = pkt_new();
	int wr = 0;
	if (pkt_decode(data, len, pkt) == PKT_OK){
		if(pkt_get_type(pkt) != PTYPE_DATA){
			pkt_del(pkt);
			return DATA_CORRUPTED;
		}
		// Remembers which sequence is expected
		uint32_t expected = (pkt_get_seqnum(pkt)) - (pkt_get_seqnum(pkt) - (win->last))%32 + 1;
		if (pkt_get_tr(pkt)){
			pkt_del(pkt);
			if(response(1, expected, sfd) < 0)
				return DATA_CORRUPTED;
			return DATA_TRUNCATED;
		}
		if (win->size == 0){
			response(1, expected, sfd);
			pkt_del(pkt);
			return WIN_FULL;
		}

		uint8_t seq = pkt_get_seqnum(pkt);

		int res = win_receive(win,seq);
		if (res != WIN_IGNORE)
			buf[seq%32] = pkt;

		uint32_t i;
		for (i=res; i>0; i--){
			pkt_t *toWrite = buf[(seq%32)-i+1];
			//fprintf(stderr, "A ecrire: %s\n", pkt_get_payload(toWrite));
			if(pkt_get_length(toWrite) == 0){
				response(0,expected,sfd);
				pkt_del(pkt);
				return DATA_ABRUPT;
			}
			wr = fwrite(pkt_get_payload(toWrite), pkt_get_length(toWrite), 1, output);
			if(wr != 1){
				fprintf(stderr, "[RECEIVER - RECEIVE_DATA] ERROR: failed to write data.\n");
				return WRITE_ERROR;
			}
			//fprintf(stderr, "%d bytes written\n",pkt_get_length(toWrite));
			//count++;
			expected = (pkt_get_seqnum(pkt));
			pkt_del(toWrite);
		}
		expected++;
		if(response(0, expected, sfd)){
			return DATA_OK;
		}
	}
	return DATA_CORRUPTED;
}

// Send acknowledgements
int response(uint8_t tr, uint32_t seqnum, int sfd){
	 pkt_t *res = pkt_new();
	 if (tr) {
	 	pkt_set_type(res, PTYPE_NACK);
	 }
	 else{
		 pkt_set_type(res, PTYPE_ACK);
	 }
	 pkt_set_window(res, win->size);
	 pkt_set_seqnum(res, seqnum);
	 pkt_set_timestamp(res, 0); //TODO
	 char data[HEADER_SIZE];
	 size_t length = HEADER_SIZE;
	 pkt_encode(res, data, &length);
	 if(write(sfd,data,length) == -1){
		 fprintf(stderr, "[RECEIVER - RESPONSE] ERROR: impossible to send acknowledgement.\n");
		 pkt_del(res);
		 return -1;
	 }
	 pkt_del(res);
	 return 1;
}
