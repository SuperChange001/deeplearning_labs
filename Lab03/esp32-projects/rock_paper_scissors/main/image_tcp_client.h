#ifndef MNIST_TCP_CLIENT_SEND
#define MNIST_TCP_CLIENT_SEND

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

void image_tcp_client_send(int8_t category_index, int8_t score, uint8_t * buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif