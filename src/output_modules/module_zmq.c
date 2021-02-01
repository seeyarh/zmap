/*
 * ZMap Copyright 2013 Regents of the University of Michigan
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../../lib/logger.h"
#include "../../lib/xalloc.h"

#include "output_modules.h"

#include <zmq.h>

#define UNUSED __attribute__((unused))

#define MAX_BUFFER_SIZE 1000

static char *output_buffer;
static void *zmq_ctx = NULL;
static void *zmq_push = NULL;

int zmq_module_init(struct state_conf *conf, UNUSED char **fields,
		    UNUSED int fieldlens)
{

	char *connect_string = NULL;
	if (conf->output_args) {
		log_debug("zmq", "output args %s", conf->output_args);
		connect_string = conf->output_args;
	} else {
		connect_string = strdup("tcp://localhost:5555");
	}

	zmq_ctx = zmq_ctx_new();
	zmq_push = zmq_socket(zmq_ctx, ZMQ_PUSH);
	int rc = zmq_connect(zmq_push, connect_string);
	if (rc < 0) {
		log_fatal("zmq"
			  "failed to connect to %s, err %s",
			  connect_string, zmq_strerror(errno));
		return EXIT_FAILURE;
	}

	// generate field names CSV list to be logged.
	char *fieldstring = xcalloc(1000, fieldlens);
	memset(fieldstring, 0, sizeof(fields));
	for (int i = 0; i < fieldlens; i++) {
		if (i) {
			strcat(fieldstring, ", ");
		}
		strcat(fieldstring, fields[i]);
	}
	log_info("zmq", "the following fields will be output to zmq: %s.",
		 fieldstring);
	free(fields);

	output_buffer = xcalloc(MAX_BUFFER_SIZE, sizeof(char));

	return EXIT_SUCCESS;
}

#define INT_STR_LEN 20 // len(9223372036854775807) == 19

static size_t guess_csv_string_length(fieldset_t *fs)
{
	size_t len = 0;
	for (int i = 0; i < fs->len; i++) {
		field_t *f = &(fs->fields[i]);
		if (f->type == FS_STRING) {
			len += strlen(f->value.ptr);
			len += 2; // potential quotes
		} else if (f->type == FS_UINT64) {
			len += INT_STR_LEN;
		} else if (f->type == FS_BOOL) {
			len +=
			    INT_STR_LEN; // 0 or 1 PRIi32 is used to print ...
		} else if (f->type == FS_BINARY) {
			len += 2 * f->len;
		} else if (f->type == FS_NULL) {
			// do nothing
		} else {
			log_fatal("csv",
				  "received unknown output type "
				  "(not str, binary, null, or uint64_t)");
		}
	}

	// estimated length + number of commas
	return len + (size_t)len + 256;
}

static void hex_encode_str(char *f, unsigned char *readbuf, size_t len)
{
	char *temp = f;
	for (size_t i = 0; i < len; i++) {
		sprintf(temp, "%02x", readbuf[i]);
		temp += (size_t)2 * sizeof(char);
	}
}

void make_csv_string(fieldset_t *fs, char *out, size_t len)
{
	memset(out, 0, len);
	for (int i = 0; i < fs->len; i++) {
		char *temp = out + (size_t)strlen(out);
		field_t *f = &(fs->fields[i]);
		char *dataloc = temp;
		if (i) { // only add comma if not first element
			sprintf(temp, ",");
			dataloc += (size_t)1;
		}
		if (f->type == FS_STRING) {
			if (strlen(dataloc) + strlen((char *)f->value.ptr) >=
			    len) {
				log_fatal("zmq-csv",
					  "out of memory---will overflow");
			}
			if (strchr((char *)f->value.ptr, ',')) {
				sprintf(dataloc, "\"%s\"",
					(char *)f->value.ptr);
			} else {
				sprintf(dataloc, "%s", (char *)f->value.ptr);
			}
		} else if (f->type == FS_UINT64) {
			if (strlen(dataloc) + INT_STR_LEN >= len) {
				log_fatal("zmq-csv",
					  "out of memory---will overflow");
			}
			sprintf(dataloc, "%" PRIu64, (uint64_t)f->value.num);
		} else if (f->type == FS_BOOL) {
			if (strlen(dataloc) + INT_STR_LEN >= len) {
				log_fatal("zmq-csv",
					  "out of memory---will overflow");
			}
			sprintf(dataloc, "%" PRIi32, (int)f->value.num);
		} else if (f->type == FS_BINARY) {
			if (strlen(dataloc) + 2 * f->len >= len) {
				log_fatal("zmq-csv",
					  "out of memory---will overflow");
			}
			hex_encode_str(dataloc, (unsigned char *)f->value.ptr,
				       f->len);
		} else if (f->type == FS_NULL) {
			// do nothing
		} else {
			log_fatal("zmq-csv", "received unknown output type");
		}
	}
}

int zmq_module_process(fieldset_t *fs)
{
	size_t reqd_space = guess_csv_string_length(fs);
	make_csv_string(fs, output_buffer, reqd_space);
	log_debug("zmq", "sending %s of size %d", output_buffer,
		  strlen(output_buffer));
	int rc = zmq_send(zmq_push, output_buffer, strlen(output_buffer), 0);
	if (rc < 0) {
		log_fatal("zmq", "failed to send %s, err: %s", output_buffer,
			  zmq_strerror(errno));
		return EXIT_FAILURE;
	}
	memset(output_buffer, 0, MAX_BUFFER_SIZE);

	return EXIT_SUCCESS;
}

int zmq_module_close(UNUSED struct state_conf *c, UNUSED struct state_send *s,
		     UNUSED struct state_recv *r)
{
	zmq_close(zmq_push);
	zmq_ctx_destroy(zmq_ctx);
	return EXIT_SUCCESS;
}

output_module_t module_zmq = {
    .name = "zmq",
    .init = &zmq_module_init,
    .start = NULL,
    .update = NULL,
    .update_interval = 0,
    .close = &zmq_module_close,
    .process_ip = &zmq_module_process,
    .supports_dynamic_output = NO_DYNAMIC_SUPPORT,
    .helptext =
	"Outputs one or more output fields in csv, and then flushes out to zmq. \n"
	"By default, the probe module does not filter out duplicates or limit to successful fields, \n"
	"but rather includes all received packets. Fields can be controlled by \n"
	"setting --output-fileds. Filtering out failures and duplicate packets can \n"
	"be achieved by setting an --output-filter."};
