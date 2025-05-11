# IQ_Group_with_Nova_PIR
This code is to be used in the Arduino IDE for programming a Raspberry Pi Pico W. 

Its purpose is to recognise 433 MHz signals from a NOVA wireless PIR device, and then transmit a separate 433 MHz signal to trigger a IQ Group Receiver - Please note the following: 
 - A pair of cheap 433mHz RX and TX modules is utilised (as pictured below). 
 - The Arduino IDE is used
 - The setup of the Arduino IDE for programming a PICO W is documented here https://randomnerdtutorials.com/programming-raspberry-pi-pico-w-arduino-ide/
 - This repro uses a Raspberry PICO W instead of an Arduino UNO, because when the two are compared, the PICO W is faster, smaller, cheaper and has Wifi capability
 - Instead of repeating the necessary setup info for the IQ Group receiver here, please refer to https://github.com/CurlyWurly-1/IQ-group-pir-wireless-arduino-TX

<img src="images/ard.jpg" alt="Ard_tx"/>

This is how an IQ Group receever looks - When the correct 433 MHz signal is received, it can switch mains power on/off via its own interanl relay (e.g. to a floodlight)

<img src="images/P1140936.jpg" alt="IQ Group 240V Mains controller"/>

This is how a NOVA Wireless PIR looks like. Note that the signals transmitted by these wireless PIR sensors are not compatible with the IQ Group Receiver. This is why the "translator" device as described here, is needed.   
