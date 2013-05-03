/*-------------------------------------------------------------------------*
 * File:  Wifi_Helper.c
 *-------------------------------------------------------------------------*
 * Description:
 *     Common wifi function & sensor used by all the Apps.
 *-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*
 * Includes:
 *-------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <CmdLib/AtCmdLib.h>
#include <drv/Glyph/lcd.h>
#include <system/mstimer.h>
#include <system/eeprom.h>
#include <system/platform.h>
#include "NVSettings.h"

// global defines

// external defines
extern NVSettings_t GNV_Setting;

// local defines


/*---------------------------------------------------------------------------*
 * Routine:  WIFI_init
 *---------------------------------------------------------------------------*
 * Description:
 *      Initial setting + DHCP and show status on the LCD.
 *
 * Inputs:
 *      void
 * Outputs:
 *      ATLIBGS_MSG_ID_E
 *---------------------------------------------------------------------------*/
ATLIBGS_MSG_ID_E WIFI_init(int16_t showMessage)
{
  ATLIBGS_MSG_ID_E rxMsgId = ATLIBGS_MSG_ID_NONE;
  char wifi_mac[20];

  // Check the link
#ifdef HOST_APP_DEBUG_ENABLE
  ConsolePrintf("Checking link\r\n");
#endif

  AtLibGs_Init();
  // Wait for the banner
  MSTimerDelay(500);

  // Send command to check
  do {
    AtLibGs_FlushIncomingMessage();
    DisplayLCD(LCD_LINE8, "Checking...");
    rxMsgId = AtLibGs_Check();
  } while (ATLIBGS_MSG_ID_OK != rxMsgId);

  do {
    rxMsgId = AtLibGs_SetEcho(0);               // disable Echo
  }while (ATLIBGS_MSG_ID_OK != rxMsgId);

  do {
    rxMsgId = AtLibGs_Version();                // check the GS version
  }while (ATLIBGS_MSG_ID_OK != rxMsgId);

  // Get MAC Address & Show
  rxMsgId = AtLibGs_GetMAC(wifi_mac);
  if (rxMsgId == ATLIBGS_MSG_ID_OK)
    AtLibGs_ParseGetMacResponse(wifi_mac);
  memset(&wifi_mac[12], 0, 7);
  if (showMessage > 0) {
    DisplayLCD(LCD_LINE5, "MAC ADDRESS");
    DisplayLCD(LCD_LINE6, (const uint8_t *)wifi_mac);
    MSTimerDelay(2000);
    DisplayLCD(LCD_LINE2, "            ");
  }

  do {
    AtLibGs_FlushIncomingMessage();
    DisplayLCD(LCD_LINE8, "Disassociate");
    rxMsgId = AtLibGs_DisAssoc();
  } while (ATLIBGS_MSG_ID_OK != rxMsgId);

  // Enable DHCP
  do {
    DisplayLCD(LCD_LINE8, "DHCP On...");
    rxMsgId = AtLibGs_DHCPSet(1);
  } while (ATLIBGS_MSG_ID_OK != rxMsgId);

  if(strlen(GNV_Setting.webprov.ssid) > 0)
  {
    if(GNV_Setting.webprov.security == ATLIBGS_PROVSECU_WEP)
    {
      do {
        DisplayLCD(LCD_LINE8, " Setting WEP");
        rxMsgId = AtLibGs_SetWEP1((int8_t*)GNV_Setting.webprov.password);
      } while (ATLIBGS_MSG_ID_OK != rxMsgId);
      DisplayLCD(LCD_LINE8, " WEP Set");
    }
    else if(GNV_Setting.webprov.security == ATLIBGS_PROVSECU_WPA_PER)
    {
      do {
        DisplayLCD(LCD_LINE8, " Setting PSK");
        rxMsgId = AtLibGs_CalcNStorePSK(GNV_Setting.webprov.ssid, GNV_Setting.webprov.password);
      } while (ATLIBGS_MSG_ID_OK != rxMsgId); 
      DisplayLCD(LCD_LINE8, " PSK Set");
    }
    else if(GNV_Setting.webprov.security == ATLIBGS_PROVSECU_WPA_ENT)
    {
      // Set AT+WAUTH=0 for WPA or WPA2
      do {
        DisplayLCD(LCD_LINE8, "       " );
        rxMsgId = AtLibGs_SetAuthentictionMode(ATLIBGS_AUTHMODE_NONE_WPA);
      } while (ATLIBGS_MSG_ID_OK != rxMsgId);
      // Security Configuration
      do {
        DisplayLCD(LCD_LINE8, "Set Security");
        rxMsgId = AtLibGs_SetSecurity(ATLIBGS_SMAUTO);
      } while (ATLIBGS_MSG_ID_OK != rxMsgId);  
    }
  }
  else
  {
#ifdef HOST_APP_SEC_WEP
    // Set AT+WAUTH=2 for WEP
    do {
      DisplayLCD(LCD_LINE8, " WEP AUTH " );
      rxMsgId = AtLibGs_SetAuthentictionMode(2);
    } while (ATLIBGS_MSG_ID_OK != rxMsgId);
    // Set WEP
    do {
      rxMsgId = AtLibGs_SetWEP1(HOST_APP_AP_SEC_WEP);
    } while (ATLIBGS_MSG_ID_OK != rxMsgId);
    // Security Configuration
    do {
      rxMsgId = AtLibGs_SetSecurity(2);        // WEP
    } while (ATLIBGS_MSG_ID_OK != rxMsgId);
#endif

#ifdef HOST_APP_SEC_PSK
    // Store the PSK value. This call takes might take few seconds to return
    do {
      DisplayLCD(LCD_LINE8, "Setting PSK");
      rxMsgId = AtLibGs_CalcNStorePSK(HOST_APP_AP_SSID, HOST_APP_AP_SEC_PSK);
    } while (ATLIBGS_MSG_ID_OK != rxMsgId);
#endif

#ifdef HOST_APP_SEC_OPEN
    // Store the PSK value. This call takes might take few seconds to return
    do {
      DisplayLCD(LCD_LINE8, "No Security" );
      rxMsgId = AtLibGs_SetAuthentictionMode(1);
    } while (ATLIBGS_MSG_ID_OK != rxMsgId);
#endif

#ifdef HOST_APP_WPA
    // Set AT+WAUTH=0 for WPA or WPA2
    do {
      DisplayLCD(LCD_LINE8, "   WPA   " );
      rxMsgId = AtLibGs_SetAuthentictionMode(0);
    } while (ATLIBGS_MSG_ID_OK != rxMsgId);

    // Store the PSK value. This call takes might take few seconds to return
    do {
      DisplayLCD(LCD_LINE8, "Setting PSK");
      rxMsgId = AtLibGs_CalcNStorePSK(HOST_APP_AP_SSID, HOST_APP_AP_SEC_PSK);
    } while (ATLIBGS_MSG_ID_OK != rxMsgId);

    // Security Configuration
    do {
      DisplayLCD(LCD_LINE8, "   WPA   ");
      rxMsgId = AtLibGs_SetSecurity(4);
    } while (ATLIBGS_MSG_ID_OK != rxMsgId);
#endif

#ifdef HOST_APP_WPA2
    // Set AT+WAUTH=0 for WPA or WPA2
    do {
      DisplayLCD(LCD_LINE8, "  WPA2   " );
      rxMsgId = AtLibGs_SetAuthentictionMode(ATLIBGS_AUTHMODE_NONE_WPA);
    } while (ATLIBGS_MSG_ID_OK != rxMsgId);

    // Store the PSK value. This call takes might take few seconds to return
    do {
      DisplayLCD(LCD_LINE8, "Setting PSK");
      rxMsgId = AtLibGs_CalcNStorePSK(HOST_APP_AP_SSID, HOST_APP_AP_SEC_PSK);
    } while (ATLIBGS_MSG_ID_OK != rxMsgId);

    // Security Configuration
    do {
      DisplayLCD(LCD_LINE8, "  Set WPA  ");
      rxMsgId = AtLibGs_SetSecurity(ATLIBGS_SMWPA2PSK);
    } while (ATLIBGS_MSG_ID_OK != rxMsgId);
#endif
  }
  // Clear MAC Address and show WIFI
  DisplayLCD(LCD_LINE6, "    WIFI   ");
  DisplayLCD(LCD_LINE5, "           ");

  return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  WIFI_Associate
 *---------------------------------------------------------------------------*
 * Description:
 *      Association and show result on the LCD
 * Inputs:
 *      void
 * Outputs:
 *      ATLIBGS_MSG_ID_E
 *---------------------------------------------------------------------------*/
ATLIBGS_MSG_ID_E WIFI_Associate(void)
{
  ATLIBGS_MSG_ID_E rxMsgId = ATLIBGS_MSG_ID_NONE;

  DisplayLCD(LCD_LINE7, " Connecting ");

  /* Associate to a particular AP specified by SSID  */
  if (strcmp(GNV_Setting.webprov.ssid, ATLIBGS_AP_SSID) == 0)
  {
    DisplayLCD(LCD_LINE7, "            ");
    MSTimerDelay(300);
    DisplayLCD(LCD_LINE7, "  NO SSID   ");
    DisplayLCD(LCD_LINE8, "            ");
    MSTimerDelay(500);
    return rxMsgId;
  }
  else
  {
    DisplayLCD(LCD_LINE8, (const uint8_t *)GNV_Setting.webprov.ssid);
    rxMsgId = AtLibGs_Assoc(GNV_Setting.webprov.ssid,NULL,HOST_APP_AP_CHANNEL);
  }
  if (ATLIBGS_MSG_ID_OK != rxMsgId) {
    /* Association error - we can retry */
#ifdef HOST_APP_DEBUG_ENABLE
        ConsolePrintf("\n Association error - retry now \n");
#endif
    DisplayLCD(LCD_LINE7, " Connecting..");
    MSTimerDelay(2000);
    DisplayLCD(LCD_LINE7, "");
  } else {
    /* Association success */
    AtLibGs_SetNodeAssociationFlag();
    DisplayLCD(LCD_LINE7, " Connected ");
    MSTimerDelay(2000);
    DisplayLCD(LCD_LINE7, "");
  }

  return rxMsgId;
}


