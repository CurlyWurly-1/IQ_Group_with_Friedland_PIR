# IQ_Group_with_Friedland_PIR
This code is to be used in the Arduino IDE for programming a Raspberry Pi Pico W. 

Its purpose is to recognise 433 MHz signals from a Friedland (NOVA) wireless PIR device, and then transmit a separate 433 MHz signal to trigger a IQ Group Receiver - Please note the following: 
 - A pair of cheap 433mHz RX and TX modules is utilised (as pictured below).
 - An LDR and 10K resistor is used for Dark detection (if not used, the device will just work)
 - The Arduino IDE is used
 - The setup of the Arduino IDE for programming a PICO W is documented here https://randomnerdtutorials.com/programming-raspberry-pi-pico-w-arduino-ide/
 - This repro uses a Raspberry PICO W instead of an Arduino UNO, because when the two are compared, the PICO W is faster, smaller, cheaper and has Wifi capability
 - Instead of repeating the necessary setup info for the IQ Group receiver here, please refer to https://github.com/CurlyWurly-1/IQ-group-pir-wireless-arduino-TX

N.B. There are 2 programs - one is for a combined 433 MHz RX and TX device, the other is to pass a MQTT message (useful for Home assistant)


<img src="images/ard.jpg" alt="Ard_tx"/>

This is how an IQ Group receiver device looks like. When the correct 433 MHz signal is received, it can switch mains power on/off via its own interanl relay (e.g. to a floodlight)

<img src="images/P1140936.jpg" alt="IQ Group 240V Mains controller"/>

This is how a Friedland (NOVA) Wireless PIR looks like. Note that the signals transmitted by these wireless PIR sensors are not compatible with the IQ Group Receiver. This is why a "433 MHz translator" device is useful, as implemented by a PICO W device programmed with this software connectred to a cheap pair of 433 MHz RX and TX modules. 

<img src="images/Friedland_PIR_and_box.jpg" alt="Friedland NOVA Wireless PIR sensor"/>
