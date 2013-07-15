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
#define ExositeAppVersion                   "   v1.02   "
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

  while (1)
  {
    if (!checkWiFiConnected(wifi_init))
    {
      wifi_init = 0;
    }
    else
    {
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





