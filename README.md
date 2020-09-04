# LoRaWAN_RF96
Converting cheap RF96 LoRa radio to LoRaWAN compatible device.

## Keywords
LoRa, LoRaWAN, DevEUI, LMIC, Raspberry PI, RF96.

## Summary
Cheap LoRa radio modems such as RF95, RF96 are not LoRa Alliance compatible by default
and are not supplied with IEEE approved device EUI number to use them in LoRaWan network.
So this project is only for testing purposes and to understand how message transmitting and 
receiving is working in LoRaWAN network. This app is using LMIC library originally provided by IBM.

Please acquire valid device EUI from your LoRaWAN provider or TTN and use it instead of generating your own.

This app shows in practice how to transmit and receive message in LoRaWAN network using 
very common platform such as Raspberry Pi and a common LoRa radio modem that is sold for 
controlling small robots and such. 

Getting these very affordable radio chips into country-wide network will expand the application range 
remarkably. LoRaWAN applications are very energy efficient and have much longer transmitting range than Bluetooth. 
Common applications could be in agriculture, construction, municipal sector, you name it.

## Components needed
 - Raspberry PI 3 or newer
 - LoRa RFM95 or such board 
 https://www.elecrow.com/wiki/index.php?title=Lora_RFM95_IOT_Board_for_RPI

## Installing
1. Install BCM2835 driver for LoRa modem 

```
 wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.52.tar.gz
 tar zxvf bcm2835*
 cd bcm2835*
 ./configure
 make
 sudo make check
 sudo make install
```

2. Set device EUI, join EUI and App key in lora source.
```
// Set DevEUI
static const u1_t PROGMEM DEVEUI[8] = { 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 };

// Join EUI (App EUI)
static const u1_t PROGMEM APPEUI[8] = { 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// App Key
static const u1_t PROGMEM APPKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12 };
```

3. Build and run this LoRa app
```
sudo make
sudo ./lora
``` 

## License
You can use most of my source for free, but some third party libraries are under LGPL.
Please refer to each third party library for more details.
