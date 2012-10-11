========================================
About RL78G14 Exosite Cloud Demo
========================================
This project is an IAR Renesas RL78G14 project that using GainSpan wifi module GS1011M to demo Exosite cloud activatation, <br>
and cloud connectivity to send and receive data to/from the cloud by using Exosite's Cloud Data Platform.<br> 

The project functionality includes:<br>
1) Activate new device and save cik to EEPROM.<br>
2) Send data periodically to draw triangle wave to Exosite cloud.<br>
3) Send adc value and temp to Exosite cloud.<br>
4) Read data source on Exosite cloud.<br>

License is BSD, Copyright 2012, Exosite LLC (see LICENSE file)

Tested and developed with IAR for RL78 30-day evaluation 1.20.3.<br>

========================================
Quick Start
========================================
1) Install IAR Embedded Workbench for Renesas RL78 30-day evaluation<br>
http://supp.iar.com/Download/SW/?item=EWRL78-EVAL <br>
In License Wizard, choose Register with IAR Systems to get an evaluation license to complete installation. <br>
2) Add a new device with the MAC address shown on the GainSpan Wifi module at https://renesas.exosite.com
3) Download the project repository on GitHub as a zip file.<br>
4) Extract the project and open ADKRenesasG14 as workspace with IAR.<br>
5) Make sure SW5.2 is OFF then plug in the USB cable to your PC.<br>
6) Compile the project and download the program to the board from IAR.<br>
7) Press SW 2 down and then run the program on IAR.<br>
8) Use a smart phone or a laptop that can select WiFi Access Point of your RL78 to connect to provision AP.<br>
9) Open http://192.168.240.1/prov.html, click Wireless and Network Configuration, click Existing Network, select the appropriate network to join, input your pass phase and then save.<br>
10) Stop program on IAR and unplug USB cable, put SW 5.2 ON.<br>
11) Press SW 1 down and plug USB cable at the same time.<br>
12) Release Switch 1 when "Cloud Demo" shows on the LCD.<br>
13) If the module successfully associates with your WiFi Access Point and connects to the Exosite server, the device will exchange security keys and will be activated.<br>
14) When connected and activated, the device will send "ping", board temp and ADC1 values periodically. Verify this by checking the data sources on https://renesas.exosite.com Portal.<br>
15) When connected, the LEDs on the board can be turned on and off from the cloud by turning the LED ON/OFF Switch on your dashboard or modifying the "LED Control" command data source on your https://renesas.exosite.com Portal (or via the API).<br>

========================================
Release Info
========================================
----------------------------------------
Release 2012-10-01
----------------------------------------
--) initial release<br>


