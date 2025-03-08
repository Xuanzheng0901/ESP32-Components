#ifndef __S0CKET_H
#define __S0CKET_H
#define SOCKET_DISCONNECTED 1 << 0
#define SOCKET_HEARTBEAT_IDLE 1 << 1
#define SOCKET_TRANSFORM_IDLE 1 << 2

// #define SOCKET_TX_BUF_READY 1 << 3
// #define SOCKET_TX_COMPLETE 1 << 4
void Socket_Init(void);
void Socket_Tx(char* tx_buffer);

#endif
