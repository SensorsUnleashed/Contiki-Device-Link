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

#define DEBUG 1
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

joinpair_t pairs[20];
int pairscount = 0;

/*
 * Get a list of destinations connected to the resource p
 * TODO: Make this a list of pairs, instead of a single one
 * */
joinpair_t* getUartSensorPair(uartsensors_device_t* p){
	joinpair_t* pair = 0;
	for(int i=0; i<pairscount; i++){
		if(pairs[i].deviceptr == p){	//Is it the right device?
			pair = &pairs[i];
			break;
		}
	}
	return pair;
}

/*
 *	Restore pairing
 * */
void activateUartSensorPairing(uartsensors_device_t* p){
	for(int i=0; i<pairscount; i++){
		int urllen = strlen(p->conf.attr);
		if(urllen == strlen(MMEM_PTR(&pairs[i].srcurl))){
			if(strncmp(MMEM_PTR(&pairs[i].srcurl), p->conf.attr, urllen) == 0){
				pairs[i].devicetype = uartsensor;
				pairs[i].deviceptr = p;
			}
		}
	}
}


//Return 0 if success
//Return 1 if memory could not be allocated
uint8_t createPairing(uip_ip6addr_t* dst_ipaddr, char* dst_uri, char* src_uri, enum datatype_e type, void* deviceptr){

	//Store the pairing pointer
	if(mmem_alloc(&pairs[pairscount].dsturl, strlen(dst_uri)) == 0){
		//Blink red light, if pairing has errors
		//REST.set_response_status(response, REST.status.INTERNAL_SERVER_ERROR);
		return 1;
	}
	if(mmem_alloc(&pairs[pairscount].srcurl, strlen(src_uri)) == 0){
		//Blink red light, if pairing has errors
		//REST.set_response_status(response, REST.status.INTERNAL_SERVER_ERROR);
		return 1;
	}

	memcpy(&pairs[pairscount].destip, &dst_ipaddr, sizeof(uip_ip6addr_t));
	memcpy(MMEM_PTR(&pairs[pairscount].dsturl), dst_uri, strlen(dst_uri));
	memcpy(MMEM_PTR(&pairs[pairscount].srcurl), src_uri, strlen(src_uri));
	pairs[pairscount].devicetype = type;
	pairs[pairscount].deviceptr = deviceptr;

	pairscount++;

	return 0;
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
// 2 = Src uri could not be parsed
// 3 = devices already paired
// 4 = Sensortype unknown
// 5 = Memory allocation failed
uint8_t pairing_handle(void* resource, enum datatype_e restype){

	uip_ip6addr_t dst_ipaddr;
	uint32_t bufindex = 0;
	uint32_t stringlen = 100;
	char stringbuf[100];
	uint8_t* payload = &buffer[0];
	char* src_uri;

	if(cp_decodeU16Array((uint8_t*) payload + bufindex, (uint16_t*)&dst_ipaddr, &bufindex) != 0){
		return 1;
	}

	if(cp_decode_string((uint8_t*) payload + bufindex, &stringbuf[0], &stringlen, &bufindex) != 0){
		return 2;
	}

	stringlen++;	//We need the /0 also

	//Check if the pairing is already done
	if(getUartSensorPair(resource) != 0){	//THIS IS NOT ENOUGH!!!
		//Ok we know that a pairing exists.

		return 3;
	}



	if(restype == uartsensor){
		uartsensors_device_t* resptr = (uartsensors_device_t*)resource;
		//Append the srcurl to the message
		cp_encodeString((uint8_t*) payload + bufindex, resptr->conf.attr, strlen(resptr->conf.attr), &bufindex);
		src_uri = resptr->conf.attr;
	}
	else{
		//Ups - not implemented this kind of resource yet
		return 4;
	}

	//Store pairing info into flash
	store_SensorPair(payload, bufindex);

	//Next create the pair, to make it work without a reboot (TODO: Do this from flash, to verify the pairing)
	if(createPairing(&dst_ipaddr, stringbuf, src_uri, restype, resource) != 0){
		return 5;
	}

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
		if(parseMessage(uartsensor, &pairs[pairscount]) == 0){
			PRINTF("SrcUri: %s -> DstUri: %s\n", (char*)MMEM_PTR(&pairs[pairscount].srcurl), (char*)MMEM_PTR(&pairs[pairscount].dsturl));
			pairscount++;

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
