/*
 * coap_proxy_protocolhandler.c
 *
 *  Created on: 18/09/2016
 *      Author: omn
 */
#include "coap_proxy_protocolhandler.h"
#include "lib/crc16.h"
#include <string.h>

/*
 *
 * Return:
 * 			 0 for OK
 * 			-1 for CRC error
 * */
int cp_decodemessage(char* source, int len, rx_msg* destination){

	char* plbuf = (char*)destination->payload;

	//Verify the integrity of the data
	char msglen = *(source + len - 3);
	unsigned short crc = *((unsigned short*)(source + len - 2));
	unsigned short crct = crc16_data((unsigned char*)source, len - 2, 0);
	if(crct != crc) return -1;	//UPS - message not ok

	//Finally store the message in a container, for later use
	//Resource strings will be stored in a seperate
	destination->seqno = *source++;
	destination->cmd = *source++;
	destination->len = msglen - 2;
	for(int i=0; i<destination->len; i++){
		*plbuf++ = *source++;
	}

	return 0;
}

/*
 * Encode the requested message
 * return: Buffer length
 * */
int cp_encodemessage(uint8_t msgid, enum req_cmd cmd, void* payload, char len, uint8_t* buffer){

	char* tmp = payload;
	char* buffer_start = (char*)buffer;
	int count;
	unsigned short crc;
	*buffer++ = msgid;
	*buffer++ = (char)cmd;
	for(int i=0; i<len; i++){
		*buffer++ = *tmp++;
	}
	count = (int)buffer - (int)buffer_start;
	*buffer++ = count;
	crc = crc16_data((uint8_t*)buffer_start, count+1, 0);
	*buffer++ = crc & 0xff;
	*buffer++ = crc >> 8;

	return ((int)buffer - (int)buffer_start);
}

//Used to read from msgpacked buffer
static bool buf_reader(cmp_ctx_t *ctx, void *data, size_t limit) {
	for(int i=0; i<limit; i++){
		 *((char*)data++) = *((char*)ctx->buf++);
	}
	return true;
}


static size_t buf_writer(cmp_ctx_t* ctx, const void *data, size_t count){
	for(int i=0; i<count; i++){
		*((uint8_t*)ctx->buf++) = *((char*)data++);
	}
	return count;
}

/*
 * MsgPack Encode the resourceconfiguration message
 * return: Buffer length
 * */
int cp_encoderesource_conf(struct resourceconf* data, uint8_t* buffer){
	cmp_ctx_t cmp;
	cmp_init(&cmp, buffer, buf_reader, buf_writer);

	cmp_write_u8(&cmp, data->id);
	cmp_write_u32(&cmp, data->resolution);
	cmp_write_u32(&cmp, data->hysteresis);
	cmp_write_u32(&cmp, data->version);
	cmp_write_u8(&cmp, data->flags);
	cmp_write_s32(&cmp, data->max_pollinterval);
	cmp_write_str(&cmp, data->unit, strlen(data->unit)+1);
	cmp_write_str(&cmp, data->spec, strlen(data->spec)+1);
	cmp_write_str(&cmp, data->type, strlen(data->type)+1);
	cmp_write_str(&cmp, data->group, strlen(data->group)+1);
	cmp_write_str(&cmp, data->attr, strlen(data->attr)+1);


	return (size_t)cmp.buf - (size_t)buffer;
}

int cp_encodereading(uint8_t* buffer, cmp_object_t *obj){
	cmp_ctx_t cmp;
	cmp_init(&cmp, buffer, 0, buf_writer);
	cmp_write_object(&cmp, obj);
	return (size_t)cmp.buf - (size_t)buffer;
}

int cp_decoderesource_conf(struct resourceconf* data, uint8_t* buffer, char* strings){
	cmp_ctx_t cmp;
	cmp_init(&cmp, buffer, buf_reader, buf_writer);

	cmp_read_u8(&cmp, &data->id);
	cmp_read_u32(&cmp, &data->resolution);
	cmp_read_u32(&cmp, &data->hysteresis);
	cmp_read_u32(&cmp, &data->version);
	cmp_read_u8(&cmp, &data->flags);
	cmp_read_s32(&cmp, &data->max_pollinterval);

	//Put the strings in a mem location and point to them from the data structure
	data->unit = strings;
	uint32_t len = 100;
	cmp_read_str(&cmp, strings, &len);
	strings += len;	//Keep the \0

	data->spec = strings;
	len = 100;	//How long can we allow it to be
	cmp_read_str(&cmp, strings, &len);
	strings += len;	//Keep the \0

	data->type = strings;
	len = 100;	//How long can we allow it to be
	cmp_read_str(&cmp, strings, &len);

	/* The coap resource URL is made up from type/group.
	 * For now we only use these fields for the url, so lets combine those*/
	strings += len-1;
	*strings++ = '/';

	data->group = strings;
	cmp_read_str(&cmp, strings, &len);
	strings += len;	//Keep the \0

	data->attr = strings;
	len = 100;	//How long can we allow it to be
	cmp_read_str(&cmp, strings, &len);
	strings += len;	//Keep the \0

	return (size_t)cmp.buf - (size_t)buffer;
}



