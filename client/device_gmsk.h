#ifndef DEVICE_GMSK_H
#define DEVICE_GMSK_H

int gmsk_parse_header(unsigned char* buf, int buf_len);
int gmsk_parse_data(unsigned char* buf, int buf_len);

#endif
