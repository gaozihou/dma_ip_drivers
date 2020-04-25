CC ?= gcc

all: reg_rw dma_to_device dma_from_device performance nbf nbf_tapeout_rc0 nbf_mc_tapeout_rc3

dma_to_device: dma_to_device.o
	$(CC) -lrt -o $@ $< -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D_LARGE_FILE_SOURCE

dma_from_device: dma_from_device.o
	$(CC) -lrt -o $@ $< -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D_LARGE_FILE_SOURCE

performance: performance.o
	$(CC) -o $@ $< -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D_LARGE_FILE_SOURCE

reg_rw: reg_rw.o
	$(CC) -o $@ $<
    
nbf: nbf.o
	$(CC) -pthread -o $@ $<

nbf_tapeout_rc0: nbf_tapeout_rc0.o
	$(CC) -pthread -o $@ $<

nbf_mc_tapeout_rc3: nbf_mc_tapeout_rc3.o
	$(CC) -pthread -o $@ $<

%.o: %.c
	$(CC) -c -std=c99 -o $@ $< -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D_LARGE_FILE_SOURCE

clean:
	rm -rf reg_rw *.o *.bin dma_to_device dma_from_device performance

