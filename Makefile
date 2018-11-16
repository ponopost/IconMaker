
TARGET = daICON
APPNAME = "icon maker"
APPID = "IMda"
DBTYPE = "DAcc"

OBJS = $(TARGET).o
LIBS = 

CC = m68k-palmos-coff-gcc

CFLAGS = -Wall -g -O2
CSFLAGS = $(CFLAGS) -S

PILRC = pilrc
OBJRES = m68k-palmos-coff-obj-res
BUILDPRC = build-prc

all: $(TARGET).prc

.c.s:
	$(CC) $(CSFLAGS) $<

.c.o:
	$(CC) $(CFLAGS) -c $<

$(TARGET).prc: code03e8.grc bin.res
	$(BUILDPRC) -t $(DBTYPE) $(TARGET).prc $(APPNAME) $(APPID) code03e8.grc *.bin

code03e8.grc: $(TARGET)
	$(OBJRES) $(TARGET)
	mv code0001.$(TARGET).grc $@
	rm *.$(TARGET).grc

bin.res: $(TARGET).rcp res.h
	rm -f *.bin
	$(PILRC) $(TARGET).rcp
	touch bin.res

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -nostartfiles $(OBJS) -o $(TARGET) $(LIBS)


clean:
	rm *.bin $(TARGET).o $(TARGET) code03e8.grc bin.res

