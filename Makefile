CPP     = g++
SRCDIR  = src
INCDIR  = include
INCDIR2 = /usr/include
LIBDIR  = /usr/lib/arm-linux-gnueabihf
#CFLAGS  = -O2 -s -w -std=gnu++11
CFLAGS  = -O0 -g -std=gnu++11
LFLAGS  = -I$(INCDIR) -I$(INCDIR2) -L$(LIBDIR) -lgps -lsqlite3 -lwiringPi -pthread -lm

.SUFFIXES: .o .h .cpp

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CPP) $(CFLAGS) -I$(INCDIR) -I$(INCDIR2) -c $< -o $@

HEADERS = $(wildcard $(INCDIR)/*.h $(INCDIR2)/sqlite*.h)

SRCS = $(wildcard $(SRCDIR)/*.cpp)

OBJS = $(SRCS:.cpp=.o)

I2COBJS = $(SRCDIR)/i2c_bus.o

SPIOBJS = $(SRCDIR)/spi_bus.o

AHRSOBJS = $(SRCDIR)/Madgwick.o $(SRCDIR)/Mahony.o $(SRCDIR)/FXOS8700.o $(SRCDIR)/FXAS21002C.o

CALOBJS = $(AHRSOBJS) $(SRCDIR)/calibration/AHRS_Calibration.o

FUSOBJS = $(AHRSOBJS) $(SRCDIR)/test/AHRS_Fusion.o

DHTOBJS = $(SRCDIR)/DHT.o $(SRCDIR)/DHT_U.o $(SRCDIR)/test/DHT_U_test.o

MPLOBJS = $(SRCDIR)/MPL3115A2.o $(SRCDIR)/MPL3115A2_U.o $(SRCDIR)/test/MPL3115A2_U_test.o

GPSOBJS = $(SRCDIR)/GPS.o $(SRCDIR)/test/GPSMM_test.o

IMGOBJS = $(SRCDIR)/test/Image_test.o

SEROBJS = $(SRCDIR)/Serializer.o $(SRCDIR)/test/Serializer_test.o

SQLOBJS = $(SRCDIR)/test/Sqlite3_test.o

#all: AHRS_Calibration AHRS_Fusion DHT_U_test MPL3115A2_U_test GPSMM_test Image_test Serializer_test Sqlite3_test clean_objects
all: HABPi

AHRS_Calibration: $(HEADERS) $(CALOBJS)
	@$(CPP) $(CFLAGS) $(CALOBJS) -o $@ $(LFLAGS)
	@echo "AHRS_Calibration compiled successfully"

AHRS_Fusion: $(HEADERS) $(FUSOBJS)
	@$(CPP) $(CFLAGS) $(FUSOBJS) -o $@ $(LFLAGS)
	@echo "AHRS_Fusion compiled successfully"

DHT_U_test: $(HEADERS) $(DHTOBJS)
	@$(CPP) $(CFLAGS) $(DHTOBJS) -o $@ $(LFLAGS)
	@echo "DHT_U_test compiled successfully"

MPL3115A2_U_test: $(HEADERS) $(MPLOBJS)
	@$(CPP) $(CFLAGS) $(MPLOBJS) -o $@ $(LFLAGS)
	@echo "MPL3115A2_U_test compiled successfully"

GPSMM_test: $(GPSOBJS)
	@$(CPP) $(CFLAGS) $(GPSOBJS) -o $@ $(LFLAGS)
	@echo "GPSMM_test compiled successfully"

Image_test: $(HEADERS) $(IMGOBJS)
	@$(CPP) $(CFLAGS) $(IMGOBJS) -o $@ $(LFLAGS)
	@echo "Image_test compiled successfully"

Serializer_test: $(HEADERS) $(SEROBJS)
	@$(CPP) $(CFLAGS) $(SEROBJS) -o $@ $(LFLAGS)
	@echo "Serializer_test compiled successfully"

Sqlite3_test: $(HEADERS) $(SQLOBJS)
	@$(CPP) $(CFLAGS) $(SQLOBJS) -o $@ $(LFLAGS)
	@echo "Sqlite3_test compiled successfully"

HABPi: $(HEADERS) $(OBJS)
	@$(CPP) $(CFLAGS) $(OBJS) -o $@ $(LFLAGS)
	@echo "HABPi compiled successfully"

clean_objects:
	@rm -f $(SRCDIR)/*.o
	@rm -f $(SRCDIR)/test/*.o

clean: clean_objects
	@rm -f AHRS_Calibration
	@rm -f AHRS_Fusion
	@rm -f DHT_U_test
	@rm -f MPL3115A2_U_test
	@rm -f GPSMM_test
	@rm -f Image_test
	@rm -f Serializer_test
	@rm -f Sqlite3_test
	@rm -f HABPi
