
/*====================================================================
 * os_pi.h
 *
 * Copyright 1995, Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics,
 * Inc.; the contents of this file may not be disclosed to third
 * parties, copied or duplicated in any form, in whole or in part,
 * without the prior written permission of Silicon Graphics, Inc.
 *
 * RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to
 * restrictions as set forth in subdivision (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS
 * 252.227-7013, and/or in similar or successor clauses in the FAR,
 * DOD or NASA FAR Supplement. Unpublished - rights reserved under the
 * Copyright Laws of the United States.
 *====================================================================*/

/*---------------------------------------------------------------------*
        Copyright (C) 1998 Nintendo. (Originated by SGI)
        
        $RCSfile: os_pi.h,v $
        $Revision: 1.1 $
        $Date: 1998/10/09 08:01:16 $
 *---------------------------------------------------------------------*/

#ifndef _OS_PI_H_
#define	_OS_PI_H_

#ifdef _LANGUAGE_C_PLUS_PLUS
extern "C" {
#endif

#include <PR/ultratypes.h>
#include "os_thread.h"
#include "os_message.h"
#include <stdint.h>

#if defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS)

/**************************************************************************
 *
 * Type definitions
 *
 */

/*
 * Structure for Enhanced PI interface
 */

/*
 * OSTranxInfo is set up for Leo Disk DMA. This info will be maintained
 * by exception handler. This is how the PIMGR and the ISR communicate.
 */

typedef struct {
	u32		errStatus;	/* error status */
        void     	*dramAddr;      /* RDRAM buffer address (DMA) */
	void		*C2Addr;	/* C2 buffer address */
	u32		sectorSize;	/* size of transfering sector */
	u32		C1ErrNum;	/* total # of C1 errors */
	u32		C1ErrSector[4];	/* error sectors */
} __OSBlockInfo;

typedef struct {
	u32     	cmdType;       	/* for disk only */
	u16     	transferMode;   /* Block, Track, or sector?   */
	u16		blockNum;	/* which block is transfering */
	s32     	sectorNum;      /* which sector is transfering */
	u32     	devAddr;        /* Device buffer address */
	u32		bmCtlShadow;	/* asic bm_ctl(510) register shadow ram */
	u32		seqCtlShadow;	/* asic seq_ctl(518) register shadow ram */
	__OSBlockInfo	block[2];	/* bolck transfer info */
} __OSTranxInfo;


typedef struct OSPiHandle_s {
        struct OSPiHandle_s     *next;  /* point to next handle on the table */
        u8                      type;   /* DEVICE_TYPE_BULK for disk */
        u8                      latency;        /* domain latency */
        u8                      pageSize;       /* domain page size */
        u8                      relDuration;    /* domain release duration */
        u8                      pulse;          /* domain pulse width */
	u8			domain;		/* which domain */
        u32                     baseAddress;    /* Domain address */
        u32                     speed;          /* for roms only */
        /* The following are "private" elements" */
        __OSTranxInfo           transferInfo;	/* for disk only */
} OSPiHandle;

typedef struct {
        u8      type;
        u32     address;
} OSPiInfo;

/*
 * Structure for I/O message block
 */
typedef struct {
        u16 		type;		/* Message type */
        u8 		pri;		/* Message priority (High or Normal) */
        u8		status;		/* Return status */
	OSMesgQueue	*retQueue;	/* Return message queue to notify I/O 
					 * completion */
} OSIoMesgHdr;

typedef struct {
	OSIoMesgHdr	hdr;		/* Message header */
	void *		dramAddr;	/* RDRAM buffer address (DMA) */
	u32		devAddr;	/* Device buffer address (DMA) */
	u32 		size;		/* DMA transfer size in bytes */
	OSPiHandle	*piHandle;	/* PI device handle */
} OSIoMesg;

/*
 * Structure for device manager block
 */
typedef struct {
        s32             active;		/* Status flag */
	OSThread	*thread;	/* Calling thread */
        OSMesgQueue  	*cmdQueue;	/* Command queue */
        OSMesgQueue  	*evtQueue;	/* Event queue */
        OSMesgQueue  	*acsQueue;	/* Access queue */
					/* Raw DMA routine */
        s32             (*dma)(s32, u32, void *, u32);
        s32             (*edma)(OSPiHandle *, s32, u32, void *, u32);
} OSDevMgr;


#endif /* defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS) */

/**************************************************************************
 *
 * Global definitions
 *
 */

/* Flags to indicate direction of data transfer */

#define	OS_READ			0		/* device -> RDRAM */
#define	OS_WRITE		1		/* device <- RDRAM */
#define	OS_OTHERS		2		/* for Leo disk only */

/*
 * I/O message types
 */
#define OS_MESG_TYPE_BASE	(10)
#define OS_MESG_TYPE_LOOPBACK	(OS_MESG_TYPE_BASE+0)
#define OS_MESG_TYPE_DMAREAD	(OS_MESG_TYPE_BASE+1)
#define OS_MESG_TYPE_DMAWRITE	(OS_MESG_TYPE_BASE+2)
#define OS_MESG_TYPE_VRETRACE	(OS_MESG_TYPE_BASE+3)
#define OS_MESG_TYPE_COUNTER	(OS_MESG_TYPE_BASE+4)
#define OS_MESG_TYPE_EDMAREAD	(OS_MESG_TYPE_BASE+5)
#define OS_MESG_TYPE_EDMAWRITE	(OS_MESG_TYPE_BASE+6)

/*
 * I/O message priority
 */
#define OS_MESG_PRI_NORMAL	0
#define OS_MESG_PRI_HIGH	1

/*
 * PI/EPI
 */
#define PI_DOMAIN1      0
#define PI_DOMAIN2      1


#if defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS)

/**************************************************************************
 *
 * Macro definitions
 *
 */


/**************************************************************************
 *
 * Extern variables
 *
 */

extern OSPiHandle      *__osPiTable;    /* The head of OSPiHandle link list */


/**************************************************************************
 *
 * Function prototypes
 *
 */

/* Peripheral interface (Pi) */
extern u32 		osPiGetStatus(void);
extern s32		osPiGetDeviceType(void);
extern s32		osPiWriteIo(u32, u32);
extern s32		osPiReadIo(u32, u32 *);
extern s32		osPiStartDma(OSIoMesg *, s32, s32, uintptr_t, void *, u32,
				     OSMesgQueue *);
extern void		osCreatePiManager(OSPri, OSMesgQueue *, OSMesg *, s32);

/* Enhanced PI interface */

extern OSPiHandle *osCartRomInit(void);
extern OSPiHandle *osLeoDiskInit(void);
extern OSPiHandle *osDriveRomInit(void);

extern s32 osEPiDeviceType(OSPiHandle *, OSPiInfo *);
extern s32 osEPiWriteIo(OSPiHandle *, u32 , u32 );
extern s32 osEPiReadIo(OSPiHandle *, u32 , u32 *);
extern s32 osEPiStartDma(OSPiHandle *, OSIoMesg *, s32);
extern s32 osEPiLinkHandle(OSPiHandle *);


#endif  /* defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS) */

#ifdef _LANGUAGE_C_PLUS_PLUS
}
#endif

#endif /* !_OS_PI_H_ */
