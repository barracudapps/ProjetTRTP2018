#include "test.h"

int main(int argc, char *argv[])
{
  fclose(stdin);

  //definit ce que les tests affiche.
  char c;
  if ((c=getopt(argc, argv, "v:"))!= -1){
    if(c == 'v'){
      verbose = atoi(optarg);
      if(verbose < 0 || verbose > 2){
        fprintf(stderr,"ERROR : verbose level must be between 0 and 2\n\n");
        fclose(stdout);
        fclose(stderr);
        return -1;
      }
    }
    else{
      fprintf(stderr,"hint: option allowed: -v level\n\n");
      fclose(stdout);
      fclose(stderr);
      return -1;
    }
  }

  //lancement des tests de chaque fonctionnalite.
  fprintf(stderr, "\nStarting tests\n\n");
  test_pkt();
  test_window();
  test_network();

  fprintf(stderr, "\nTests completed : ");
  if(passed==total)
    fprintf(stderr, "All tests passed.(%d tests)\n",total);
  else
    fprintf(stderr, "%d tests passed over %d.\n", passed, total);

  if(verbose == 0)
    fprintf(stderr, "For more details, use -v [1-2]\n");
  fprintf(stderr, "\n");

  fclose(stdout);
  fclose(stderr);
  return 0;
}

//Code inspire de https://www.youtube.com/watch?v=1Ca5iRsta2Q
void test(int expected, int actual, const char* name){
  total++;
  if(expected == actual){
    passed++;
    if(verbose){
      fprintf(stderr,"PASSED %s\n", name);
      if(verbose == 2)
        printf("\treturned value: %d\n",expected);
    }
  }else{
    fprintf(stderr,"FAILED %s\n\texpected: %d actual: %d\n", name, expected, actual);
  }
}

void test_unexpected(int unexpected, int actual, const char* name){
  total++;
  if(unexpected != actual){
    passed++;
    if(verbose){
      fprintf(stderr,"PASSED %s\n", name);
      if(verbose == 2)
        printf("\treturned value: %d\n",unexpected);
    }
  }else{
    fprintf(stderr,"FAILED %s\n\tunexpected: %d actual: %d\n", name, unexpected, actual);
  }
}

void test_ptr(void* expected, void* actual, const char* name){
  total++;
  if(expected == actual){
    passed++;
    if(verbose){
      fprintf(stderr,"PASSED %s\n", name);
      if(verbose == 2)
        printf("\treturned value: %p\n",expected);
    }
  }else{
    fprintf(stderr,"FAILED %s\n\texpected: %p actual: %p\n", name, expected, actual);
  }
}

void test_unexpected_ptr(void* unexpected, void* actual, const char* name){
  total++;
  if(unexpected != actual){
    passed++;
    if(verbose){
      fprintf(stderr,"PASSED %s\n", name);
      if(verbose == 2)
        printf("\treturned value: %p\n",unexpected);
    }
  }else{
    fprintf(stderr,"FAILED %s\n\tunexpected: %p actual: %p\n", name, unexpected, actual);
  }
}



void test_pkt()
{
  pkt_t* pkt = pkt_new();
  test_unexpected_ptr(NULL, (void*)pkt, "Pkt: New");

  /* GETTERS-SETTERS*/

  // type
  test(PKT_OK, pkt_set_type(pkt, PTYPE_ACK), "Pkt: set type");
  test(PTYPE_ACK, pkt_get_type(pkt), "Pkt: get type");
  test(E_TYPE, pkt_set_type(pkt, 0), "Pkt: set unconsistent type");
  test_unexpected(0, pkt_get_type(pkt), "Pkt: get unconsistent type");

  // tr
  test(E_UNCONSISTENT, pkt_set_tr(pkt, 1), "Pkt: set tr wrong type");
  pkt_set_type(pkt, PTYPE_DATA);
  test(PKT_OK, pkt_set_tr(pkt, 1), "Pkt: set tr");
  test(1, pkt_get_tr(pkt), "Pkt: get tr");
  test(E_TR, pkt_set_tr(pkt, 10), "Pkt: set unconsistent tr");
  test_unexpected(10, pkt_get_tr(pkt), "Pkt: get unconsistent tr");

  // window
  test(PKT_OK, pkt_set_window(pkt, 19), "Pkt: set window");
  test(19, pkt_get_window(pkt), "Pkt: get window");
  test(E_WINDOW, pkt_set_window(pkt, 75), "Pkt: set unconsistent window");
  test_unexpected(75, pkt_get_window(pkt), "Pkt: get unconsistent window");

  // seqnum
  test(PKT_OK, pkt_set_seqnum(pkt, 123), "Pkt: set seqnum");
  test(123, pkt_get_seqnum(pkt), "Pkt: get seqnum");

  // length
  test(PKT_OK, pkt_set_length(pkt, 500), "Pkt: set length");
  test(500, pkt_get_length(pkt), "Pkt: get length");
  test(E_LENGTH, pkt_set_length(pkt, 1000), "Pkt: set unconsistent length");
  test_unexpected(1000, pkt_get_length(pkt), "Pkt: get unconsistent length");

  // timestamp
  test(PKT_OK, pkt_set_timestamp(pkt, 0x1A7B4C56), "Pkt: set timestamp");
  test(0x1A7B4C56, pkt_get_timestamp(pkt), "Pkt: get timestamp");

  // crc1
  test(PKT_OK, pkt_set_crc1(pkt, 169867561), "Pkt: set crc1");
  test(169867561, pkt_get_crc1(pkt), "Pkt: get crc1");

  // payload
  char pload[520] = "Hello World!";
  test(PKT_OK, pkt_set_payload(pkt, pload, 13), "Pkt: set payload");
  test(13, pkt_get_length(pkt), "Pkt: setting payload changes length");
  test(0, memcmp(pkt_get_payload(pkt), pload, 13), "Pkt: get payload");
  test(E_LENGTH, pkt_set_payload(pkt, pload, 520), "Pkt: set too long payload");
  pkt_set_payload(pkt, NULL, 0);
  test_ptr(NULL, (void*)pkt_get_payload(pkt), "Pkt: set payload to NULL");
  test(0, pkt_get_length(pkt), "Pkt: setting payload to NUll changes length to 0");

  // crc2
  test(PKT_OK, pkt_set_crc2(pkt, 68432168), "Pkt: set crc2");
  test(68432168, pkt_get_crc2(pkt), "Pkt: get crc2");



  /* ENCODE-DECODE*/

  size_t buf_len = HEADER_SIZE;
  char* buf = calloc(buf_len, sizeof(char));
  size_t written = 5;
  test(E_NOMEM, pkt_encode(pkt, buf, &written), "Encode: buffer too short : 5B");
  test(0, written, "Encode: Bytes written - too short");

  // ack - encode
                 // window = 0b10011     |
  pkt_set_tr(pkt, 0); //0b0              | => 0b10010011 = 0x93 = 147
  pkt_set_type(pkt, PTYPE_ACK); //0b10   |

  written = buf_len;
  test_unexpected(E_NOMEM, pkt_encode(pkt, buf, &written), "Encode: Ack: buffer long enough : 14B");
  test(12, written, "Encode: Ack: Bytes written");
  test(0x93, (uint8_t)*buf, "Encode: Ack: first byte");
  test(123, (uint8_t)*(buf+1), "Encode: Ack: seqnum");
  test(0, *(uint16_t*)(buf+2), "Encode: Ack: length = 0");
  uint32_t crc1 = crc32(0L, Z_NULL, 0);
  crc1 = htonl(crc32(crc1, (Bytef*)buf, 8));
  test((uint32_t)0x1A7B4C56, *(uint32_t*)(buf+4), "Encode: Ack: timestamp");
  test(crc1, *(uint32_t*)(buf+8), "Encode: Ack: crc1");


  // ack - decode
  pkt_t* decoded = pkt_new();
  test(PKT_OK, pkt_decode(buf, written, decoded), "Decode: Ack: functions completion");
  test(pkt_get_window(pkt), pkt_get_window(decoded), "Decode: Ack: window");
  test(pkt_get_tr(pkt), pkt_get_tr(decoded), "Decode: Ack: tr");
  test(pkt_get_type(pkt), pkt_get_type(decoded), "Decode: Ack: type");
  test(pkt_get_seqnum(pkt), pkt_get_seqnum(decoded), "Decode: Ack: seqnum");
  test(pkt_get_length(pkt), pkt_get_length(decoded), "Decode: Ack: length");
  test(pkt_get_timestamp(pkt), pkt_get_timestamp(decoded), "Decode: Ack: timestamp");
  test(crc1, pkt_get_crc1(decoded), "Decode: Ack: crc1");
  test_ptr(NULL, (void*)pkt_get_payload(decoded), "Decode: Ack: payload = NULL");
  test(0, pkt_get_crc2(decoded), "Decode: Ack: crc2 = 0");
  pkt_del(decoded);

  // data - encode
  pkt_set_type(pkt, PTYPE_DATA);// first byte => 0b01010011 = 0x53 = 83
  //TODO c'est sur le buffer, petit con

  char* payload = "Sample test data";
  pkt_set_payload(pkt, payload, 17); // host-byte order => 0x0011

  written = buf_len;
  test(E_NOMEM, pkt_encode(pkt, buf, &written), "Encode: Data: buffer too short : 14B");
  test(0, written, "Encode: Data: Bytes written - too short");

  free(buf);
  written = buf_len = HEADER_SIZE + pkt_get_length(pkt) + 4;
  buf = calloc(buf_len, sizeof(char));
  test_unexpected(E_NOMEM, pkt_encode(pkt, buf, &written), "Encode: Data: buffer long enough : 34B");
  test(buf_len, written, "Encode: Data: Bytes written");

  test(0x53, *buf, "Encode: Data: first byte");
  test(123, *(buf+1), "Encode: Data: seqnum");
  test(0x1100, *(((uint16_t*)buf)+1), "Encode: Data: length");
  test(0x1A7B4C56,*(((uint32_t*)buf)+1), "Encode: Data: timestamp");
  crc1 = crc32(0L, Z_NULL, 0);
  crc1 = htonl(crc32(crc1, (Bytef*)buf, 8));
  test(crc1,*(((uint32_t*)buf)+2), "Encode: Data: crc1");
  test(0, memcmp(payload,(buf+12),17), "Encode: Data: payload");
  uint32_t crc2 = crc32(0L, Z_NULL, 0);
  crc2 = htonl(crc32(crc2, (Bytef*)pkt_get_payload(pkt), pkt_get_length(pkt)));
  test(crc2, *(uint32_t*)(buf+29), "Encode: Data: crc2");

  // data - decode
  decoded = pkt_new();
  test(PKT_OK, pkt_decode(buf, written, decoded), "Decode: Data: functions completion");
  test(pkt_get_window(pkt), pkt_get_window(decoded), "Decode: Data: window");
  test(pkt_get_tr(pkt), pkt_get_tr(decoded), "Decode: Data: tr");
  test(pkt_get_type(pkt), pkt_get_type(decoded), "Decode: Data: type");
  test(pkt_get_seqnum(pkt), pkt_get_seqnum(decoded), "Decode: Data: seqnum");
  test(pkt_get_length(pkt), pkt_get_length(decoded), "Decode: Data: length");
  test(pkt_get_timestamp(pkt), pkt_get_timestamp(decoded), "Decode: Data: timestamp");
  test(crc1, pkt_get_crc1(decoded), "Decode: Data: crc1");
  test(0,memcmp(pkt_get_payload(pkt), pkt_get_payload(decoded),17), "Decode: Data: payload");
  test(crc2, pkt_get_crc2(decoded), "Decode: Data: crc2");
  pkt_del(decoded);

  // truncated data - encode
  pkt_set_tr(pkt, 1);// first byte => 0b01110011 = 0x73 = 115
  free(buf);
  written = buf_len = HEADER_SIZE;
  buf = calloc(buf_len, sizeof(char));
  test_unexpected(E_NOMEM, pkt_encode(pkt, buf, &written), "Encode: TR: buffer long enough : 13B");
  test(buf_len, written, "Encode: TR: Bytes written");
  test(0x73, *buf, "Encode: TR: first byte");
  test(0x1100, *(uint16_t*)(buf+2), "Encode: TR: length");
  test(crc1, *(uint32_t*)(buf+8), "Encode: TR: crc1");

  // truncated data - decode
  decoded = pkt_new();
  test(PKT_OK, pkt_decode(buf, written, decoded), "Decode: TR: functions completion");
  test(pkt_get_window(pkt), pkt_get_window(decoded), "Decode: TR: window");
  test(1, pkt_get_tr(decoded), "Decode: TR: tr");
  test(pkt_get_type(pkt), pkt_get_type(decoded), "Decode: TR: type");
  test(pkt_get_seqnum(pkt), pkt_get_seqnum(decoded), "Decode: TR: seqnum");
  test(pkt_get_length(pkt), pkt_get_length(decoded), "Decode: TR: length");
  test(pkt_get_timestamp(pkt), pkt_get_timestamp(decoded), "Decode: TR: timestamp");
  test(crc1, pkt_get_crc1(decoded), "Decode: TR: crc1");
  test_ptr(NULL, (void*)pkt_get_payload(decoded), "Decode: TR: payload = NULL");
  test(0, pkt_get_crc2(decoded), "Decode: TR: crc2 = 0");
  pkt_del(decoded);

  free(buf);

  pkt_del(pkt);
}


void test_window(){
  // Creating the test window
  win_t* win = win_new();
  test_unexpected_ptr(NULL, (void*)win, "Window: new");
  if(win == NULL)
    return;

  // Simple case
  test(1, win_receive(win, 0), "Window: first in-sequence item");
  test(WIN_IGNORE, win_receive(win, 0), "Window: last received item");
  test(1, win_receive(win, 1), "Window: second in-sequence item");
  test(WIN_NOT_IN_SEQUENCE, win_receive(win, 3), "Window: first not-in-sequence item");
  test(WIN_NOT_IN_SEQUENCE, win_receive(win, 12), "Window: second not-in-sequence item");
  test(2, win_receive(win, 2), "Window: in-sequence item, followed by one not-in-sequence item");

  // More complex
  win_receive(win, 5);
  win_receive(win, 6);
  test(WIN_NOT_IN_SEQUENCE, win_receive(win, 7), "Window: many not-in-sequence items in a row");
  test(4, win_receive(win, 4), "Window: in-sequence item, followed by many not-in-sequence items");

  // A few unacceptable items
  test(WIN_IGNORE, win_receive(win, 6), "Window: already received item");
  test(WIN_IGNORE, win_receive(win, 12), "Window: already received not-in-sequence item");
  test(WIN_IGNORE, win_receive(win, 114), "Window: unacceptable: seq not in current window");
  test(WIN_IGNORE, win_receive(win, 39), "Window: unacceptable: seq not in current window (limit case)");

  // Exceeding 31
  test(WIN_NOT_IN_SEQUENCE, win_receive(win, 34), "Window: not-in-sequence, seq > 31");
  test(WIN_NOT_IN_SEQUENCE, win_receive(win, 38), "Window: not-in-sequence, seq > 31 (limit case)");

  // Seq reaching its maximum and falling back to 0
  int i;
  for(i=8; i<254; i++){
    win_receive(win, i);
  }
  test(WIN_NOT_IN_SEQUENCE, win_receive(win, 255), "Window: not-in-sequence, seq = 255");
  test(WIN_NOT_IN_SEQUENCE, win_receive(win, 0), "Window: not-in-sequence, seq back to 0");
  test(WIN_NOT_IN_SEQUENCE, win_receive(win, 3), "Window: not-in-sequence, seq back to 3");
  test(3, win_receive(win, 254), "Window: in-sequence, seq reaching 255");
  test(1, win_receive(win, 1), "Window: in-sequence, seq back to 0");

  win_del(win);
}

void test_network(){
  struct sockaddr_in6 rval;
  const char* domain = "::";
  const char* eResult = real_address(domain, &rval);
  int returnVal = 0;
  if(eResult == NULL)
    returnVal = 1;
  test(1, returnVal, "Lib: getting address");
  int sfd = create_socket( NULL, -1, &rval, 8888);
  if(sfd == -1)
    returnVal = 0;
  else
    returnVal = 1;
  close(sfd);
  test(1, returnVal, "Lib: creating client socket");
  sfd = create_socket(&rval, 8890, NULL, -1);
  if(sfd == -1)
    returnVal = 0;
  else
    returnVal = 1;
  close(sfd);
  test(1, returnVal, "Lib: creating server socket");
}
