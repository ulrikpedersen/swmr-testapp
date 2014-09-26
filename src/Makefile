HDF5_SWMR=../hdf5swmr

INC := $(INC) -I$(HDF5_SWMR)/include/
CC=gcc
CFLAGS := $(CFLAGS) $(INC) -g
ODIR=obj

LIBS := -lhdf5 -llog4cxx -lz
LDFLAGS := $(LDFLAGS) -L$(HDF5_SWMR)/lib $(LIBS)
BINDIR=..

_DEPS = hdf5.h
DEPS = $(patsubst %,$(HDF5_SWMR)/include/%,$(_DEPS))

_WROBJ = swmr-testapp.o swmr-writer.o
WROBJ = $(patsubst %,$(ODIR)/%,$(_WROBJ))

_RDOBJ = swmr-testapp.o swmr-reader.o
RDOBJ = $(patsubst %,$(ODIR)/%,$(_RDOBJ))

all: $(BINDIR)/swmr-writer $(BINDIR)/swmr-reader

# Compile
$(ODIR)/%.o: %.cpp $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

# Link the writer application
$(BINDIR)/swmr-writer: $(WROBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

# Link the reader application
$(BINDIR)/swmr-reader: $(RDOBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

