
/*
 * ZMap Copyright 2013 Regents of the University of Michigan
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef ZMAP_SEND_XDP_H
#define ZMAP_SEND_XDP_H

#include "../lib/includes.h"
#include "state.h"

void* xsk_send();



int send_run_init(sock_t s)
{
    return 0;
}

int send_packet(sock_t sock, void *buf, int len, UNUSED uint32_t idx)
{
	log_info("send-xdp", "sending packet over af_xdp");
    xsk_send(zconf.xdp.xsk, buf, len);
    return 0;
}

#endif /* ZMAP_SEND_XDP_H */
