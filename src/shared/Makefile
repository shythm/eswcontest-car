CP_PATH = $(abspath ../..)/CrossCompiler
CROSS_COMPILE = $(CP_PATH)/gcc-linaro-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-

ARCH = arm
CC = $(CROSS_COMPILE)gcc

OUTPUT_NAME_CTRLBOARD = ctrlboard
REQUIRED_FILE_CTRLBOARD = ctrlboard.c ctrlboard-lib.c util.c
$(OUTPUT_NAME_CTRLBOARD) : $(REQUIRED_FILE_CTRLBOARD)
	$(CC) -o $(OUTPUT_NAME_CTRLBOARD) $(REQUIRED_FILE_CTRLBOARD)

OUTPUT_NAME_CARPSD = carpsd
REQUIRED_FILE_CARPSD = carpsd.c carpsd-lib.c util.c
$(OUTPUT_NAME_CARPSD) : $(REQUIRED_FILE_CARPSD)
	$(CC) -o $(OUTPUT_NAME_CARPSD) $(REQUIRED_FILE_CARPSD) -lm

clean:
	rm -rf *.o $(OUTPUT_NAME_CARPSD) $(OUTPUT_NAME_CTRLBOARD)