#ifndef MNIST_TCP_CLIENT_SEND
#define MNIST_TCP_CLIENT_SEND

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

void mnist_tcp_client_send(uint8_t * buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif