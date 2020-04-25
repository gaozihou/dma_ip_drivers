/*
 * This file is part of the Xilinx DMA IP Core driver tools for Linux
 *
 * Copyright (c) 2016-present,  Xilinx, Inc.
 * All rights reserved.
 *
 * This source code is licensed under BSD-style license (found in the
 * LICENSE file in the root directory of this source tree)
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <byteswap.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <pthread.h> 
#include <limits.h> 
#include <stdbool.h>

#include <sys/types.h>
#include <sys/mman.h>

/* ltoh: little to host */
/* htol: little to host */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define ltohl(x)       (x)
#define ltohs(x)       (x)
#define htoll(x)       (x)
#define htols(x)       (x)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define ltohl(x)     __bswap_32(x)
#define ltohs(x)     __bswap_16(x)
#define htoll(x)     __bswap_32(x)
#define htols(x)     __bswap_16(x)
#endif

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)

#define MAP_SIZE (32*1024UL)
#define MAP_MASK (MAP_SIZE - 1)


// Usable capacity is (QUEUE_CAPACITY-1)
#define QUEUE_CAPACITY 1024

// A structure to represent a queue 
// Thread safe (one thread read and another thread write)
struct Queue { 
    int front, rear;
    int* array; 
}; 

struct Queue* createQueue() { 
    struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue)); 
    queue->front = queue->rear = 0;  
    queue->array = (int*) malloc(QUEUE_CAPACITY * sizeof(int)); 
    return queue; 
} 

int isFull(struct Queue* queue) {
    return ((queue->front-queue->rear+QUEUE_CAPACITY)%QUEUE_CAPACITY == 1);  
} 
  
int isEmpty(struct Queue* queue) {
    return (queue->front == queue->rear);
} 

void enqueue(struct Queue* queue, int item) { 
    if (isFull(queue)) return; 
    queue->array[queue->rear] = item; 
    queue->rear = (queue->rear + 1)%QUEUE_CAPACITY; 
} 

int dequeue(struct Queue* queue) { 
    if (isEmpty(queue)) return INT_MIN; 
    int item = queue->array[queue->front]; 
    queue->front = (queue->front + 1)%QUEUE_CAPACITY; 
    return item; 
} 

void *getCharThread(void * queue) {
  int ch = -1;
  while(1) {
    ch = getchar();
    enqueue((struct Queue*)queue, ch);
  }
} 

static struct termios oldtty;
static int old_fd0_flags;

static void term_exit(void)
{
    tcsetattr (0, TCSANOW, &oldtty);
    fcntl(0, F_SETFL, old_fd0_flags);
}

static void term_init(bool allow_ctrlc)
{
    struct termios tty;

    memset(&tty, 0, sizeof(tty));
    tcgetattr (0, &tty);
    oldtty = tty;
    old_fd0_flags = fcntl(0, F_GETFL);

    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
                          |INLCR|IGNCR|ICRNL|IXON);
    tty.c_oflag |= OPOST;
    tty.c_lflag &= ~(ECHO|ECHONL|ICANON|IEXTEN);
    if (!allow_ctrlc)
        tty.c_lflag &= ~ISIG;
    tty.c_cflag &= ~(CSIZE|PARENB);
    tty.c_cflag |= CS8;
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    tcsetattr (0, TCSANOW, &tty);

    atexit(term_exit);
}

int main(int argc, char **argv)
{
	int fd;
	void *map_base, *virt_addr;
	uint32_t read_result, writeval, addr_result, data_result, head_result;
	off_t target;
	/* access width */
	int access_width = 'w';
	char *device;

	/* not enough arguments given? */
	if (argc < 3) {
		fprintf(stderr,
			"\nUsage:\t%s <device> <address> [[type] data]\n"
			"\tdevice  : character device to access\n"
			"\taddress : memory address to access\n"
			"\ttype    : access operation type : [b]yte, [h]alfword, [w]ord\n"
			"\tdata    : data to be written for a write\n\n",
			argv[0]);
		exit(1);
	}

	//printf("argc = %d\n", argc);

	device = strdup(argv[1]);
	//printf("device: %s\n", device);
	target = strtoul(argv[2], 0, 0);
	//printf("address: 0x%08x\n", (unsigned int)target);

	//printf("access type: %s\n", argc >= 4 ? "write" : "read");

	/* data given? */
	if (argc >= 4) {
		//printf("access width given.\n");
		access_width = tolower(argv[3][0]);
	}
	//printf("access width: ");
    /*
	if (access_width == 'b')
		printf("byte (8-bits)\n");
	else if (access_width == 'h')
		printf("half word (16-bits)\n");
	else if (access_width == 'w')
		printf("word (32-bits)\n");
	else {
		printf("word (32-bits)\n");
		access_width = 'w';
	}
    */

	if ((fd = open(argv[1], O_RDWR | O_SYNC)) == -1)
		FATAL;
	//printf("character device %s opened.\n", argv[1]);
	fflush(stdout);

	/* map one page */
	map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map_base == (void *)-1)
		FATAL;
	//printf("Memory mapped at address %p.\n", map_base);
	fflush(stdout);

	/* calculate the virtual address to be accessed */
	virt_addr = map_base + target;
	/* read only */
	if (argc <= 4) {
		//printf("Read from address %p.\n", virt_addr); 
		switch (access_width) {
		case 'b':
			read_result = *((uint8_t *) virt_addr);
			printf
			    ("Read 8-bits value at address 0x%08x (%p): 0x%02x\n",
			     (unsigned int)target, virt_addr,
			     (unsigned int)read_result);
			break;
		case 'h':
			read_result = *((uint16_t *) virt_addr);
			/* swap 16-bit endianess if host is not little-endian */
			read_result = ltohs(read_result);
			printf
			    ("Read 16-bit value at address 0x%08x (%p): 0x%04x\n",
			     (unsigned int)target, virt_addr,
			     (unsigned int)read_result);
			break;
		case 'w':
			read_result = *((uint32_t *) virt_addr);
			/* swap 32-bit endianess if host is not little-endian */
			read_result = ltohl(read_result);
			printf
			    ("Read 32-bit value at address 0x%08x (%p): 0x%08x\n",
			     (unsigned int)target, virt_addr,
			     (unsigned int)read_result);
			return (int)read_result;
			break;
		default:
			fprintf(stderr, "Illegal data type '%c'.\n",
				access_width);
			exit(2);
		}
		fflush(stdout);
	}
	/* data value given, i.e. writing? */
	if (argc >= 5) {
		writeval = strtoul(argv[4], 0, 0);
        //printf("Write 32-bits value 0x%08x to 0x%08x (0x%p)\n",
        //       (unsigned int)writeval, (unsigned int)target,
        //       virt_addr);
        /* swap 32-bit endianess if host is not little-endian */
        
        //*((uint32_t *) virt_addr) = htoll(writeval);
        uint32_t * virt_addr_ptr = (uint32_t *) virt_addr;
        uint32_t * load_addr_ptr = (uint32_t *) (map_base + 0x00000020);
        
        
        // mmio host program
        uint32_t * map_base_ptr = (uint32_t *) map_base;
        
        int counter = 0;
        const int num_cores = 1;
        
        // IO memory
        uint32_t io_memory[1<<18];
        
        struct Queue* queue = createQueue(); 
        pthread_t thread_id; 
        pthread_create(&thread_id, NULL, getCharThread, (void *)queue); 
        
        term_init(1);
        
        while (1){
            // drain the hardware buffer
            read_result = *map_base_ptr;
			read_result = ltohl(read_result);
            
            if (read_result > 0){
                head_result = *virt_addr_ptr;
            } else {
                break;
            }
        }

        
        // load nbf to hardware
        FILE *fp;
        char str[16];
        char* filename = argv[5];
     
        fp = fopen(filename, "r");
        if (fp == NULL){
            printf("Could not open file %s",filename);
            return 1;
        }
        while (fgets(str, 16, fp) != NULL){
            uint32_t word = (uint32_t)strtol(str, NULL, 16);
            //printf("%08x\n", word);
            //printf("%s", str);
            *virt_addr_ptr = htoll(word);
        }
        fclose(fp);

        
        while (1){
            // check if empty
			read_result = *map_base_ptr;
			read_result = ltohl(read_result);
            
            if (read_result > 0){
                // read addr and data
                head_result = *virt_addr_ptr;
                head_result = ltohl(head_result);
                addr_result = *virt_addr_ptr;
                addr_result = ltohl(addr_result);
                data_result = *virt_addr_ptr;
                data_result = ltohl(data_result);
                
                uint8_t  mc_src_x_cord = (head_result & 0x000000FF) >> 0;
                uint8_t  mc_src_y_cord = (head_result & 0x0000FF00) >> 8;
                uint8_t  mc_we         = (head_result & 0x00FF0000) >> 16;
                uint8_t  mc_mask       = (head_result & 0xFF000000) >> 24;
                
                if (mc_we == 1) {
                    if (((addr_result & 0x08000000) >> 27) == 0) {
                        
                        uint16_t mc_epa_addr = (addr_result & 0x00003FFF) << 2;
                        
                        if (mc_epa_addr == 0xEAD0) {
                            printf("[INFO][MONITOR] RECEIVED BSG_FINISH PACKET from tile y,x=%2d,%2d, data=%x\n", mc_src_y_cord, mc_src_x_cord, data_result);
                            break;
                        } else if (mc_epa_addr == 0xEAD4) {
                            printf("[INFO][MONITOR] RECEIVED TIME BSG_PACKET from tile y,x=%2d,%2d, data=%x\n", mc_src_y_cord, mc_src_x_cord, data_result);
                        } else if (mc_epa_addr == 0xEAD8) {
                            printf("[INFO][MONITOR] RECEIVED BSG_FAIL PACKET from tile y,x=%2d,%2d, data=%x\n", mc_src_y_cord, mc_src_x_cord, data_result);
                            break;
                        } else if (mc_epa_addr == 0xEADC) {
                            for (int i = 0; i < 4; i++) {
                                if ((mc_mask & (1 << i)) != 0) {
                                    printf("%c", (data_result >> (i*8)) & 0x000000FF);
                                }
                            }
                        } else {
                            printf("[INFO][MONITOR] RECEIVED BSG_IO PACKET from tile y,x=%2d,%2d, data=%x, addr=%x\n", mc_src_y_cord, mc_src_x_cord, data_result, addr_result);
                            uint32_t mc_write_result = io_memory[(addr_result & 0x0003FFFF)];
                            for (int i = 0; i < 4; i++) {
                                if ((mc_mask & (1 << i)) != 0) {
                                    mc_write_result = (mc_write_result & ~(0xFF<<(i*8))) | (data_result & (0xFF<<(i*8)));
                                }
                            }
                            io_memory[(addr_result & 0x0003FFFF)] = mc_write_result;
                        }
                        
                    }
                } else {
                    if (((addr_result & 0x08000000) >> 27) == 0) {
                        printf("[INFO][MONITOR] RECEIVED BSG_IO PACKET from tile y,x=%2d,%2d\n", mc_src_y_cord, mc_src_x_cord);
                        *load_addr_ptr = htoll(io_memory[(addr_result & 0x0003FFFF)]);
                    }
                }
                
                //uint32_t core_id = (addr_result & 0x00000FFF) >> 3;
/*                
                if ((addr_result>>12) == 0x3000){
                    printf("[CORE%0x PRT] %x\n", core_id, data_result);
                } else if ((addr_result>>12) == 0x3001){
                    printf("[CORE%0x PRT] %c\n", core_id, data_result);
                } else if ((addr_result>>12) == 0x3002){
                    if (data_result == 0){
                      printf("[CORE%0x FSH] PASS\n", core_id);
                    } else {
                      printf("[CORE%0x FSH] FAIL\n", core_id);
                    }
                    counter++;
                    if (counter == num_cores){
                        break;
                    }
                }
*/
/*
                if ((addr_result>>12) == 0x101){
                    printf("%c", data_result);
                    fflush(stdout);
                } else if ((addr_result>>12) == 0x102){
                    if (data_result == 0){
                      printf("[CORE%0x FSH] PASS\n", core_id);
                      printf("%0x\n", data_result);
                    } else {
                      printf("[CORE%0x FSH] FAIL\n", core_id);
                      printf("%0x\n", data_result);
                    }
                    fflush(stdout);
                    counter++;
                    if (counter == num_cores){
                        break;
                    }
                } else if ((addr_result>>12) == 0x100){
                    if (isEmpty(queue)) {
                        *load_addr_ptr = htoll((uint32_t)0xFFFFFFFF);
                        *load_addr_ptr = htoll((uint32_t)0xFFFFFFFF);
                    } else {
                        *load_addr_ptr = htoll((uint32_t)(dequeue(queue)));
                        *load_addr_ptr = htoll((uint32_t)0x00000000);
                    }
                }
*/
            }
        }
        
        term_exit();
        
		fflush(stdout);
	}
	if (munmap(map_base, MAP_SIZE) == -1)
		FATAL;
	close(fd);
	return 0;
}
