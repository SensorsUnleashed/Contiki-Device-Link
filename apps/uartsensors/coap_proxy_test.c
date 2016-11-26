/*
 * coap_proxy_test.c
 *
 *  Created on: 18/09/2016
 *      Author: omn
 */
#include "contiki.h"
#include <stdio.h>      // standard input / output functions
#include <stdlib.h>
#include <string.h>     // string function definitions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions

/*
 * To use the QT sensor test app, start this command from the command line,
 * and attach to the interface given.
 * socat -d -d PTY PTY
 * */

#define END     0xC0
#define ESC     0xDB
#define ESC_END 0xDC
#define ESC_ESC 0xDD

#define ACK     0x06
#define NAK     0x15

process_event_t event_data_ready;
PROCESS(coap_proxy_test, "Coap uart proxy test");

static int (* input_handler)(unsigned char c);
static int fd;

int set_interface_attribs(int fd, int speed)
{
	struct termios tty;

	if (tcgetattr(fd, &tty) < 0) {
		printf("Error from tcgetattr: %s\n", strerror(errno));
		return -1;
	}

	cfsetospeed(&tty, (speed_t)speed);
	cfsetispeed(&tty, (speed_t)speed);

	tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;         /* 8-bit characters */
	tty.c_cflag &= ~PARENB;     /* no parity bit */
	tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
	tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

	/* setup for non-canonical mode */
	tty.c_cc[VMIN]   =  0;                  // read doesn't block
	tty.c_cc[VTIME]  =  0;                  // 0.5 seconds read timeout
	tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

	/* Make raw */
	cfmakeraw(&tty);


	/* Flush Port, then applies attributes */
	tcflush( fd, TCIFLUSH );

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		printf("Error from tcsetattr: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}


void uart_set_input(uint8_t uart, int (* input)(unsigned char c))
{
	input_handler = input;

	//init uart
	char *portname = "/dev/pts/2";

	int wlen;

	fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK);
	if (fd < 0) {
		printf("Error opening %s: %s\n", portname, strerror(errno));
		return;
	}
	/*baudrate 115200, 8 bits, no parity, 1 stop bit */
	set_interface_attribs(fd, B115200);

    /* delay for output */
	//Start the process from here
	process_start(&coap_proxy_test, 0);
}

void uart_write_byte(uint8_t uart, uint8_t c)
{
	static char last_c = 0;
	if(c == ESC_ESC && last_c == ESC){
		write(fd, &last_c, 1);
	}
	else if(c == ESC_END && last_c == ESC){
		write(fd, (char)END, 1);
	}
	else{
		write(fd, &c, 1);
	}
	last_c = c;

	tcdrain(fd);    /* delay for output */
}
char c;
PROCESS_THREAD(coap_proxy_test, ev, data)
{
	static struct etimer et;

	PROCESS_BEGIN();

	printf("Coap proxy test started\n");

	etimer_set(&et, CLOCK_SECOND/10);

	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		while(read(fd, &c, 1) > 0){
			input_handler(c);
		}
		etimer_reset(&et);
	}
	PROCESS_END();
}





