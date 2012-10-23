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
Your RL78G14 should have come with cloud connectivity pre-programmed, but if you need to re-apply the Exosite enabled demo back on your board, then you can follow the steps below.<br>
Choose one of the following options before step 8:<br>

Option A: Factory default binary file<br>
1) Download Renesas Flash Tool from<br> 
   http://am.renesas.com/support/downloads/download_results/C1000000-C9999999/tools/evaluation_rfp.jsp<br>
2) Download the binary file from https://github.com/exosite-garage/rl78g14_gs_cloud/downloads<br>
3) Make sure SW5.2 is OFF then plug in the USB cable to your PC<br>
4) Run the flash programmer -> select the Basic mode -> select RL78 -> in the table, type in R5F104PJ under filter to locate the target Micro-controller, and input a workspace name<br>
5) Select the RDK com port in Tools then click next for the rest of the setting -> browse the binary file in User/Data area and click Start to download<br>
6) Close the flash programmer -> disconnect the board -> switch SW5.2 ON<br>
7) Press SW 2 down and re-connect the board to PC<br>

Option B: Compile source code with IAR<br>
1) Install IAR Embedded Workbench for Renesas RL78 30-day evaluation<br>
   http://supp.iar.com/Download/SW/?item=EWRL78-EVAL<br>
   In License Wizard, choose “Register with IAR Systems to get an evaluation license to complete installation"<br>
2) Download the project repository on GitHub as a zip file<br>
3) Extract the project and open RDKRenesasG14 as workspace with IAR<br>
4) Make sure SW5.2 is OFF then plug in the USB cable to your PC<br>
5) Compile the project and download the program to the board from IAR<br>
6) Press SW 2 down and then run the program on IAR<br>
7) Stop program on IAR and unplug USB cable -> switch SW 5.2 ON<br>

Shared steps:<br>
8) Use a smart phone or a laptop that can select WiFi Access Point of your RL78 to connect to provision AP<br>
9) Open http://192.168.240.1/prov.html -> click Wireless and Network Configuration -> click Existing Network -> select the appropriate network to join -> input your pass phase and then save<br>
10) Press SW 1 down and reset your board<br>
11) Release Switch 1 when "Cloud Demo" shows on the LCD<br>
12) Register for a free account on https://renesas.exosite.com<br>
13) Add a new device with the MAC address shown on the GainSpan Wifi module at https://renesas.exosite.com<br>
14) If the module successfully associates with your WiFi Access Point and connects to the Exosite server, the device will exchange security keys and will be activated.<br>
15) When connected and activated, the device will send "ping", board temp and ADC1 values periodically. Verify this by checking the data sources on your https://renesas.exosite.com Portal<br>
16) When connected, the LEDs on the board can be turned on and off from the cloud by turning the LED ON/OFF Switch on your dashboard or modifying the "LED Control" command data source on your https://renesas.exosite.com Portal (or via the API)<br>

========================================
Release Info
========================================
----------------------------------------
Release 2012-10-01
----------------------------------------
--) initial release<br>


