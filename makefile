CC = arm-none-eabi-gcc
C++ = arm-none-eabi-g++
OBJCOPY = arm-none-eabi-objcopy
LD = arm-none-eabi-ld
DRIVERS = ./drivers
COMPILE_FLAGS = -mcpu=cortex-m4 \
	-mthumb -O2 -ffunction-sections \
	-fdata-sections \
    -Wall -Wextra -mfloat-abi=hard -mfpu=fpv4-sp-d16 
LINK_FLAGS = -T mem.ld

all: blink.bin pc.elf
blink.bin: blink.elf
	$(OBJCOPY) -O binary out_dir/blink.elf blink.bin

blink.elf: out_dir
	$(C++) $(COMPILE_FLAGS) src/main.cpp -c -o out_dir/main.o 
	$(CC) $(COMPILE_FLAGS) startup/startup.c -c -o out_dir/startup.o \

	$(LD) $(LINK_FLAGS) out_dir/main.o out_dir/startup.o -o out_dir/blink.elf

out_dir:
	mkdir out_dir

pc.elf: 
	cargo build --release
	mv ./target/release/pc ./pc.elf


test:
	dfu-util -a 0 -D blink.bin --dfuse-address 0x08000000
test_pc:
	printf "sudo required to get access to your serial ports (check pc/src/main.rs)\n\
	it's not malware\n"
	
	sudo ./pc.elf
clean:
	rm -rf blink.bin blink.elf out_dir ./pc.elf
c_flash:
	dfu-util -a 0 -e