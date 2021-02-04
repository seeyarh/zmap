#!/bin/bash

mkdir -p /tmp/zmap

./pull_client > /tmp/zmap/zmap-zmq-recvd &
zmap -p 80 -f saddr,sport --output-module zmq --output-args="tcp://localhost:5555" -n 1000 2> /tmp/zmap/zmap-stderr

zmap_rx_pkts=$(tail -n 2 /tmp/zmap/zmap-stderr | head -n 1 |cut -d ' ' -f 13)
zmq_rx_pkts=$(wc -l /tmp/zmap/zmap-zmq-recvd | cut -d ' ' -f 1)
echo "zmap rx packets      $zmap_rx_pkts"
echo "zmq  rx packets      $zmq_rx_pkts"
