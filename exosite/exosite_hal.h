/*****************************************************************************
*
*  exosite_hal.h - Common header for Exosite hardware adapation layer
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

#ifndef EXOSITE_HAL_H
#define EXOSITE_HAL_H

// defines
#define EXOSITE_HAL_SN_MAXLENGTH             25

// functions for export
extern int exoHAL_ReadUUID(unsigned char if_nbr, unsigned char * UUID_buf);
extern void exoHAL_EnableMeta(void);
extern void exoHAL_EraseMeta(void);
extern void exoHAL_WriteMetaItem(unsigned char * buffer, unsigned char len, int offset);
extern void exoHAL_ReadMetaItem(unsigned char * buffer, unsigned char len, int offset);
extern void exoHAL_SocketClose(long socket);
extern long exoHAL_SocketOpenTCP(unsigned char *server);
extern long exoHAL_ServerConnect(long socket);
extern long exoHAL_ClientSSLOpen(long socket, char caName[]);
extern unsigned char exoHAL_SocketSend(long socket, char * buffer, unsigned char len);
extern unsigned char exoHAL_SocketRecv(long socket, char * buffer, unsigned char len);
extern void exoHAL_MSDelay(unsigned short delay);

#endif

