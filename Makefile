APP = snowboard

include $(SDK_DIR)/Makefile.defs

OBJS = $(ASSETS).gen.o main.o
ASSETDEPS += *.png $(ASSETS).lua
CCFLAGS += -DCUBE_ALLOCATION=12

include $(SDK_DIR)/Makefile.rules
