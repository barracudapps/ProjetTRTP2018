#include "network.h"


const char * real_address(const char *address, struct sockaddr_in6 *rval){
    struct addrinfo *result = NULL;
    struct addrinfo hints ={0};
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    char* error = NULL;
    if(address != NULL){
        if(getaddrinfo(address, NULL, &hints, &result) == 0){
            if(result->ai_family == AF_INET6){
                *rval = *(struct sockaddr_in6*)(result->ai_addr);
                if(rval == NULL)
                  error = "[NETWORK - REAL_ADDRESS] ERROR: null return value.\n";
            }else
              error = "[NETWORK - REAL_ADDRESS] ERROR: incorrect ai_family.\n";
        }else
          error = "[NETWORK - REAL_ADDRESS] ERROR: problem executing getaddrinfo.\n";
    }else{
        error = "[NETWORK - REAL_ADDRESS] ERROR: null address.\n";
    }
    freeaddrinfo(result);
    return error;
}

int create_socket(struct sockaddr_in6 *source_addr, int src_port, struct sockaddr_in6 *dest_addr, int dst_port){
    struct sockaddr_in6 newAdd;
    int sfd = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP);
    socklen_t len = sizeof(struct sockaddr_in6);
    if(sfd != -1 || (source_addr->sin6_family != AF_INET6 && dest_addr->sin6_family != AF_INET6)){
        if(source_addr != NULL){
            memcpy(&newAdd, source_addr, len);
            if(src_port > 0){
                newAdd.sin6_port = htons(src_port);
                if(bind(sfd,(struct sockaddr *)&newAdd,len) == 0){
                    fprintf(stderr, "Bound.\n");
                    return sfd;
                }
            }
        }
        else if(dest_addr != NULL){
            memcpy(&newAdd, dest_addr, len);
            if(dst_port > 0){
                newAdd.sin6_port = htons(dst_port);
                if(connect(sfd,(struct sockaddr *)&newAdd,len) != -1){
                    fprintf(stderr, "Connected.\n");
                    return sfd;
                }
            }
        }
    }
    return -1;
}

int wait_for_client(int sfd){
    int rec = -1;
    char buf[200];
    struct sockaddr_in6 addr;
    socklen_t len = sizeof(addr);
    rec = recvfrom(sfd, buf, 200, (MSG_PEEK), (struct sockaddr*)&addr, &len);
    if(rec == -1){
      fprintf(stderr,"[NETWORK - WAIT_FOR_CLIENT] ERROR: problem with recvfrom.\n");
      return -1;
    }
    if(connect(sfd,(struct sockaddr *)&addr, len) == -1){
      fprintf(stderr, "Connected.\n");
      return -1;
    }
    return 0;
}

int create_connection(char* hostname, int port, struct sockaddr_in6* addr, int sorc){
	// Recovering IPv6 address
	if(real_address(hostname, addr)!= NULL){
		fprintf(stderr, "[NETWORK - CREATE_CONNECTION] ERROR: failed to recover IPv6 address\n");
		return -1;
	}
  if(sorc == RECEIVER){
    // Creating socket as server
    sfd = create_socket(&realAddr, port, NULL, -1);
    if(sfd == -1){
      fprintf(stderr, "[NETWORK - CREATE_CONNECTION] ERROR: failed to create socket\n");
      return -1;
    }
    // Waiting for client
    if(wait_for_client(sfd) == -1){
      close(sfd);
      fprintf(stderr, "[NETWORK - CREATE_CONNECTION] ERROR: no message received from client\n");
      return -1;
    }
    return sfd;
  }
  if(sorc == SENDER){
    // Creating socket as client
    sfd = create_socket(NULL, -1, &realAddr, port);
    if(sfd == -1){
      fprintf(stderr, "[NETWORK - CREATE_CONNECTION] ERROR: failed to create socket\n");
      return -1;
    }
    return sfd;
  }
  return -1;
}

uint32_t gettime(){
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC, &tp);
  return (uint32_t)(tp.tv_sec * 1000 + tp.tv_nsec / 1000000);
}
