#include "socket.h"
#include "state.h"

void* xsk_new();
void* xsk_delete();

#define MAX_PKT_SIZE 1500


sock_t get_socket(uint32_t id) {
	log_info("socket-xdp", "creating new af_xdp socket on interface %s", zconf.iface);
    if(zconf.xdp.xsk == NULL)
        zconf.xdp.xsk = xsk_new(zconf.iface);

    /*
    int i, j;
    int pkts_to_recv = 10;
    size_t len = 1500;


    for(i = 0; i < pkts_to_recv; i++) {
        char buf[MAX_PKT_SIZE] = {0};
        xsk_recv(xsk, &buf, len);
        for(j = 0; j < MAX_PKT_SIZE; j++) {
            printf("0x%hhx,", buf[j]);
        }
        printf("\n");
    }

    char pkt_to_send[50] = {0x82, 0xff, 0x40, 0x35, 0x17, 0xa2, 0x9e, 0x4f, 0x30, 0x9e, 0xe1, 0x31, 0x08, 0x00, 0x45, 0x00,
                            0x00, 0x24, 0x00, 0x00, 0x40, 0x00, 0x14, 0x11, 0x5b, 0x75, 0xc0, 0xa8, 0x45, 0x01, 0xc0, 0xa8,
                            0x45, 0x02, 0x04, 0xd2, 0x10, 0xe1, 0x00, 0x10, 0xde, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00};


    for(i = 0; i < pkts_to_recv; i++) {
        xsk_send(xsk, &pkt_to_send, 50);
    }


    xsk_delete(xsk);
    return 0;
    */

	sock_t sock;
    sock.sock = id;
    return sock;
}
