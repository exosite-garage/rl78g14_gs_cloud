/*****************************************************************************
*
*  exosite_hal.c - Exosite hardware & environmenat adapation layer.
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
*    Neither the name of Exosite LLC nor the names of its contributors may
*    be used to endorse or promote products derived from this software
*    without specific prior written permission.
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
#include "exosite.h"
#include "exosite_hal.h"
#include "exosite_meta.h"
#include <string.h>
#include <inc/common.h>
#include <CmdLib/AtCmdLib.h>
#include <drv/Glyph/lcd.h>
#include <system/mstimer.h>
#include <init/hwsetup.h>
#include <system/eeprom.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <HostApp.h>
#include <system/platform.h>

#include <drv/Glyph/lcd.h>
#include <system/mstimer.h>
#include <system/console.h>
#include <apps/apps.h>

// local variables
#define GS_RECV_OFFSET 1
static uint8_t cid = 0xff;
char exometa[META_SIZE];
static int exo_recv_index = -1; // first socket buffer byte was GainSpan command byte

// local functions

// externs
extern void DisplayLCD(uint8_t, const uint8_t *);
extern char *itoa(int n, char *s, int b);

// global variables
#define EXOMETA_ADDR 177


/*****************************************************************************
*
*  exoHAL_ReadHWMAC
*
*  \param  Interface Number (1 - WiFi), buffer to return hexadecimal MAC
*
*  \return 0 if failure; len of UUID if success;
*
*  \brief  Reads the MAC address from the hardware
*
*****************************************************************************/
int
exoHAL_ReadUUID(unsigned char if_nbr, unsigned char * UUID_buf)
{
  int retval = 0;
  int check_count = 0;
  unsigned char macBuf[20];
  ATLIBGS_MSG_ID_E rxMsgId;

  switch (if_nbr) {
    case IF_GPRS:
      break;
    case IF_ENET:
      break;
    case IF_WIFI:
      // Read MAC address from gainspan
      rxMsgId = AtLibGs_GetMAC((char *)macBuf);
      if (rxMsgId == ATLIBGS_MSG_ID_OK)
        AtLibGs_ParseGetMacResponse((char *)macBuf);

      while (macBuf[check_count] && (check_count++ < 12));
      if (check_count > 13 || check_count < 11)
        return retval;

      memcpy(UUID_buf, macBuf, 12); // mac address length : 12
      UUID_buf[12] = 0;
      retval = strlen((char *)UUID_buf);
      break;
    default:
      break;
  }

  return retval;
}


/*****************************************************************************
*
* exoHAL_EnableNVMeta
*
*  \param  None
*
*  \return None
*
*  \brief  Enables meta non-volatile memory, if any
*
*****************************************************************************/
void
exoHAL_EnableMeta(void)
{
  return;
}


/*****************************************************************************
*
*  exoHAL_EraseNVMeta
*
*  \param  None
*
*  \return None
*
*  \brief  Wipes out meta information - replaces with 0's
*
*****************************************************************************/
void
exoHAL_EraseMeta(void)
{
  EEPROM_Erase(EXOMETA_ADDR,256);

  return;
}


/*****************************************************************************
*
*  exoHAL_WriteMetaItem
*
*  \param  buffer - string buffer containing info to write to meta; len -
*          size of string in bytes; offset - offset from base of meta
*          location to store the item
*
*  \return None
*
*  \brief  Stores information to the NV meta structure
*
*****************************************************************************/
void
exoHAL_WriteMetaItem(unsigned char * buffer, unsigned char len, int offset)
{
  EEPROM_Write(EXOMETA_ADDR+offset,(uint8_t *)buffer,len);

  return;
}


/*****************************************************************************
*
*  exoHAL_ReadMetaItem
*
*  \param  buffer - buffer we can read meta info into; len - size of the
*          buffer (max 256 bytes); offset - offset from base of meta to begin
*          reading from;
*
*  \return None
*
*  \brief  Reads information from the NV meta structure
*
*****************************************************************************/
void
exoHAL_ReadMetaItem(unsigned char * buffer, unsigned char len, int offset)
{
  EEPROM_Seq_Read(EXOMETA_ADDR+offset,(uint8_t *)buffer, len);

  return;
}


/*****************************************************************************
*
*  exoHAL_SocketClose
*
*  \param  socket - socket handle
*
*  \return None
*
*  \brief  Closes a socket
*
*****************************************************************************/
void
exoHAL_SocketClose(long socket)
{
  if(socket == (long)cid)
  {
    AtLibGs_Close(cid);
    cid = 0xff;
    exo_recv_index = -1;
  }
  return;
}


/*****************************************************************************
*
*  exoHAL_SocketOpenTCP
*
*  \param  None
*
*  \return -1: failure; Other: socket handle
*
*  \brief  Opens a TCP socket
*
*****************************************************************************/
long
exoHAL_SocketOpenTCP(unsigned char *server)
{
  int ip_len = 0;
  char serverip[20];
  uint16_t port;

  if (cid != 0xff)
    return -1;

  memset(serverip, 0, 20);
  itoa(server[0], &serverip[0], 10);
  if(server[0] >= 100)
    serverip[ip_len += 3] = 0x2e;
  else if(server[0] >= 10)
    serverip[ip_len += 2] = 0x2e;
  else
    serverip[ip_len += 1] = 0x2e;
  ip_len += 1;

  itoa(server[1], &serverip[ip_len], 10);
  if(server[1] >= 100)
    serverip[ip_len += 3] = 0x2e;
  else if(server[1] >= 10)
    serverip[ip_len += 2] = 0x2e;
  else
    serverip[ip_len += 1] = 0x2e;
  ip_len += 1;

  itoa(server[2], &serverip[ip_len], 10);
  if(server[2] >= 100)
    serverip[ip_len += 3] = 0x2e;
  else if(server[2] >= 10)
    serverip[ip_len += 2] = 0x2e;
  else
    serverip[ip_len += 1] = 0x2e;
  ip_len += 1;

  itoa(server[3], &serverip[ip_len], 10);

  port = server[4];
  port = (port << 8) + (uint16_t)server[5];
  AtLibGs_TCPClientStart(serverip, port, &cid);

  return (long)cid;
}


/*****************************************************************************
*
*  exoHAL_ServerConnect
*
*  \param  None
*
*  \return socket - socket handle
*
*  \brief  Opens a TCP socket
*
*****************************************************************************/
long
exoHAL_ServerConnect(long sock)
{
  //TODO - use DNS or check m2.exosite.com/ip to check for updates
  if( sock == (long)cid)
    return (long)cid;
  else
    return -1;
}

/*****************************************************************************
*
*  exoHAL_SocketSend
*
*  \param  socket - socket handle; buffer - string buffer containing info to
*          send; len - size of string in bytes;
*
*  \return Number of bytes sent
*
*  \brief  Sends data out to the internet
*
*****************************************************************************/
unsigned char
exoHAL_SocketSend(long socket, char * buffer, unsigned char len)
{
  App_PrepareIncomingData();
  if(socket == (long)cid)
    AtLibGs_SendTCPData(cid, (char *)buffer, len);
  else
    len = 0;

  return len;
}


/*****************************************************************************
*
*  exoHAL_SocketRecv
*
*  \param  socket - socket handle; buffer - string buffer to put info we
*          receive; len - size of buffer in bytes;
*
*  \return Number of bytes received
*
*  \brief  Receives data from the internet
*
*****************************************************************************/
unsigned char
exoHAL_SocketRecv(long socket, char * buffer, unsigned char len)
{
  if (socket == (long)cid)
  {
    int rec_len = 0, rxbufsize = 0;
    ATLIBGS_MSG_ID_E rxMsgId = ATLIBGS_MSG_ID_NONE;

    if (exo_recv_index == -1) {
      rxMsgId = AtLibGs_ReceiveDataHandle(3000);
      if (ATLIBGS_MSG_ID_DATA_RX != rxMsgId || G_receivedCount <= GS_RECV_OFFSET)
        return 0;
      exo_recv_index = GS_RECV_OFFSET;
    }
    rxbufsize = G_receivedCount - exo_recv_index;

    rec_len = len <= rxbufsize ? len : rxbufsize;
    memcpy(buffer, &G_received[exo_recv_index], rec_len);
    exo_recv_index += rec_len;

    if(exo_recv_index == G_receivedCount) {
      exo_recv_index = -1;
    }

    return rec_len;
  }

  return 0;
}

long exoHAL_ClientSSLOpen(long socket, char caName[])
{
  if(AtLibGs_SSLOpen((uint8_t)socket, caName) !=  ATLIBGS_MSG_ID_OK)
  {
    exoHAL_SocketClose(socket);
    return -1;
  }
  
  return socket;
}


/*****************************************************************************
*
*  exoHAL_MSDelay
*
*  \param  delay - milliseconds to delay
*
*  \return None
*
*  \brief  Delays for specified milliseconds
*
*****************************************************************************/
void
exoHAL_MSDelay(unsigned short delay)
{
  MSTimerDelay(delay);

  return;
}

