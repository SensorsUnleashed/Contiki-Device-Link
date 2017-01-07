#pragma once
#include <stdio.h>

#define DEBUG 1
#undef DEBUG

#define DBG_NEWLINE "\n"

#define INFO(...) printf(__VA_ARGS__); printf(DBG_NEWLINE);
#define INFOX(...); printf(__VA_ARGS__);
#define ERR(...) printf(__VA_ARGS__); printf(DBG_NEWLINE);

#ifdef DEBUG
    #define DBG(...) fprintf(stderr,"%s:%d ",__FILE__,__LINE__); fprintf(stderr,__VA_ARGS__); fprintf(stderr,"\r\n"); fflush(stderr);
    #define DBGX(...) fprintf(stderr,__VA_ARGS__); fflush(stderr);
    #define DBGLX(...) fprintf(stderr,"%s:%d ",__FILE__,__LINE__); fprintf(stderr,__VA_ARGS__); fflush(stderr);
    #define DBG_PDU() printBin(); fflush(stderr);
#else
    #define DBG(...) {};
	#define DBGX(...) {};
	#define DBGLX(...) {};
	#define DBG_PDU() {};
#endif
