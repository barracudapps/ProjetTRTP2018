#include "packet_interface.h"

struct __attribute__((__packed__)) pkt {
  // All attributes are stored in host-byte-order
  uint8_t window : 5;
  uint8_t tr : 1;
  ptypes_t type : 2;
  uint8_t seqnum;
  uint16_t length;
  uint32_t timestamp;
  uint32_t crc1;
  char* payload;
  uint32_t crc2;
};

pkt_t* pkt_new()
{
  return (pkt_t*)calloc(1,sizeof(pkt_t));
}

void pkt_del(pkt_t *pkt)
{
  if(pkt!=NULL){
    if(pkt->payload!=NULL)
      free(pkt->payload);
    free(pkt);
    pkt = NULL;
  }
}

pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt)
{
  if(len < HEADER_SIZE)
    return E_NOHEADER;

  // Header
  memcpy(pkt, data, 2);

  uint16_t length;
  memcpy(&length, data + 2, 2);
  pkt->length = length = ntohs(length);

  memcpy(&(pkt->timestamp), data + 4, 8);

  // CRC1
  char header[8];
  header[0] = (data[0] & 0b11011111);
  memcpy(header + 1, data + 1, 7);
  if (crc32(0, (Bytef*)header, 8) != ntohl(pkt->crc1))
    return E_CRC;

  // Coherence du header
  if(length > MAX_PAYLOAD_SIZE)
    return E_LENGTH;
  if(pkt->tr == 1 && pkt->type != PTYPE_DATA)
   return E_TR;

  // Payload
  if(!pkt->tr && length > 0){
    if(len != (size_t)(HEADER_SIZE + length + 4))
      return E_LENGTH;

    char* payload = (char*)malloc(length);
    if(payload == NULL)
      return E_NOMEM;
    memcpy(payload, data + HEADER_SIZE, length);

  // CRC2
    memcpy(&(pkt->crc2), data + HEADER_SIZE + length, 4);
    if (crc32(0, (Bytef*)payload, length) != ntohl(pkt->crc2)){
      free(payload);
      return E_CRC;
    }
    pkt->payload = payload;
  }
  return PKT_OK;
}

pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
{
  int pcrc = (pkt->tr == 0) && (pkt->length != 0);
  if(*len < (size_t)(HEADER_SIZE + pcrc * (pkt->length + 4)) || buf == NULL){
    *len = 0;
    return E_NOMEM;
  }

  // Header
  memcpy(buf, pkt, 2);

  uint16_t length = htons(pkt->length);
  memcpy(buf + 2, &length, 2);
  length = pkt->length;

  memcpy(buf + 4, &(pkt->timestamp), 4);

  // CRC1
  char header[8];
  header[0] = (buf[0] & 0b11011111);
  memcpy(header + 1, buf + 1, 7);
  uint32_t crc = htonl(crc32(0, (Bytef*)header, 8));
  memcpy(buf + 8, &(crc), 4);

  // Payload
  if(pcrc){
    memcpy(buf + HEADER_SIZE, pkt->payload, length);
  // CRC2
    crc = htonl(crc32(0, (Bytef*)(pkt->payload), length));
    memcpy(buf + HEADER_SIZE + length, &crc, 4);
  }
  *len = (size_t)(HEADER_SIZE + pcrc * (pkt->length + 4));
  return PKT_OK;
}

ptypes_t pkt_get_type(const pkt_t* pkt){
  return pkt->type;
}
uint8_t pkt_get_tr(const pkt_t* pkt){
  return pkt->tr;
}
uint8_t pkt_get_window(const pkt_t* pkt){
  return pkt->window;
}
uint8_t pkt_get_seqnum(const pkt_t* pkt){
  return pkt->seqnum;
}
uint16_t pkt_get_length(const pkt_t* pkt){
  return pkt->length;
}
uint32_t pkt_get_timestamp(const pkt_t* pkt){
  return pkt->timestamp;
}
uint32_t pkt_get_crc1(const pkt_t* pkt){
  return pkt->crc1;
}
uint32_t pkt_get_crc2(const pkt_t* pkt){
  return pkt->crc2;
}
const char* pkt_get_payload(const pkt_t* pkt){
  return pkt->payload;
}

pkt_status_code pkt_set_type(pkt_t *pkt, const ptypes_t type)
{
  if(type < 1 || type > 3)
    return E_TYPE;
  pkt->type = type;
  return PKT_OK;
}
pkt_status_code pkt_set_tr(pkt_t *pkt, const uint8_t tr)
{
  if(tr > 1)
    return E_TR;
  if(pkt->type != PTYPE_DATA && tr)
    return E_UNCONSISTENT;
  pkt->tr = tr;
  return PKT_OK;
}
pkt_status_code pkt_set_window(pkt_t *pkt, const uint8_t window)
{
  if(window > MAX_WINDOW_SIZE)
    return E_WINDOW;
  pkt->window = window;
  return PKT_OK;
}
pkt_status_code pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum)
{
  pkt->seqnum = seqnum;
  return PKT_OK;
}
pkt_status_code pkt_set_length(pkt_t *pkt, const uint16_t length)
{
  if(length > MAX_PAYLOAD_SIZE)
    return E_LENGTH;
  pkt->length = length;
  return PKT_OK;
}
pkt_status_code pkt_set_timestamp(pkt_t *pkt, const uint32_t timestamp)
{
  pkt->timestamp = timestamp;
  return PKT_OK;
}
pkt_status_code pkt_set_crc1(pkt_t *pkt, const uint32_t crc1)
{
  pkt->crc1 = crc1;
  return PKT_OK;
}
pkt_status_code pkt_set_crc2(pkt_t *pkt, const uint32_t crc2)
{
  pkt->crc2 = crc2;
  return PKT_OK;
}
pkt_status_code pkt_set_payload(pkt_t *pkt, const char *data, const uint16_t length)
{
  if(data == NULL){
    free(pkt->payload);
    pkt->payload = NULL;
    pkt->length = 0;
    return PKT_OK;
  }
  if(length > 512)
    return E_LENGTH;

  char* payload = (char*)malloc(length);
  if(payload == NULL){
    pkt->length = 0;
    return E_NOMEM;
  }
  memcpy(payload, data, length);
  if(pkt->payload != NULL)
    free(pkt->payload);

  pkt->payload = payload;
  pkt->length = length;
  return PKT_OK;
}
