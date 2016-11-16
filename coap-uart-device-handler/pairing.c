/*
 * pairing.c
 *
 *  Created on: 12/11/2016
 *      Author: omn
 */
#include "pairing.h"

#include "contiki.h"
#include "cfs/cfs.h"
#include "../apps/uartsensors/cmp.h"
#include "uartsensors.h"
#include <string.h>
#include "net/ip/uip.h"
#include "lib/memb.h"

//#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

struct file_s{
	int offset;
	int fd;
};

static bool file_reader(cmp_ctx_t *ctx, void *data, uint32_t limit);
static uint32_t file_writer(cmp_ctx_t* ctx, const void *data, uint32_t count);

#define BUFFERSIZE	200
static uint8_t buffer[BUFFERSIZE] = { 0 };
static uint32_t bufsize = 0;

LIST(pairings_list);
MEMB(pairings, joinpair_t, 20);

list_t pairing_get_pairs(void)
{
  return pairings_list;
}

/*
 *	Restore pairing
 * */
void activateUartSensorPairing(uartsensors_device_t* p){

	joinpair_t *pair = NULL;

	for(pair = (joinpair_t *)list_head(pairings_list);
			pair; pair = pair->next) {
		int urllen = strlen(p->conf.attr);
		if(urllen == strlen((char*)MMEM_PTR(&pair->srcurl))){
			if(strncmp((char*)MMEM_PTR(&pair->srcurl), p->conf.attr, urllen) == 0){
				pair->devicetype = uartsensor;
				pair->deviceptr = p;
			}
		}
	}
}

uint8_t pairing_assembleMessage(const uint8_t* data, uint32_t len){
	//For now only one can pair at a time. There is a single buffer.
	memcpy(buffer + bufsize, data, len);
	bufsize += len;
	return 0;
}


//Return 0 if success
//Return >0 if error:
// 1 = IP address can not be parsed
// 2 = dst_uri could not be parsed
// 3 = Unable to allocate enough dynamic memory
// 4 = src_uri could not be parsed

uint8_t parseMessage(enum datatype_e restype, joinpair_t* pair){

	uint32_t stringlen = 100;
	char stringbuf[100];
	uint8_t* payload = &buffer[0];
	uint32_t bufindex = 0;

	if(cp_decodeU16Array((uint8_t*) payload + bufindex, (uint16_t*)&pair->destip, &bufindex) != 0){
		return 1;
	}

	if(cp_decode_string((uint8_t*) payload + bufindex, &stringbuf[0], &stringlen, &bufindex) != 0){
		return 2;
	}
	stringlen++;	//We need the /0 also
	if(mmem_alloc(&pair->dsturl, stringlen) == 0){
		return 3;
	}
	memcpy(MMEM_PTR(&pair->dsturl), stringbuf, stringlen);

	stringlen = 100;
	if(cp_decode_string((uint8_t*) payload + bufindex, &stringbuf[0], &stringlen, &bufindex) != 0){
		return 4;
	}
	stringlen++;	//We need the /0 also
	if(mmem_alloc(&pair->srcurl, stringlen) == 0){
		return 3;
	}
	memcpy(MMEM_PTR(&pair->srcurl), stringbuf, stringlen);

	pair->devicetype = restype;
	pair->deviceptr = 0;

	return 0;
}

//Return 0 if success
//Return >0 if error:
// 1 = IP address can not be parsed
// 2 = dst_uri could not be parsed
// 3 = Unable to allocate enough dynamic memory
// 4 = src_uri could not be parsed
//5 = device already paired

uint8_t pairing_handle(void* resource, enum datatype_e restype){

	uint32_t bufindex = bufsize;
	uint8_t* payload = &buffer[0];

	//Append the src uri to the message.
	if(restype == uartsensor){
		uartsensors_device_t* resptr = (uartsensors_device_t*)resource;
		cp_encodeString((uint8_t*) payload + bufindex, resptr->conf.attr, strlen(resptr->conf.attr), &bufindex);
	}

	joinpair_t p;

	int ret = parseMessage(restype, &p);

	if(ret != 0) return ret;

	p.deviceptr = resource;

	//Check if the pairing is already done
	joinpair_t* pair;
	uartsensors_device_t* r = (uartsensors_device_t*) resource;
	for(pair = (joinpair_t *)list_head(pairings_list);
			pair; pair = pair->next) {
		int urllen = strlen(r->conf.attr);
		if(urllen == strlen((char*)MMEM_PTR(&pair->srcurl))){
			if(strncmp((char*)MMEM_PTR(&pair->srcurl), r->conf.attr, urllen) == 0){
				return 3;
			}
		}
	}

	pair = (joinpair_t*)memb_alloc(&pairings);
	if(pair == 0) return 3;

	//Add pair to the list of pairs
	memcpy(pair, &p, sizeof(joinpair_t));
	list_add(pairings_list, pair);

	//Finally store pairing info into flash
	store_SensorPair(payload, bufindex);

	bufsize = 0;	//Ready for next pairing
	return 0;
}

void store_SensorPair(uint8_t* data, uint32_t len){
	const char* filename = "pairs";

	struct file_s write;
	write.fd = cfs_open(filename, CFS_READ | CFS_WRITE | CFS_APPEND);
	write.offset = 0;

	if(write.fd < 0) {
		return;
	}

	cmp_ctx_t cmp;
	cmp_init(&cmp, &write, file_reader, file_writer);

	cmp_write_bin(&cmp, data, len);
	cfs_close(write.fd);
}

void restore_SensorPairs(void){
	const char* filename = "pairs";

	struct file_s read;
	read.fd = cfs_open(filename, CFS_READ);
	read.offset = 0;

	if(read.fd < 0) {
		return;
	}

	cmp_ctx_t cmp;
	cmp_init(&cmp, &read, file_reader, file_writer);
	bufsize = BUFFERSIZE;
	while(cmp_read_bin(&cmp, buffer, &bufsize)){
		joinpair_t pair;
		if(parseMessage(uartsensor, &pair) == 0){
			PRINTF("SrcUri: %s -> DstUri: %s\n", (char*)MMEM_PTR(&pair.srcurl), (char*)MMEM_PTR(&pair.dsturl));
			joinpair_t* p = (joinpair_t*)memb_alloc(&pairings);
			memcpy(p, &pair, sizeof(joinpair_t));
			if(p != 0){
				list_add(pairings_list, p);
			}
		}
		bufsize = BUFFERSIZE;
	}
	bufsize = 0;
	cfs_close(read.fd);
}

static bool file_reader(cmp_ctx_t *ctx, void *data, uint32_t len) {

	struct file_s* file = (struct file_s*)ctx->buf;
	if(file->fd >= 0) {
		cfs_seek(file->fd, file->offset, CFS_SEEK_SET);
		file->offset += cfs_read(file->fd, data, len);
	}

	return true;
}

static uint32_t file_writer(cmp_ctx_t* ctx, const void *data, uint32_t len){

	struct file_s* file = (struct file_s*)ctx->buf;

	if(file->fd >= 0) {
		len = cfs_write(file->fd, data, len);
	}

	return len;
}
