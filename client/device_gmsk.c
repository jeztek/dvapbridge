#include <stdio.h>
#include <string.h>

#include "common.h"
#include "device.h"
#include "device_gmsk.h"

int
gmsk_parse_header(unsigned char* buf, int buf_len)
{
  int i;
  union dvap_dstar_header_union dstar;
  char str[9];

  if (buf_len != sizeof(dvap_dstar_header_t)) {
    return FALSE;
  }

  for (i = 0; i < buf_len; i++) {
    dstar.bytes[i] = buf[i];
  }

  printf("gmsk header - ");
  printf("stream_id: %d, ", (dstar.header.stream_id[1] << 8) +
         dstar.header.stream_id[0]);
  printf("header: %d, ", dstar.header.header_flag);
  printf("end: %d, ", dstar.header.end_of_stream_flag);
  printf("prev header pkt: %d\n", dstar.header.using_prev_header_packet_flag);
  printf("              frame_pos: %d, ", dstar.header.frame_pos);
  printf("seq: %d\n", dstar.header.seq);

  str[8] = 0;
  strncpy(str, (char *)dstar.header.rpt1, 8);
  printf("rpt1: [%s], ", str);

  strncpy(str, (char *)dstar.header.rpt2, 8);
  printf("rpt2: [%s], ", str);

  strncpy(str, (char *)dstar.header.urcall, 8);
  printf("urcall: [%s], ", str);

  strncpy(str, (char *)dstar.header.mycall, 8);
  printf("mycall: [%s]\n", str);

  return TRUE;
}

int
gmsk_parse_data(unsigned char* buf, int buf_len)
{
  int i;
  union dvap_dstar_data_union dstar;

  if (buf_len != sizeof(dvap_dstar_data_t)) {
    return FALSE;
  }

  for (i = 0; i < buf_len; i++) {
    dstar.bytes[i] = buf[i];
  }

  printf("gmsk data - ");
  printf("stream_id: %d, ", (dstar.data.stream_id[1] << 8) + 
         dstar.data.stream_id[0]);
  printf("header: %d, ", dstar.data.header_flag);
  printf("end: %d, ", dstar.data.end_of_stream_flag);
  printf("prev header pkt: %d\n", dstar.data.using_prev_header_packet_flag);
  printf("            frame_pos: %d, ", dstar.data.frame_pos);
  printf("seq: %d\n", dstar.data.seq);
  return TRUE;
}
