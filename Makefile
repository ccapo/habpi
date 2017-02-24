ifneq ($V,1)
Q ?= @
endif

SRCDIR = src
INCDIR = include
CC = gcc

#CFLAGS	= -O0 -g -Wall -Winline -I$(INCDIR) -I/usr/include -I/usr/local/include -pipe
CFLAGS	= -O3 -s -Wall -Winline -I$(INCDIR) -I/usr/include -I/usr/local/include -pipe
LFLAGS = -L/usr/lib -L/usr/local/lib -lwiringPi -lwiringPiDev -lpthread -lsqlite3 -lm

.SUFFIXES: .o .h .c

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$Q echo [Compile] $<
	$Q $(CC) $(CFLAGS) -c $< -o $@

SOURCES = $(wildcard $(SRCDIR)/*.c)

HEADERS = $(wildcard $(INCDIR)/*.h)

OBJECTS = $(SOURCES:.c=.o)

all: hab

hab: $(HEADERS) $(OBJECTS)
	$Q echo [Link] $@
	$Q $(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LFLAGS)
	$Q rm -f $(OBJECTS)

clean:
	$Q echo "[Clean]"
	$Q rm -f $(OBJECTS) hab log/*.log

depend:
	makedepend -Y $(SOURCES)

# DO NOT DELETE
