/*
 * reverseNotify.c
 *
 *  Created on: 28. dec. 2017
 *      Author: omn
 */
#include "contiki.h"
#include "net/ip/uiplib.h"
#include "mmem.h"
#include "lib/memb.h"
#include "lib/list.h"
#include "cfs/cfs.h"
#include "cmp.h"
#include "susensors.h"
#include "reverseNotify.h"

LIST(revlookup);
MEMB(revlookup_memb, revlookup_t, 20);

static void writeFile();
static bool file_reader(cmp_ctx_t *ctx, void *data, uint32_t len);
static uint32_t file_writer(cmp_ctx_t* ctx, const void *data, uint32_t len);

static const char* filename = "revnotify";

struct file_s{
	int offset;
	int fd;
};

void initReverseNotify(){
	list_init(revlookup);
	memb_init(&revlookup_memb);

	struct file_s read;
	read.fd = cfs_open(filename, CFS_READ);
	read.offset = 0;

	if(read.fd < 0) {
		return;
	}

	cmp_ctx_t cmp;
	cmp_init(&cmp, &read, file_reader, file_writer);
	uint32_t size;

	while(cmp_read_array(&cmp, &size) == true){
		if(size != 8) return;
		revlookup_t* addr = (revlookup_t*)memb_alloc(&revlookup_memb);
		if(addr == NULL) return;

		for(int j=0; j<size; j++){
			cmp_read_u16(&cmp, &addr->srcip.u16[j]);
		}
		list_add(revlookup, addr);

		//Signal to the susensor class, that a remote node needs to know of our presence.
		process_post(&susensors_process, susensors_presence, addr);
	}
}

void addSource(uip_ip6addr_t* srcaddr){
	int add = 1;

	//Find out if the addr is already in our list
	for(revlookup_t* i = list_head(revlookup); i && add; i = list_item_next(i)){
		add = 0;
		for(int j=0; j<8; j++){
			if(srcaddr->u16[j] != i->srcip.u16[j]){
				add = 1;
				break;
			}
		}
	}

	//Add to the list and write the entire list to flash.
	//This will seldom happen, so no need to append to the
	//list. It only adds complexity
	if(add){
		revlookup_t* addr = (revlookup_t*)memb_alloc(&revlookup_memb);

		if(addr){
			memcpy(&addr->srcip, srcaddr, 16);
			list_add(revlookup, addr);
			writeFile();
		}
	}
}

void removeItem(revlookup_t* item){
	list_remove(revlookup, item);
	writeFile();
}

static void writeFile(){
	cmp_ctx_t cmp;
	struct file_s write;
	write.fd = cfs_open(filename, CFS_READ | CFS_WRITE );
	write.offset = 0;

	if(write.fd < 0) {
		return;
	}

	if(list_length(revlookup) == 0){
		cfs_remove(filename);
	}
	else{
		cmp_init(&cmp, &write, 0, file_writer);

		for(revlookup_t* i = list_head(revlookup); i; i = list_item_next(i)){
			cmp_write_array(&cmp, 8);
			for(int j=0; j<8; j++){
				cmp_write_u16(&cmp, i->srcip.u16[j]);
			}
		}
	}

	cfs_close(write.fd);
}

static bool file_reader(cmp_ctx_t *ctx, void *data, uint32_t len){

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
