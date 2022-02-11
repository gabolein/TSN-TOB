IDIR = include
ODIR = obj
SDIR = src

CC = gcc
CFLAGS = -g -Wall -std=gnu11 -I$(IDIR) -DLOG_USE_COLOR -fcommon
TARGET = synch_broadcast

#bench: run "make <benchtype>_BENCH=1" to compile benchmark code
ifeq ($(LATENCY_BENCH),1)
	CFLAGS += -DLATENCY_BENCH
endif
ifeq ($(DELIVERY_BENCH),1)
	CFLAGS += -DDELIVERY_BENCH
endif

default: $(TARGET)
all: default

_DEPS = communication.h
_DEPS += interface.h
_DEPS += membership.h
_DEPS += node.h
_DEPS += service_time.h
_DEPS += log.h
_DEPS += failures.h
_DEPS += time_util.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o
_OBJ += communication.o
_OBJ += interface.o
_OBJ += membership.o
_OBJ += node.o
_OBJ += service_time.o
_OBJ += log.o 
_OBJ += failures.o
_OBJ += benchmark.o
_OBJ += time_util.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	-rm -f $(ODIR)/*.o
	-rm -f $(TARGET)
