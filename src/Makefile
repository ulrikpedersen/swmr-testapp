HDF5_SWMR=../hdf5swmr

INC := $(INC) -I$(HDF5_SWMR)/include/
CC=gcc
CFLAGS := $(CFLAGS) $(INC) -g
ODIR=obj

LIBS := -lhdf5 -llog4cxx -lz -lrt -lboost_program_options
LDFLAGS := $(LDFLAGS) -L$(HDF5_SWMR)/lib $(LIBS)
BINDIR=..

_DEPS = hdf5.h
DEPS = $(patsubst %,$(HDF5_SWMR)/include/%,$(_DEPS))

_OBJ = timestamp.o frame.o swmr-reader.o swmr-writer.o swmr-demo.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: $(BINDIR)/swmr

# Compile
$(ODIR)/%.o: %.cpp $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

# Link the main application
$(BINDIR)/swmr: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

