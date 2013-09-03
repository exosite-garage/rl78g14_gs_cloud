========================================
About RL78G14 Exosite Cloud Demo
========================================
This project is an IAR Renesas RL78G14 project that using GainSpan wifi module GS1011M to demo Exosite cloud activatation, <br>
and cloud connectivity to send and receive data to/from the cloud by using Exosite's Cloud Data Platform.<br> 

The project functionality includes:<br>
1) New device activation and save cik to EEPROM.<br>
2) Period send data to draw triangle wave to Exosite cloud.<br>
3) Send adc value and temp to Exosite cloud.<br>
4) Read data source on Exosite cloud.<br>

License is BSD, Copyright 2012, Exosite LLC (see LICENSE file)

Tested and developed ith IAR for RL78 30-day evaluation 1.20.3.<br>

========================================
Quick Start
========================================
1) Install IAR Embedded Workbench for Renesas RL78 30-day evaluation<br>
http://supp.iar.com/Download/SW/?item=EWRL78-EVAL <br>
2).Add a new device with the MAC address shown on the GainSpan Wifi module at https://renesas.exosite.com
3) Download project repository as a zip file.<br>
4) Extract the project and open it with IAR.<br>
5) Confirm SW5.2 is OFF then plug in the USB cable to your PC<br>
6) Compile the project and download the program to the board<br>
7) Press Switch 2 down before running program.
8) Use a smart phone or a laptop that can select WiFi Access Point to connect to provision AP.<br>
9) Open http://192.168.240.1/prov.html, Select XXXX, input your SSID and pass phase then save.<br>
10) Stop program and press Switch 1 down before running program.<br>
11) Release Switch 1 when "Cloud Demo" shows on the LCD.<br>
12) If the module successfully associates with your WiFi Access Point and connects to the Exosite server, the device will exchange security keys and will be activated<br>
13) When connected and activated, the device will send "ping", board temp and ADC1 values periodically<br>
14) When connected, the LEDs on the board can be turned on and off from the cloud by modifying the "LED Control" command data source in your https://renesas.exosite.com Portal (or via the API).<br>
15) When activated and had provisioned, it'll auto boot on "Cloud Demo" mode<br>

========================================
Release Info
========================================
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Release 2013-07-29
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
--) improvement Exosite library in activation function
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Release 2013-05-03
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
--) implement Exosite library
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Release 2012-10-01
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
--) initial release<br>


