/*
 * ZMap Copyright 2013 Regents of the University of Michigan
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 */

#include "recv.h"
#include "recv-internal.h"

#include "../lib/includes.h"
#include "../lib/logger.h"

#include <errno.h>
#include <unistd.h>

#include "state.h"
#include <unistd.h>

void* xsk_recv();
void* xsk_delete();

#define MAX_PKT_SIZE 1500
uint8_t pkt_buf[MAX_PKT_SIZE] = {0};

void recv_init()
{
	log_info("recv-xdp", "creating new af_xdp socket on interface %s", zconf.iface);
    if(zconf.xdp.xsk == NULL)
        zconf.xdp.xsk = xsk_new(zconf.iface);
	zconf.data_link_size = sizeof(struct ether_header);
}

void recv_cleanup()
{
    xsk_delete(zconf.xdp.xsk);
}

void recv_packets()
{
    size_t len = MAX_PKT_SIZE;
    memset(&pkt_buf, 0, MAX_PKT_SIZE);
    xsk_recv(zconf.xdp.xsk, &pkt_buf, &len);
	struct timespec ts;
	handle_packet(len, pkt_buf, ts);
}

int recv_update_stats(void)
{
	return EXIT_SUCCESS;
}
