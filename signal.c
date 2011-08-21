#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>


#include "hdd.h"
extern struct disk_stat *diskp_first;
extern char *lineptr;

void init_signal_handler(void) {
	signal(SIGINT,(__sighandler_t)cleanup);
	signal(SIGTERM,(__sighandler_t)cleanup);
}

void cleanup(int sig) {
	struct disk_stat *diskp;
	outb(0x00,BASEIO+GP1);
	outb(0x00,BASEIO+GP5);
	diskp=diskp_first;
	while(diskp_first!=NULL) {
		diskp_first=diskp->next;
		free(diskp);
		diskp=diskp_first;
	}
	if(lineptr!=NULL) { free(lineptr); }
	exit(0);
}
