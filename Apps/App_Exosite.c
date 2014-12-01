/*****************************************************************************
*
*  App_Exosite.c - Exosite application for Custom Demo.
*  Copyright (C) 2012 Exosite LLC
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <drv/Glyph/lcd.h>
#include <system/mstimer.h>
#include "led.h"
#include "NVSettings.h"
#include <sensors/Temperature.h>
#include <sensors/Potentiometer.h>
#include <exosite/exosite_hal.h>
#include <exosite/exosite_meta.h>
#include <exosite/exosite.h>
#include <inc/common.h>

// Globals:
NVSettings_t GNV_Setting;

// local defines
#define ExositeAppVersion                   "   v1.04   "
#define SHOW_VERSION
#define WRITE_INTERVAL 5
#define EXO_BUFFER_SIZE 200
char exo_buffer[EXO_BUFFER_SIZE];
char ping = 0;
int16_t G_adc_int[2] = { 0, 0 };
char G_temp_int[2] = { 0, 0 };

// external defines

// external funsions
extern ATLIBGS_MSG_ID_E WIFI_init(int16_t showMessage);
extern ATLIBGS_MSG_ID_E WIFI_Associate(void);


/*****************************************************************************
*
*  TemperatureReading
*
*  \param  None
*
*  \return None
*
*  \brief  Takes a reading of temperature and shows it on the LCD display
*
*****************************************************************************/
void TemperatureReading(void)
{
  char lcd_buffer[20];

  // Temperature sensor reading
  int16_t temp;
  temp = Temperature_Get()>>3;
  // Get the temperature and show it on the LCD
  G_temp_int[0] = (int16_t)temp / 16 - 2;
  G_temp_int[1] = (int16_t)((temp & 0x000F) * 10) / 16;

  /* Display the contents of lcd_buffer onto the debug LCD */
  sprintf((char *)lcd_buffer, "TEMP: %d.%d C", G_temp_int[0], G_temp_int[1]);
  DisplayLCD(LCD_LINE3, (const uint8_t *)lcd_buffer);
}


/*****************************************************************************
*
*  PotentiometerReading
*
*  \param  None
*
*  \return None
*
*  \brief  Takes a reading of potentiometer and shows it on the LCD display
*
*****************************************************************************/
void PotentiometerReading(void)
{
  char lcd_buffer[20];

  // Temperature sensor reading
  int32_t percent;
  percent = Potentiometer_Get();
  G_adc_int[0] = (int16_t)(percent / 10);
  G_adc_int[1] = (int16_t)(percent % 10);

  sprintf((char *)lcd_buffer, " POT: %d.%d ", G_adc_int[0], G_adc_int[1]);
  /* Display the contents of lcd_buffer onto the debug LCD */
  DisplayLCD(LCD_LINE4, (const uint8_t *)lcd_buffer);
}


/*****************************************************************************
*
*  RSSIReading
*
*  \param  None
*
*  \return None
*
*  \brief  Takes a reading of Wifi RSSI level and shows it on the LCD display
*
*****************************************************************************/
void RSSIReading(void)
{
  int16_t rssi;
  char line[20];
  int rssiFound = 0;

  if (AtLibGs_IsNodeAssociated()) {
    if (AtLibGs_GetRssi() == ATLIBGS_MSG_ID_OK) {
      if (AtLibGs_ParseRssiResponse(&rssi)) {
        sprintf(line, "RSSI: %d", rssi);
        DisplayLCD(LCD_LINE5, (const uint8_t *)line);
        rssiFound = 1;
      }
    }
  }

  if (!rssiFound) {
    DisplayLCD(LCD_LINE5, "RSSI: ----");
  }
}


/*****************************************************************************
*
*  UpdateReadings
*
*  \param  None
*
*  \return None
*
*  \brief  Takes a reading of temperature and potentiometer and shows
*          on the LCD display
*
*****************************************************************************/
void UpdateReadings(void)
{
  TemperatureReading();
  PotentiometerReading();
  DisplayLCD(LCD_LINE7, "");
  DisplayLCD(LCD_LINE8, "");
}


/*****************************************************************************
*
*  show_status
*
*  \param  None
*
*  \return None
*
*  \brief  Shows the status message on the LCD display
*
*****************************************************************************/
void
show_status(void)
{
  int code = Exosite_StatusCode();

  DisplayLCD(LCD_LINE6, "  Exosite  ");

  if (code == EXO_STATUS_BAD_TCP) {
    DisplayLCD(LCD_LINE7, "Connecting ");
    DisplayLCD(LCD_LINE8, "   Retry   ");
  } else if (code == EXO_STATUS_BAD_UUID) {
    DisplayLCD(LCD_LINE7, "  Bad UUID ");
  } else if (code == EXO_STATUS_BAD_VENDOR) {
    DisplayLCD(LCD_LINE7, " Incorrect ");
    DisplayLCD(LCD_LINE8, "Vendor Name");
  } else if (code == EXO_STATUS_BAD_MODEL) {
    DisplayLCD(LCD_LINE7, " Incorrect ");
    DisplayLCD(LCD_LINE8, " Model Name");
  } else if (code == EXO_STATUS_BAD_SN) {
    DisplayLCD(LCD_LINE7, " Add Device");
    DisplayLCD(LCD_LINE8, "  to Portal");
  } else if (code == EXO_STATUS_BAD_CIK) {
    DisplayLCD(LCD_LINE7, "CIK Invalid");
    MSTimerDelay(1000);
    DisplayLCD(LCD_LINE7, "  Re-enable");
    DisplayLCD(LCD_LINE8, "   Device  ");
  } else if (code == EXO_STATUS_CONFLICT) {
    DisplayLCD(LCD_LINE7, "  Re-enable");
    DisplayLCD(LCD_LINE8, "   Device  ");
  } else if (code == EXO_STATUS_NOAUTH) {
    DisplayLCD(LCD_LINE7, "CIK Invalid");
  }

  return;
}


/*****************************************************************************
*
*  ReportReadings
*
*  \param  None
*
*  \return None
*
*  \brief  Reports the sensor and customization values to Exosite cloud
*
*****************************************************************************/
void ReportReadings(void)
{
  static char content[256];

#ifdef HOST_APP_TCP_DEBUG
  if (updateError) 
  {
    sprintf(content, "temp=%d.%d&adc1=%d.%d&ping=%d&ect=%d\r\n",
                     G_temp_int[0],G_temp_int[1], G_adc_int[0], G_adc_int[1],
                     ping,parsererror);
    updateError = 0;
  } else {
    sprintf(content, "temp=%d.%d&adc1=%d.%d&ping=%d\r\n",
                     G_temp_int[0],G_temp_int[1], G_adc_int[0], G_adc_int[1],
                     ping);
  }
#else
  sprintf(content, "temp=%d.%d&adc1=%d.%d&ping=%d\r\n",
                   G_temp_int[0],G_temp_int[1], G_adc_int[0], G_adc_int[1],
                   ping);
#endif
  ping++;
  if (ping >= 100)
    ping = 0;
  DisplayLCD(LCD_LINE6, "  Exosite  ");
  DisplayLCD(LCD_LINE7, "   Write   ");
  if (Exosite_Write(content, strlen(content)))
  {
    DisplayLCD(LCD_LINE8, "     OK    ");
  }

  return;
}


/*****************************************************************************
*
*  ReadCloudCommands
*
*  \param  None
*
*  \return None
*
*  \brief  Reads the commands from Exosite cloud
*
*****************************************************************************/
void ReadCloudCommands(void)
{
  char * pbuf = exo_buffer;
  DisplayLCD(LCD_LINE6, "  Exosite  ");
  DisplayLCD(LCD_LINE7, "    Read   ");
  if (Exosite_Read("led_ctrl", pbuf, EXO_BUFFER_SIZE)) {
    DisplayLCD(LCD_LINE8, "     OK    ");
    if (!strncmp(pbuf, "0", 1)) 
      led_all_off();
    else if (!strncmp(pbuf, "1", 1)) 
      led_all_on();
  }
  else show_status();
  MSTimerDelay(500);

  return;
}


/*****************************************************************************
*
*  checkWiFiConnected
*
*  \param  None
*
*  \return TRUE if connected, FALSE if not
*
*  \brief  Checks to see that WiFi is still connected
*
*****************************************************************************/
unsigned char
checkWiFiConnected(int reinit)
{
  unsigned char status = AtLibGs_IsNodeAssociated();

  if (!status)
  {
    if (reinit)
    {
      WIFI_init(1);
    }
    WIFI_Associate();
    status = AtLibGs_IsNodeAssociated();
  }
  RSSIReading();

  return status;
}


/*****************************************************************************
*
*  App_Exosite
*
*  \param  None
*
*  \return None
*
*  \brief  Takse a reading of temperature and potentiometer and sends to
*          Exosite cloud using a TCP connection
*
*****************************************************************************/
void App_Exosite(void)
{
  int loop_time = 1000;
  unsigned char loopCount = 0;
  int wifi_init = 0;
  int badcik = 1;
  static const uint8_t geoCert[] = { 0x30, 0x82, 0x03, 0x54, 0x30, 0x82, 0x02, 0x3c, 0xa0, 0x03,
                                     0x02, 0x01, 0x02, 0x02, 0x03, 0x02, 0x34, 0x56, 0x30, 0x0d,
                                     0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01,
                                     0x05, 0x05, 0x00, 0x30, 0x42, 0x31, 0x0b, 0x30, 0x09, 0x06,
                                     0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x16,
                                     0x30, 0x14, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x0d, 0x47,
                                     0x65, 0x6f, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x49, 0x6e,
                                     0x63, 0x2e, 0x31, 0x1b, 0x30, 0x19, 0x06, 0x03, 0x55, 0x04,
                                     0x03, 0x13, 0x12, 0x47, 0x65, 0x6f, 0x54, 0x72, 0x75, 0x73,
                                     0x74, 0x20, 0x47, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x20, 0x43,
                                     0x41, 0x30, 0x1e, 0x17, 0x0d, 0x30, 0x32, 0x30, 0x35, 0x32,
                                     0x31, 0x30, 0x34, 0x30, 0x30, 0x30, 0x30, 0x5a, 0x17, 0x0d,
                                     0x32, 0x32, 0x30, 0x35, 0x32, 0x31, 0x30, 0x34, 0x30, 0x30,
                                     0x30, 0x30, 0x5a, 0x30, 0x42, 0x31, 0x0b, 0x30, 0x09, 0x06,
                                     0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x16,
                                     0x30, 0x14, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x0d, 0x47,
                                     0x65, 0x6f, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x49, 0x6e,
                                     0x63, 0x2e, 0x31, 0x1b, 0x30, 0x19, 0x06, 0x03, 0x55, 0x04,
                                     0x03, 0x13, 0x12, 0x47, 0x65, 0x6f, 0x54, 0x72, 0x75, 0x73,
                                     0x74, 0x20, 0x47, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x20, 0x43,
                                     0x41, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a,
                                     0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00,
                                     0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82, 0x01, 0x0a, 0x02,
                                     0x82, 0x01, 0x01, 0x00, 0xda, 0xcc, 0x18, 0x63, 0x30, 0xfd,
                                     0xf4, 0x17, 0x23, 0x1a, 0x56, 0x7e, 0x5b, 0xdf, 0x3c, 0x6c,
                                     0x38, 0xe4, 0x71, 0xb7, 0x78, 0x91, 0xd4, 0xbc, 0xa1, 0xd8,
                                     0x4c, 0xf8, 0xa8, 0x43, 0xb6, 0x03, 0xe9, 0x4d, 0x21, 0x07,
                                     0x08, 0x88, 0xda, 0x58, 0x2f, 0x66, 0x39, 0x29, 0xbd, 0x05,
                                     0x78, 0x8b, 0x9d, 0x38, 0xe8, 0x05, 0xb7, 0x6a, 0x7e, 0x71,
                                     0xa4, 0xe6, 0xc4, 0x60, 0xa6, 0xb0, 0xef, 0x80, 0xe4, 0x89,
                                     0x28, 0x0f, 0x9e, 0x25, 0xd6, 0xed, 0x83, 0xf3, 0xad, 0xa6,
                                     0x91, 0xc7, 0x98, 0xc9, 0x42, 0x18, 0x35, 0x14, 0x9d, 0xad,
                                     0x98, 0x46, 0x92, 0x2e, 0x4f, 0xca, 0xf1, 0x87, 0x43, 0xc1,
                                     0x16, 0x95, 0x57, 0x2d, 0x50, 0xef, 0x89, 0x2d, 0x80, 0x7a,
                                     0x57, 0xad, 0xf2, 0xee, 0x5f, 0x6b, 0xd2, 0x00, 0x8d, 0xb9,
                                     0x14, 0xf8, 0x14, 0x15, 0x35, 0xd9, 0xc0, 0x46, 0xa3, 0x7b,
                                     0x72, 0xc8, 0x91, 0xbf, 0xc9, 0x55, 0x2b, 0xcd, 0xd0, 0x97,
                                     0x3e, 0x9c, 0x26, 0x64, 0xcc, 0xdf, 0xce, 0x83, 0x19, 0x71,
                                     0xca, 0x4e, 0xe6, 0xd4, 0xd5, 0x7b, 0xa9, 0x19, 0xcd, 0x55,
                                     0xde, 0xc8, 0xec, 0xd2, 0x5e, 0x38, 0x53, 0xe5, 0x5c, 0x4f,
                                     0x8c, 0x2d, 0xfe, 0x50, 0x23, 0x36, 0xfc, 0x66, 0xe6, 0xcb,
                                     0x8e, 0xa4, 0x39, 0x19, 0x00, 0xb7, 0x95, 0x02, 0x39, 0x91,
                                     0x0b, 0x0e, 0xfe, 0x38, 0x2e, 0xd1, 0x1d, 0x05, 0x9a, 0xf6,
                                     0x4d, 0x3e, 0x6f, 0x0f, 0x07, 0x1d, 0xaf, 0x2c, 0x1e, 0x8f,
                                     0x60, 0x39, 0xe2, 0xfa, 0x36, 0x53, 0x13, 0x39, 0xd4, 0x5e,
                                     0x26, 0x2b, 0xdb, 0x3d, 0xa8, 0x14, 0xbd, 0x32, 0xeb, 0x18,
                                     0x03, 0x28, 0x52, 0x04, 0x71, 0xe5, 0xab, 0x33, 0x3d, 0xe1,
                                     0x38, 0xbb, 0x07, 0x36, 0x84, 0x62, 0x9c, 0x79, 0xea, 0x16,
                                     0x30, 0xf4, 0x5f, 0xc0, 0x2b, 0xe8, 0x71, 0x6b, 0xe4, 0xf9,
                                     0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x53, 0x30, 0x51, 0x30,
                                     0x0f, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04,
                                     0x05, 0x30, 0x03, 0x01, 0x01, 0xff, 0x30, 0x1d, 0x06, 0x03,
                                     0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0xc0, 0x7a, 0x98,
                                     0x68, 0x8d, 0x89, 0xfb, 0xab, 0x05, 0x64, 0x0c, 0x11, 0x7d,
                                     0xaa, 0x7d, 0x65, 0xb8, 0xca, 0xcc, 0x4e, 0x30, 0x1f, 0x06,
                                     0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14,
                                     0xc0, 0x7a, 0x98, 0x68, 0x8d, 0x89, 0xfb, 0xab, 0x05, 0x64,
                                     0x0c, 0x11, 0x7d, 0xaa, 0x7d, 0x65, 0xb8, 0xca, 0xcc, 0x4e,
                                     0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
                                     0x01, 0x01, 0x05, 0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00,
                                     0x35, 0xe3, 0x29, 0x6a, 0xe5, 0x2f, 0x5d, 0x54, 0x8e, 0x29,
                                     0x50, 0x94, 0x9f, 0x99, 0x1a, 0x14, 0xe4, 0x8f, 0x78, 0x2a,
                                     0x62, 0x94, 0xa2, 0x27, 0x67, 0x9e, 0xd0, 0xcf, 0x1a, 0x5e,
                                     0x47, 0xe9, 0xc1, 0xb2, 0xa4, 0xcf, 0xdd, 0x41, 0x1a, 0x05,
                                     0x4e, 0x9b, 0x4b, 0xee, 0x4a, 0x6f, 0x55, 0x52, 0xb3, 0x24,
                                     0xa1, 0x37, 0x0a, 0xeb, 0x64, 0x76, 0x2a, 0x2e, 0x2c, 0xf3,
                                     0xfd, 0x3b, 0x75, 0x90, 0xbf, 0xfa, 0x71, 0xd8, 0xc7, 0x3d,
                                     0x37, 0xd2, 0xb5, 0x05, 0x95, 0x62, 0xb9, 0xa6, 0xde, 0x89,
                                     0x3d, 0x36, 0x7b, 0x38, 0x77, 0x48, 0x97, 0xac, 0xa6, 0x20,
                                     0x8f, 0x2e, 0xa6, 0xc9, 0x0c, 0xc2, 0xb2, 0x99, 0x45, 0x00,
                                     0xc7, 0xce, 0x11, 0x51, 0x22, 0x22, 0xe0, 0xa5, 0xea, 0xb6,
                                     0x15, 0x48, 0x09, 0x64, 0xea, 0x5e, 0x4f, 0x74, 0xf7, 0x05,
                                     0x3e, 0xc7, 0x8a, 0x52, 0x0c, 0xdb, 0x15, 0xb4, 0xbd, 0x6d,
                                     0x9b, 0xe5, 0xc6, 0xb1, 0x54, 0x68, 0xa9, 0xe3, 0x69, 0x90,
                                     0xb6, 0x9a, 0xa5, 0x0f, 0xb8, 0xb9, 0x3f, 0x20, 0x7d, 0xae,
                                     0x4a, 0xb5, 0xb8, 0x9c, 0xe4, 0x1d, 0xb6, 0xab, 0xe6, 0x94,
                                     0xa5, 0xc1, 0xc7, 0x83, 0xad, 0xdb, 0xf5, 0x27, 0x87, 0x0e,
                                     0x04, 0x6c, 0xd5, 0xff, 0xdd, 0xa0, 0x5d, 0xed, 0x87, 0x52,
                                     0xb7, 0x2b, 0x15, 0x02, 0xae, 0x39, 0xa6, 0x6a, 0x74, 0xe9,
                                     0xda, 0xc4, 0xe7, 0xbc, 0x4d, 0x34, 0x1e, 0xa9, 0x5c, 0x4d,
                                     0x33, 0x5f, 0x92, 0x09, 0x2f, 0x88, 0x66, 0x5d, 0x77, 0x97,
                                     0xc7, 0x1d, 0x76, 0x13, 0xa9, 0xd5, 0xe5, 0xf1, 0x16, 0x09,
                                     0x11, 0x35, 0xd5, 0xac, 0xdb, 0x24, 0x71, 0x70, 0x2c, 0x98,
                                     0x56, 0x0b, 0xd9, 0x17, 0xb4, 0xd1, 0xe3, 0x51, 0x2b, 0x5e,
                                     0x75, 0xe8, 0xd5, 0xd0, 0xdc, 0x4f, 0x34, 0xed, 0xc2, 0x05,
                                     0x66, 0x80, 0xa1, 0xcb, 0xe6, 0x33};

  bool isTimeSync = false;

  // wait 1 sec for LCD messages display
  MSTimerDelay(1000);
  NVSettingsLoad(&GNV_Setting);

#ifdef SHOW_VERSION
  DisplayLCD(LCD_LINE2, (const uint8_t *)ExositeAppVersion);
#endif

  // must initialize one time for mac address prepare..
  WIFI_init(1);
  if (!Exosite_Init("renesas", "rl78g14", IF_WIFI, 0))
  {
    show_status();
    while(1);
  }

  while(AtLibGs_AddCert(  EXOSITE_CA_NAME,
                          true,
                          geoCert,
                          sizeof(geoCert)) != ATLIBGS_MSG_ID_OK)
  {
    DisplayLCD(LCD_LINE4, "Add CA FAILED");
  }

  while (1)
  {
    if (!checkWiFiConnected(wifi_init))
    {
      wifi_init = 0;
    }
    else
    {
      if(!isTimeSync)
      {
        if(Exosite_SyncTime() == 0)
          isTimeSync = true;
      }
      
      UpdateReadings();

      int code = Exosite_StatusCode();
      if (code == EXO_STATUS_OK)
      {
        badcik = 0;
        wifi_init = 1;

        ReadCloudCommands();

        if (loopCount++ >= WRITE_INTERVAL) 
        {
          // POST the Sensor and templature values
          ReportReadings();
          loopCount = 0;
        }
        loop_time = 500; //delay 0.5 seconds before next turn..
      }
      else if (1 == badcik || EXO_STATUS_BAD_CIK == code || EXO_STATUS_NOAUTH == code)
      {
        DisplayLCD(LCD_LINE6, "  Exosite  ");
        DisplayLCD(LCD_LINE7, " Connecting");
        DisplayLCD(LCD_LINE8, "           ");

        if (!Exosite_Activate())
        {
          badcik = 1;
          loop_time = 3000; // delay 3 seconds
        }
        else
        {
          DisplayLCD(LCD_LINE7, " Connected ");
        }
      }
      show_status();
    }

    MSTimerDelay(loop_time);  //delay before looping again
  }
}

