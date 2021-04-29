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

#define MAX_PKT_SIZE 1500

void recv_init()
{
	log_info("recv-xdp", "creating new af_xdp socket on interface %s", zconf.iface);
    if(zconf.xdp.xsk == NULL)
        zconf.xdp.xsk = xsk_new(zconf.iface);
}

void recv_cleanup()
{
}

void recv_packets()
{
	log_info("recv-xdp", "receiving packet");
    size_t len = 1500;
    char pkt_buf[MAX_PKT_SIZE] = {0};
    xsk_recv(zconf.xdp.xsk, &pkt_buf, len);
	struct timespec ts;
	handle_packet(len, pkt_buf, ts);
}

int recv_update_stats(void)
{
	return EXIT_SUCCESS;
}
