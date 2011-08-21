#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/io.h>
#include <sys/types.h>
#include <syslog.h>
#include <pwd.h>
#include <unistd.h>

#include "hdd.h"


//Global vars for signal handling cleanup.
struct disk_stat *diskp_first=NULL;
char *lineptr=NULL;

int main(int argc, char **argv) {
	u8 val_gp1=0x0, val_gp5=0x0; // Values for GPIO registers.
	struct disk_stat *diskp=NULL, *diskp_new=NULL;
	char *data=NULL, *tok=NULL;
	char temp_name[10];
	int temp_read=0, temp_written=0, matched=0;
	FILE *fp=NULL;

	if(fork()) { exit(0); }  //I can haz child?

	if((ioperm(BASEIO,BASEIO_SZ,1))!=0) {
		perror("ioperm");
		exit(0);
	}

	if(drop_privs()!=0) {
		syslog(LOG_ALERT,"Warning, unable to drop privileges, continuing to run as root.\n");
	}

	init_signal_handler();
	lineptr=data=(char *)malloc(sizeof(char)*4096);
	//MAIN LOOP
	for(;;) {
		if((fp=fopen(DISK_STAT,"r"))==NULL) {
			perror("fopen");
			exit(0);
		}
		data=lineptr;
		memset(data,'\0',4096);
		
		while(!feof(fp)) {
			*data=fgetc(fp);
			data++;
		}
		data--;		//Truncate weird character at end of /proc/diskstats
		*data='\0';
		fclose(fp);
		data=lineptr;
		for(tok=strsep(&data,"\n");tok!=NULL;tok=strsep(&data,"\n")) {
			if(strlen(tok)) {
				sscanf(tok,"%*d %*d %s 	%*d %*d %d %*d %*d %*d %d",temp_name,&temp_read,&temp_written);
				if(diskp_first==NULL) {
					diskp_new=(struct disk_stat *)malloc(sizeof(struct disk_stat));
					memset(diskp_new,'\0',sizeof(struct disk_stat));
					strncpy(diskp_new->device_name,temp_name,strlen(temp_name));
					diskp_new->sectors_read=temp_read;
					diskp_new->sectors_written=temp_written;
					diskp_new->read_act=diskp_new->write_act=0;
					diskp_new->next=NULL;
					diskp_first=diskp_new;
				}
				else {
					for(diskp=diskp_first;diskp!=NULL;diskp=diskp->next) {
						if((strcmp(diskp->device_name,temp_name))==0) {
							if(temp_read > diskp->sectors_read) {
								diskp->read_act=1;
								diskp->sectors_read=temp_read;
							}
							else { diskp->read_act=0; }
							if(temp_written > diskp->sectors_written) {
								diskp->write_act=1;
								diskp->sectors_written=temp_written;
							}
							else { diskp->write_act=0; }
							matched=1;
							break;
						}
					}
					if(!matched) {
						diskp_new=(struct disk_stat *)malloc(sizeof(struct disk_stat));
						memset(diskp_new,'\0',sizeof(struct disk_stat));
						strncpy(diskp_new->device_name,temp_name,strlen(temp_name));
						diskp_new->sectors_read=temp_read;
						diskp_new->sectors_written=temp_written;
						diskp_new->write_act=diskp_new->read_act=0;
						diskp_new->next=NULL;
						for(diskp=diskp_first;diskp->next!=NULL;diskp=diskp->next) {}
						diskp->next=diskp_new;
						diskp=diskp_first;
					}
				}
			}
		}
		for(diskp=diskp_first;diskp->next!=NULL;diskp=diskp->next) {
			if(strcmp(diskp->device_name,HD1)==0) {
				if(diskp->read_act) { val_gp5|=HD1_BLUE; }
				if(diskp->write_act) { val_gp5|=HD1_RED; }
				if(!diskp->read_act) { val_gp5&=~HD1_BLUE; }
				if(!diskp->write_act) { val_gp5&=~HD1_RED; }
				continue; /* These continue's avoid evaluating unneccessary conditionals.  */
			}		  /* And obviously a continue on the last condition would have no effect. */
			if(strcmp(diskp->device_name,HD2)==0) {
				if(diskp->read_act) { val_gp5|=HD2_BLUE; }
				if(diskp->write_act) { val_gp5|=HD2_RED; }
				if(!diskp->read_act) { val_gp5&=~HD2_BLUE; }
				if(!diskp->write_act) { val_gp5&=~HD2_RED; }
				continue;
			}
			if(strcmp(diskp->device_name,HD3)==0) {
				if(diskp->read_act) { val_gp5|=HD3_BLUE; }
				if(diskp->write_act) { val_gp5|=HD3_RED; }
				if(!diskp->read_act) { val_gp5&=~HD3_BLUE; }
				if(!diskp->write_act) { val_gp5&=~HD3_RED; }
				continue;
			}
			if(strcmp(diskp->device_name,HD4)==0) {
				if(diskp->read_act) { val_gp1|=HD4_BLUE; }
				if(diskp->write_act) { val_gp1|=HD4_RED; }
				if(!diskp->read_act) { val_gp1&=~HD4_BLUE; }
				if(!diskp->write_act) { val_gp1&=~HD4_RED; }
			}
		}
		outb(val_gp1,BASEIO+GP1);
		outb(val_gp5,BASEIO+GP5);
		usleep(SAMPLE_RATE);
	} //END MAIN LOOP
	return 0; //Never reached.
}


int drop_privs() {
	struct passwd *pw;
	int ret;
	if((pw=getpwnam("nobody"))==NULL) {
		return -1;
	}
	if((ret=setuid(pw->pw_uid))!=0) { return ret; }
	if((ret=setgid(pw->pw_gid))!=0) { return ret; }
	return 0;
}

