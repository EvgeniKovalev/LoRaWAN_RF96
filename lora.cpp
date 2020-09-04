#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <string>
#include <iostream>
#include <inttypes.h>

#include <lmic.h>
#include <hal/hal.h>
#include <chrono>

using namespace std;

// Set DevEUI
static const u1_t PROGMEM DEVEUI[8] = { 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 };
void os_getDevEui(u1_t* buf) { memcpy_P(buf, DEVEUI, 8); }

// Join EUI (App EUI)
static const u1_t PROGMEM APPEUI[8] = { 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getArtEui(u1_t* buf) { memcpy_P(buf, APPEUI, 8); }

// App Key
static const u1_t PROGMEM APPKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12 };
void os_getDevKey(u1_t* buf) { memcpy_P(buf, APPKEY, 16); }

// Communication interval in seconds
int COMM_INTERVAL = 5;

// 4 int16_t values 
static osjob_t sendjob;

//Flag for Ctrl-C
volatile sig_atomic_t force_exit = 0;

// Raspberry PI Lora hat GPIO config
#define RF_LED_PIN RPI_V2_GPIO_P1_16 // Led on GPIO23 so P1 connector pin #16
#define RF_CS_PIN  RPI_V2_GPIO_P1_26 // Slave Select on CE0 so P1 connector pin #26
#define RF_IRQ_PIN RPI_V2_GPIO_P1_22 // IRQ on GPIO25 so P1 connector pin #22
#define RF_RST_PIN RPI_V2_GPIO_P1_15 // RST on GPIO22 so P1 connector pin #15

// Pin mapping
const lmic_pinmap lmic_pins = {
		.nss = RF_CS_PIN,
		.rxtx = LMIC_UNUSED_PIN,
		.rst = RF_RST_PIN,
		.dio = {LMIC_UNUSED_PIN, LMIC_UNUSED_PIN, LMIC_UNUSED_PIN},
};

/**
 * Waits for Ctl-C
 * */
void sig_handler(int sig)
{
	printf("\nBreak received, exiting!\n");
	force_exit = true;
}

/**
 * Pings LoRaWAN with short message
 * **/
void Ping(osjob_t* j)
{
	if (LMIC.opmode & OP_TXRXPEND)
	{
		printf("Previous job is still sending.. waiting for the new round \n");
	}
	else
	{
		int numberOfBytesInTable = 1;
		uint8_t pingData[numberOfBytesInTable * sizeof(int16_t)];
		int t = 1;
		pingData[0] = t >> 8; 		// MSB
		pingData[1] = t & 0xFF; 	// LSB
		LMIC_setTxData2(1, pingData, sizeof(pingData), 0);
	}
}

/**
 *  Callback form LMIC library
 * */
void onEvent(ev_t ev)
{
	char strTime[16];
	getSystemTime(strTime, sizeof(strTime));
	printf("%s: ", strTime);

	switch (ev) {
	case EV_JOINED:
		printf("EV_JOINED\n");
		LMIC_setLinkCheckMode(0);
		break;

	case EV_TXCOMPLETE:
		printf("EV_TXCOMPLETE (incl. waiting RX)\n");


		if (LMIC.txrxFlags & TXRX_ACK)
		{
			printf("\n\n%s ===== Received ACK ===== \n", strTime);
		}

		if (LMIC.dataLen)
		{
			PurgeFile();
			printf("\n\n\n%s ****************** Received %d bytes of payload ****************** \n\n\n\n\n", strTime, LMIC.dataLen);

			//Printing what was received
			for (int i = 0; i < LMIC.dataLen; i++)
			{
				printf("%X", LMIC.frame[LMIC.dataBeg + i]);
			}
			printf("\n\n");
		}
		else
		{
			printf("\n\n --- NOTHING ARRIVED :(\n");
		}
		// Schedule next transmission
		os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(COMM_INTERVAL), Ping);

		break;
	case EV_SCAN_TIMEOUT: 	printf("EV_SCAN_TIMEOUT\n");	break;
	case EV_BEACON_FOUND:		printf("EV_BEACON_FOUND\n");	break;
	case EV_BEACON_MISSED:	printf("EV_BEACON_MISSED\n");	break;
	case EV_BEACON_TRACKED:	printf("EV_BEACON_TRACKED\n"); break;
	case EV_JOINING:				printf("EV_JOINING\n"); 			break;
	case EV_RFU1: 					printf("EV_RFU1\n");					break;
	case EV_JOIN_FAILED:		printf("EV_JOIN_FAILED\n");		break;
	case EV_REJOIN_FAILED:	printf("EV_REJOIN_FAILED\n"); break;
	case EV_LOST_TSYNC:			printf("EV_LOST_TSYNC\n");		break;
	case EV_RESET:					printf("EV_RESET\n");					break;
	case EV_RXCOMPLETE:			printf("EV_RXCOMPLETE\n");		break;
	case EV_LINK_DEAD:			printf("EV_LINK_DEAD\n");			break;
	case EV_LINK_ALIVE:			printf("EV_LINK_ALIVE\n");		break;
	default:
		printf("Unknown event\n");
		break;
	}
}
/**
 * Main function
 **/
int main()
{
	// caught CTRL-C to do clean-up
	signal(SIGINT, sig_handler);

	// Display Hardware RFM95 configuration
	printKeys();

	if (!bcm2835_init())
	{
		printf("bcm2835_init failed.\n");
	}
	else if (!bcm2835_spi_begin())
	{
		printf("bcm2835_spi_begin failed\n");
	}

	// Light on LED
	pinMode(RF_LED_PIN, OUTPUT);
	// digitalWrite(RF_LED_PIN, HIGH);

	if (!bcm2835_i2c_begin())
	{
		fprintf(stderr, "bcm2835_i2c_begin() failed\n");
		return 1;
	}

	// Set I2C speed to 100KHz
	bcm2835_i2c_set_baudrate(100000);

	// LMIC init
	os_init();

	// Reset the state. Session and pending data transfers will be discarded.
	LMIC_reset();

	// Start job (sending automatically starts OTAA too)
	// Then on transmit will reset it's own send 
	Ping(&sendjob);
	;

	// Main loop until CTRL-C
	while (!force_exit) {
		os_runloop_once();
		// cooldown processor, otherwise runs on 99%
		usleep(1000);
	}

	// Light off on board LED
	digitalWrite(RF_LED_PIN, LOW);

	// Release I2C and BCM2835
	bcm2835_i2c_end();
	bcm2835_close();
	return 0;
}
