ROOT_DIR := $(notdir $(CURDIR))
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

HOST_MKIFS := mkifs

SUFFIXES := .build .ifs .raw

.PHONY: all clean

all: ifs-omapl137.raw 

clean: 
	$(RM_HOST) ifs-omapl137.raw

ifs-omapl137.raw: omapl137.build
	$(HOST_MKIFS) -vvvv -r../install $(MKIFSFLAGS) $^ $@
