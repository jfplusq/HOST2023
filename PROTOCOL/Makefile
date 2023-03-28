# Location of ARM toolchain
SDK_PATH = /home/research/Xilinx/SDK/2019.1
ARM_TOOLS_PATH = $(SDK_PATH)/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin

MKDIR_P = mkdir -p

# Compilation tools
CC = gcc
CC_ARM = $(ARM_TOOLS_PATH)/arm-linux-gnueabihf-gcc
CXX_ARM = $(ARM_TOOLS_PATH)/arm-linux-gnueabihf-g++

CFLAGS = -Wall -std=c99 -Wno-format-overflow 
CXXFLAGS = -Wall -Wno-format-overflow

# DATABASE
DEFINES = 

LIB_PATHS = 
INCLUDE_PATHS = -I.
LINK_FLAGS = -lm -lsqlite3 

LIB_PATHS_ARM = -LARM_LIBS
INCLUDE_PATHS_ARM = -I. -IARM_INCLUDES -IAES 
LINK_FLAGS_ARM = -lm 

# All output binaries
BIN_VRG = verifier_regeneration
BIN_DRG = device_regeneration.elf

TARGETS = $(BIN_VRG) $(BIN_DRG) 

# Object files required for each binary
USER_OBJS_VRG = utility.o common.o commonDB.o commonDB_RT.o verifier_regen_funcs.o verifier_regeneration.o
USER_OBJS_DRG = utility.o common.o device_common.o device_regen_funcs.o commonDB.o device_regeneration.o

# Build directory locations
OBJDIR_X86 = build/x86
OBJDIR_ARM_CXX = build/arm-g++
OBJDIR_ARM_CC = build/arm-gcc

# Append build directory paths to lists of object files
OBJS_VRG = $(patsubst %, $(OBJDIR_X86)/%, $(USER_OBJS_VRG))
OBJS_DRG = $(patsubst %, $(OBJDIR_ARM_CC)/%, $(USER_OBJS_DRG))

# Create the build directory automatically
$(shell $(MKDIR_P) $(OBJDIR_X86) $(OBJDIR_ARM_CC) $(OBJDIR_ARM_CXX))

# Default target
.PHONY: all
all: $(TARGETS)


$(BIN_DRG): $(OBJS_DRG)
	$(CC_ARM) $^ $(LIB_PATHS_ARM) $(LINK_FLAGS_ARM) -lpthread -lsqlite3 -o $@

# Output binaries
$(BIN_VRG): $(OBJS_VRG)
	$(CC) $(LIB_PATHS) $(LINK_FLAGS) -lpthread $^ -o $@ 

# x80 object files
$(OBJDIR_X86)/utility.o: utility.c utility.h
$(OBJDIR_X86)/common.o: common.c common.h
$(OBJDIR_X86)/verifier_common.o: verifier_common.c verifier_common.h common.h 

$(OBJDIR_X86)/commonDB.o: commonDB.c commonDB.h
$(OBJDIR_X86)/commonDB_RT.o: commonDB_RT.c commonDB_RT.h commonDB.h verifier_common.h common.h
$(OBJDIR_X86)/verifier_regen_funcs.o: verifier_regen_funcs.c commonDB.h verifier_regen_funcs.h verifier_common.h commonDB_RT.h common.h
$(OBJDIR_X86)/verifier_regeneration.o: verifier_regeneration.c commonDB.h verifier_regen_funcs.h verifier_common.h commonDB_RT.h common.h

$(OBJDIR_X86)/%.o:
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDE_PATHS) -c $< -o $@


# ARM C object files
$(OBJDIR_ARM_CC)/utility.o: utility.c utility.h
$(OBJDIR_ARM_CC)/common.o: common.c common.h

$(OBJDIR_ARM_CC)/commonDB.o: commonDB.c commonDB.h
$(OBJDIR_ARM_CC)/commonDB_RT.o: commonDB_RT.c commonDB_RT.h commonDB.h verifier_common.h common.h
$(OBJDIR_ARM_CC)/device_common.o: device_common.c device_common.h common.h device_regen_funcs.h commonDB.h device_hardware.h
$(OBJDIR_ARM_CC)/device_regen_funcs.o: device_regen_funcs.c device_regen_funcs.h device_common.h common.h device_hardware.h commonDB.h
$(OBJDIR_ARM_CC)/device_regeneration.o: device_regeneration.c device_regen_funcs.h device_common.h common.h device_hardware.h commonDB.h

$(OBJDIR_ARM_CC)/%.o:
	$(CC_ARM) $(CFLAGS) $(DEFINES) $(INCLUDE_PATHS_ARM) -c $< -o $@


# ARM C++ object files
$(OBJDIR_ARM_CXX)/utility.o: utility.c utility.h
$(OBJDIR_ARM_CXX)/common.o: common.c common.h

$(OBJDIR_ARM_CXX)/commonDB.o: commonDB.c commonDB.h
$(OBJDIR_ARM_CXX)/device_common.o: device_common.c device_common.h common.h device_regen_funcs.h commonDB.h device_hardware.h
$(OBJDIR_ARM_CXX)/device_regen_funcs.o: device_regen_funcs.c device_regen_funcs.h device_common.h common.h device_hardware.h
$(OBJDIR_ARM_CXX)/device_regeneration.o: device_regeneration.c device_regen_funcs.h device_common.h common.h device_hardware.h

$(OBJDIR_ARM_CXX)/%.o:
	$(CXX_ARM) $(CXXFLAGS) $(DEFINES) $(INCLUDE_PATHS_ARM) -c $< -o $@

# Utility
.PHONY: clean install

clean:
	-rm $(TARGETS)
	-rm -r build
