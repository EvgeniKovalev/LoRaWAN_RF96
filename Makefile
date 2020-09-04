# Makefile
# Requires bcm2835 library to be already installed
#	 wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.52.tar.gz
#	 tar zxvf bcm2835*
#	 cd bcm2835*
#	 ./configure
#	 make
#	 sudo make check
#	 sudo make install

CC       = g++
CFLAGS   = -std=c++11 -Wall -DBCM2835_NO_DELAY_COMPATIBILITY -DRASPBERRY_PI -D__BASEFILE__=\"$*\"
LIBS     = -lbcm2835
BASE = lib
INCLUDE  = -I$(BASE) 

all: lora

raspi.o: $(BASE)/raspi/raspi.cpp
				$(CC) $(CFLAGS) -c $(BASE)/raspi/raspi.cpp $(INCLUDE)

radio.o: $(BASE)/lmic/radio.c
				$(CC) $(CFLAGS) -c $(BASE)/lmic/radio.c $(INCLUDE)

oslmic.o: $(BASE)/lmic/oslmic.c
				$(CC) $(CFLAGS) -c $(BASE)/lmic/oslmic.c $(INCLUDE)

lmic.o: $(BASE)/lmic/lmic.c
				$(CC) $(CFLAGS) -c $(BASE)/lmic/lmic.c $(INCLUDE)

hal.o: $(BASE)/hal/hal.cpp
				$(CC) $(CFLAGS) -c $(BASE)/hal/hal.cpp $(INCLUDE)

aes.o: $(BASE)/aes/lmic.c
				$(CC) $(CFLAGS) -c $(BASE)/aes/lmic.c $(INCLUDE) -o aes.o

lora.o: lora.cpp
				$(CC) $(CFLAGS) -c $(INCLUDE) $<

lora: lora.o raspi.o radio.o oslmic.o lmic.o hal.o aes.o
				$(CC) $^ $(LIBS) -o lora

clean:
				rm -rf *.o lora
