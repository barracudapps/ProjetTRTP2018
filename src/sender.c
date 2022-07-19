#include "sender.h"

int main(int argc, char** argv){
  // Checking arguments
  if(argc < 3){
    fprintf(stderr,"ERROR : too few arguments.\n");
    return -1;
  }
  char *hostname;

  char c;
  if((c = getopt(argc, argv, "f:")) != -1){
    if(optarg == NULL){
      fprintf(stderr,"ERROR : option format must be -f filename.\n");
      return -1;
    }
    if(argc != 5){
      fprintf(stderr,"ERROR : bad number of arguments.\n");
      return -1;
    }
    input = open(optarg, O_RDONLY);
    hostname = argv[3];
    port = atoi(argv[4]);
  }
  else{
    if(argc != 3){
      fprintf(stderr,"ERROR : bad number of arguments.\n");
      return -1;
    }
    input = STDIN;
    hostname = argv[1];
    port = atoi(argv[2]);
  }

  // Initiating connection
  sfd = create_connection(hostname, port, &realAddr, SENDER);
  if(sfd == -1){
    fprintf(stderr, "ERROR: connection problem.\n");
    return -1;
  }
  fprintf(stderr, "Sending data.\n\n");
  // Send window
  win = win_new();
  if(win == NULL){
    fprintf(stderr, "ERROR: failed to initialize window.\n");
    close(sfd);
    return -1;
  }
  win->size = 1;

  read_write_loop(sfd);

  fprintf(stderr, "\nEnd of connection.\n");
  // Clear memory
  win_del(win);
  close(input);
  close(sfd);
  return 0;
}

//RW loop
void read_write_loop(int sfd){
  /* Code inspiré de https://broux.developpez.com/articles/c/sockets/#L4-1*/
  char data[BUF_LEN];
  int rd;
  int wr;
  uint32_t waitTimer = gettime();
  int pollret;
  struct pollfd spoll[] = {{input,POLLIN,0},{sfd,POLLIN,0},{sfd,POLLOUT,0}};
  pkt_t *pkt;
  while(1){
    if((input == STDIN && gettime()-waitTimer > (uint32_t)10000) || (input != STDIN && gettime()-waitTimer > 10000*(uint32_t)RTT_MAX)){
			fprintf(stderr, "[SENDER - READ_WRITE_LOOP] ERROR: running out of time\n");
			break;
		}
    pollret = poll(spoll,3,1);
    if(pollret == -1)
      break;

    // Send data
    if(win->size > 0){
      pkt = buf[(win->last)+1];//%32?
      // Reading on input
      if (pkt == NULL){
        if(finished == 0){
          if(spoll[0].revents & POLLIN){
            pkt = pkt_new();
            rd = read_input(pkt);
            if(rd < 0)
              break;
            else if(rd == 0){
              end = pkt_get_seqnum(pkt);
              finished = 1;
            }
            if((spoll[2].revents & POLLOUT) && win->size > 0)//TODO
              wr = send_data(pkt, sfd);
          }
        }
      }
      // send the next packet if timeout
      else if(gettime() - pkt_get_timestamp(pkt) > RTT_MAX){
        if(spoll[2].revents & POLLOUT)
          wr = send_data(pkt, sfd);
      }
      if(wr < 0)
        break;
    }

    // ACK/NACK reception
    if(spoll[1].revents & POLLIN){
      rd = read(sfd, data, HEADER_SIZE);
      if(rd < 0)
        break;
      fprintf(stderr,"[ACK/NACK] received.\n");
      wr = receive_data(data, HEADER_SIZE, pkt);
      if(wr == DATA_TRUNCATED && seqnum - pkt_get_seqnum(pkt) < WINDOW_SIZE && win->size > 0){
        if(spoll[2].revents & POLLOUT)
          wr = send_data(pkt, sfd);
        if(wr < 0)
          break;
      }
      if(wr == DATA_GRACEFUL){
        fprintf(stderr, "\nTransfer succeeded.\n");
        break;
      }
      waitTimer = gettime();
    }
  }
}

int read_input(pkt_t *pkt){
  char payload[MAX_PAYLOAD_SIZE];
  int rd = read(input, payload, MAX_PAYLOAD_SIZE);
  pkt_set_window(pkt, 31);
  pkt_set_type(pkt, PTYPE_DATA);
  pkt_set_seqnum(pkt, seqnum);//TODO essayer seqnum++ ici
  seqnum++;
  if(rd > 0)
    pkt_set_payload(pkt, payload, rd);
  return rd;
}

int send_data(pkt_t *pkt, int sfd){
  if(pkt == NULL)
    return 0;
  size_t len = BUF_LEN;
  char data[len];
  pkt_set_timestamp(pkt, gettime());
  if(pkt_encode(pkt, data, &len) != PKT_OK)
    return -1;
  int wr = write(sfd,data,len);
  if(wr == -1)
    return -1;
  usleep(1);//TODO

  fprintf(stderr,"%d bytes sent.\n",wr);
  win->size--;
  if(win->last == 31){
    win->last = 0;
    win->crossed ++;
  }else{
    win->last++;
  }
  buf[win->last] = pkt;
  return wr;
}

int receive_data(const char* data, size_t len, pkt_t* pkt){
  pkt = pkt_new();
	if (pkt_decode(data, len, pkt) != PKT_OK){
    pkt_del(pkt);
    pkt = NULL;
    return DATA_CORRUPTED;
  }
  ptypes_t type = pkt_get_type(pkt);
  if(type == PTYPE_ACK){
    uint16_t new = win_get_seqnum(win);
    while(old != new){
      if(old == 255){
        old = 0;
      }else{
        old++;
      }
      pkt_del(buf[old%32]);
      buf[old%32] = NULL;
    }
    if(old == end){
      pkt_del(pkt);
      pkt = NULL;
      return DATA_GRACEFUL;
    }
    win->size = pkt_get_window(pkt);
  }
  pkt_del(pkt);
  pkt = NULL;
  if(type == PTYPE_NACK){
    return DATA_TRUNCATED;
  }
  if(type == PTYPE_DATA){
    return DATA_ABRUPT;
  }
  return DATA_OK;
}
