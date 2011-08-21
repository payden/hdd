#ifndef _HDD_H
#define _HDD_H

#define BASEIO 0x800
#define BASEIO_SZ 0x7f

#define DISK_STAT "/proc/diskstats"

#define GP1 0x4b
#define GP5 0x4f

#define HD1_RED (1 << 7)
#define HD1_BLUE (1 << 6)
#define HD2_RED (1 << 3)
#define HD2_BLUE (1 << 2)
#define HD3_RED (1 << 1)
#define HD3_BLUE (1 << 0)
#define HD4_RED (1 << 1)
#define HD4_BLUE (1 << 4)

#define SAMPLE_RATE 50000

#define HD1 "sda"
#define HD2 "sdb"
#define HD3 "sdc"
#define HD4 "sdd"

#define u8 unsigned char

//Structure for storing some simple disk statistics

struct disk_stat {
	char device_name[8];
	int sectors_read;
	int sectors_written;
	int read_act;
	int write_act;
	struct disk_stat *next;
};

void init_signal_handler(void);
void cleanup(int sig);
int drop_privs(void);

#endif
