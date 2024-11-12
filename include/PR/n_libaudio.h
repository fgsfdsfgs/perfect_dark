/*====================================================================
 *
 * Copyright 1993, Silicon Graphics, Inc.
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

#ifndef __N_LIBAUDIO__
#define __N_LIBAUDIO__

#include <PR/libaudio.h>

#ifdef _LANGUAGE_C_PLUS_PLUS
extern "C" {
#endif

#include <PR/ultratypes.h>
#include <PR/mbi.h>
#include "platform.h"

struct var8009c340 {
	u8 surround;
	u8 mono;
	u8 headphone;
	u8 unk03;
};

extern struct var8009c340 var8009c340;
extern u8 var8009c344[2];
extern u8 var8009c346[2];
extern u8 var8009c348[4];

/*
 * Synthesis driver stuff
 */
typedef struct N_ALVoice_s {
    ALLink              node;
    struct N_PVoice_s     *pvoice;
    ALWaveTable         *table;
    void                *clientPrivate;
    s16                 state;
    s16                 priority;
    s16                 fxBus;
    s16                 unityPitch;
} N_ALVoice;

typedef struct {
    ALPlayer    *head;          /* client list head                     */
    ALLink      pFreeList;      /* list of free physical voices         */
    ALLink      pAllocList;     /* list of allocated physical voices    */
    ALLink      pLameList;      /* list of voices ready to be freed     */
    s32         paramSamples;
    s32         curSamples;     /* samples from start of game           */
    ALDMANew    dma;
    ALHeap      *heap;
    struct ALParam_s    *paramList;
    struct N_ALMainBus_s  *mainBus;
    struct N_ALAuxBus_s   *auxBus;
    s32                 numPVoices;
    s32                 maxAuxBusses;
    s32                 outputRate;
    s32                 maxOutSamples;
    uintptr_t           sv_dramout;
    s32                 sv_first;
} N_ALSynth;


void    n_alSynAddPlayer(ALPlayer *client);
void    n_alSynAddSndPlayer(ALPlayer *client);
void    n_alSynAddSeqPlayer(ALPlayer *client);

ALFxRef n_alSynAllocFX( s16 bus,ALSynConfig *c, ALHeap *hp);
s32     n_alSynAllocVoice( N_ALVoice *voice, ALVoiceConfig *vc);


void    n_alSynFreeVoice(N_ALVoice *voice);
ALFxRef n_alSynGetFXRef( s16 bus, s16 index);
s16     n_alSynGetPriority( N_ALVoice *voice);
void    n_alSynRemovePlayer( ALPlayer *client);
void    n_alSynSetFXMix(N_ALVoice *v, u8 fxmix);
void    n_alSynSetFXParam(ALFxRef fx, s16 paramID, void *param);
void    n_alSynFreeFX(ALFxRef *fx);
void    n_alSynSetPan(N_ALVoice *v, u8 pan);
void    n_alSynSetPitch( N_ALVoice *v, f32 pitch);
void    n_alSynSetPriority( N_ALVoice *voice, s16 priority);
void    n_alSynSetVol( N_ALVoice *v, s16 volume, ALMicroTime t);
void    n_alSynStartVoice(N_ALVoice *v, ALWaveTable *table);
void    n_alSynStartVoiceParams(N_ALVoice *v, ALWaveTable *w,f32 pitch, s16 vol,
				ALPan pan, u8 fxmix, u8 arg6, f32 arg7, u8 arg8, ALMicroTime t);
void    n_alSynStopVoice( N_ALVoice *v);

void    n_alSynFilter11(N_ALVoice *v, u8 channel);
void    n_alSynFilter12(N_ALVoice *v, u8 arg1);
void    n_alSynFilter13(N_ALVoice *v, f32 arg1);

void    n_alSynNew(ALSynConfig *c);
void    n_alSynDelete(void);


/*
 * Audio Library (AL) stuff
 */
typedef struct {
    N_ALSynth     drvr;
} N_ALGlobals;

extern N_ALGlobals *n_alGlobals;
extern N_ALSynth *n_syn;

void n_alInit(N_ALGlobals *g, ALSynConfig *c);
void n_alClose(N_ALGlobals *glob);
Acmd *n_alAudioFrame(Acmd *cmdList, s32 *cmdLen,
			s16 *outBuf, s32 outLen);


/*
 * Sequence Player stuff
 */
typedef struct {
    struct N_ALVoice_s    *voice;
} N_ALNoteEvent;


typedef struct {
    struct N_ALVoice_s    *voice;
    ALMicroTime         delta;
    u8                  vol;
} N_ALVolumeEvent;


typedef struct {
    struct N_ALVoiceState_s      *vs;
    void                       *oscState;
    u8                         chan;
} N_ALOscEvent;

typedef struct {
    f32 unk00;
    f32 unk04;
} N_AL18Event;

typedef struct {
    u8 unk00;
    u8 unk01;
    u8 unk02;
    u8 unk03;
    u8 param;
} N_AL19Event;

typedef struct {
    struct sndstate *sndstate;
    s32 data;
    s32 data2;
} N_ALGenericEvent;

typedef struct {
    s16                 	type;
    union {
		ALMIDIEvent     	midi;
		ALTempoEvent    	tempo;
		ALEndEvent      	end;
		N_ALNoteEvent     	note;
		N_ALVolumeEvent   	vol;
		ALSeqpLoopEvent 	loop;
		ALSeqpVolEvent  	spvol;
		ALSeqpPriorityEvent	sppriority;
		ALSeqpSeqEvent		spseq;
		ALSeqpBankEvent		spbank;
		N_ALOscEvent      	osc;
		N_AL18Event      	evt18;
		N_AL19Event      	evt19;
		N_ALGenericEvent    generic;
    } msg;
} N_ALEvent;


typedef struct {
    ALLink      node;
    ALMicroTime delta;
    N_ALEvent     evt;
} N_ALEventListItem;

void            n_alEvtqNew(ALEventQueue *evtq, N_ALEventListItem *items, s32 itemCount);
ALMicroTime     n_alEvtqNextEvent(ALEventQueue *evtq, N_ALEvent *evt);
void            n_alEvtqPostEvent(ALEventQueue *evtq, N_ALEvent *evt, ALMicroTime delta, s32 arg3);
void        	n_alEvtqFlushType(ALEventQueue *evtq, s16 type);

struct oscstate {
	u32 unk00;
	u8 unk04;
	u32 unk08;
	f32 unk0c;
	f32 unk10;
};

struct fx {
	s16 unk00;
	s16 unk02;
	s32 unk04;
	s16 unk08[16];
};

typedef struct N_ALVoiceState_s {
    struct N_ALVoiceState_s *next;/* MUST be first                */
    N_ALVoice   voice;
    ALSound    *sound;
    ALMicroTime envEndTime;     /* time of envelope segment end */
    f32         pitch;          /* currect pitch ratio          */
    f32         vibrato;        /* current value of the vibrato */
    u8          envGain;        /* current envelope gain        */
    u8          channel;        /* channel assignment           */
    u8          key;            /* note on key number           */
    u8          velocity;       /* note on velocity             */
    u8          envPhase;       /* what envelope phase          */
    u8          phase;
    u8          tremelo;        /* current value of the tremelo */
    u8          flags;          /* bit 0 tremelo flag
                                   bit 1 vibrato flag           */
	void *oscState;
	struct oscstate *oscState2;
} N_ALVoiceState;

typedef struct {
    ALPlayer            node;          /* note: must be first in structure */
    N_ALSynth          *drvr;          /* reference to the client driver   */
    ALSeq              *target;        /* current sequence                 */
    ALMicroTime         curTime;
    ALBank             *bank;           /* current ALBank                   */
    s32                 uspt;           /* microseconds per tick            */
    s32                 nextDelta;      /* microseconds to next callback    */
    s32                 state;
    u16                 chanMask;       /* active channels                  */
    s16                 vol;            /* overall sequence volume          */
    u8                  maxChannels;    /* number of MIDI channels          */
    u8                  debugFlags;     /* control which error get reported */
    N_ALEvent           nextEvent;
    ALEventQueue        evtq;
    ALMicroTime         frameTime;
    ALChanState        *chanState;      /* 16 channels for MIDI             */
    N_ALVoiceState     *vAllocHead;     /* list head for allocated voices   */
    N_ALVoiceState     *vAllocTail;     /* list tail for allocated voices   */
    N_ALVoiceState     *vFreeList;      /* list of free voice state structs */
    ALOscInit           initOsc;
    ALOscUpdate         updateOsc;
    ALOscStop           stopOsc;
    ALSeqMarker        *loopStart;
    ALSeqMarker        *loopEnd;
    intptr_t            loopCount;      /* -1 = loop forever, 0 = no loop   */
    u8 unk88;
    u8 unk89;
} N_ALSeqPlayer;

typedef struct {
    ALPlayer            node;           /* note: must be first in structure */
    N_ALSynth          *drvr;           /* reference to the client driver   */
    ALCSeq             *target;         /* current sequence                 */
    ALMicroTime         curTime;
    ALBank             *bank;           /* current ALBank                   */
    s32                 uspt;           /* microseconds per tick            */
    s32                 nextDelta;      /* microseconds to next callback    */
    s32                 state;
    u16                 chanMask;       /* active channels                  */
    s16                 vol;            /* overall sequence volume          */
    u8                  maxChannels;    /* number of MIDI channels          */
    u8                  debugFlags;     /* control which error get reported */
    N_ALEvent           nextEvent;
    ALEventQueue        evtq;
    ALMicroTime         frameTime;
    ALChanState        *chanState;      /* 16 channels for MIDI             */
    N_ALVoiceState     *vAllocHead;     /* list head for allocated voices   */
    N_ALVoiceState     *vAllocTail;     /* list tail for allocated voices   */
    N_ALVoiceState     *vFreeList;      /* list of free voice state structs */
    ALOscInit           initOsc;
    ALOscUpdate         updateOsc;
    ALOscStop           stopOsc;

#ifdef PLATFORM_64BIT
    s32 _pad1_;
    f32 unk7c;
    s32 _pad2_;
    f32 unk80;
#else
    f32 unk7c;
    f32 unk80;
#endif

    void *queue;

    u8 unk88;
    u8 unk89;
} N_ALCSPlayer;


/*
 * Sequence data representation routines
 */
void    n_alSeqNextEvent(ALSeq *seq, N_ALEvent *event);
void    n_alSeqNewMarker(ALSeq *seq, ALSeqMarker *m, u32 ticks);

void    n_alCSeqNew(ALCSeq *seq, u8 *ptr);
void    n_alCSeqNextEvent(ALCSeq *seq, N_ALEvent *evt, s32 arg2);
void    n_alCSeqNewMarker(ALCSeq *seq, ALCSeqMarker *m, u32 ticks);


/*
 * Sequence Player routines
 */
void    n_alSeqpNew(N_ALSeqPlayer *seqp, ALSeqpConfig *config);
void    n_alSeqpDelete(N_ALSeqPlayer *seqp);
u8	n_alSeqpGetChlVol(N_ALSeqPlayer *seqp, u8 chan);
u8      n_alSeqpGetChlFXMix(N_ALSeqPlayer *seqp, u8 chan);
ALPan   n_alSeqpGetChlPan(N_ALSeqPlayer *seqp, u8 chan);
u8      n_alSeqpGetChlPriority(N_ALSeqPlayer *seqp, u8 chan);
s32     n_alSeqpGetChlProgram(N_ALSeqPlayer *seqp, u8 chan);
ALSeq  *n_alSeqpGetSeq(N_ALSeqPlayer *seqp);
s32	n_alSeqpGetState(N_ALSeqPlayer *seqp);
s32     n_alSeqpGetTempo(N_ALSeqPlayer *seqp);
s16     n_alSeqpGetVol(N_ALSeqPlayer *seqp);		/* Master volume control */
void    n_alSeqpPlay(N_ALSeqPlayer *seqp);
void    n_alSeqpSendMidi(N_ALSeqPlayer *seqp, s32 ticks, u8 status, u8 byte1, u8 byte2);
void    n_alSeqpSetBank(N_ALSeqPlayer *seqp, ALBank *b);
void	n_alSeqpSetChlVol(N_ALSeqPlayer *seqp, u8 chan, u8 vol);
void    n_alSeqpSetChlFXMix(N_ALSeqPlayer *seqp, u8 chan, u8 fxmix);
void    n_alSeqpSetChlPan(N_ALSeqPlayer *seqp, u8 chan, ALPan pan);
void    n_alSeqpSetChlPriority(N_ALSeqPlayer *seqp, u8 chan, u8 priority);
void    n_alSeqpSetChlProgram(N_ALSeqPlayer *seqp, u8 chan, u8 prog);
void    n_alSeqpSetSeq(N_ALSeqPlayer *seqp, ALSeq *seq);
void    n_alSeqpSetTempo(N_ALSeqPlayer *seqp, s32 tempo);
void    n_alSeqpSetVol(N_ALSeqPlayer *seqp, s16 vol);
void    n_alSeqpStop(N_ALSeqPlayer *seqp);
void    n_alSeqpLoop(N_ALSeqPlayer *seqp, ALSeqMarker *start, ALSeqMarker *end, s32 count);
void    n_alSeqpSetFilter11(N_ALSeqPlayer *seqp);


/*
 * Compressed Sequence Player routines
 */
void    n_alCSPNew(N_ALCSPlayer *seqp, ALSeqpConfig *config);
void    n_alCSPDelete(N_ALCSPlayer *seqp);
u8	n_alCSPGetChlVol(N_ALCSPlayer *seqp, u8 chan);
u8      n_alCSPGetChlFXMix(N_ALCSPlayer *seqp, u8 chan);
ALPan   n_alCSPGetChlPan(N_ALCSPlayer *seqp, u8 chan);
u8      n_alCSPGetChlPriority(N_ALCSPlayer *seqp, u8 chan);
s32     n_alCSPGetChlProgram(N_ALCSPlayer *seqp, u8 chan);
ALCSeq *n_alCSPGetSeq(N_ALCSPlayer *seqp);
s32	n_alCSPGetState(N_ALCSPlayer *seqp);
s32     n_alCSPGetTempo(N_ALCSPlayer *seqp);
s16     n_alCSPGetVol(N_ALCSPlayer *seqp);
void    n_alCSPPlay(N_ALCSPlayer *seqp);
void    n_alCSPSendMidi(N_ALCSPlayer *seqp, s32 ticks, u8 status, u8 byte1, u8 byte2);
void    n_alCSPSetBank(N_ALCSPlayer *seqp, ALBank *b);
void	n_alCSPSetChlVol(N_ALCSPlayer *seqp, u8 chan, u8 vol);
void    n_alCSPSetChlFXMix(N_ALCSPlayer *seqp, u8 chan, u8 fxmix);
void    n_alCSPSetChlPan(N_ALCSPlayer *seqp, u8 chan, ALPan pan);
void    n_alCSPSetChlPriority(N_ALCSPlayer *seqp, u8 chan, u8 priority);
void    n_alCSPSetChlProgram(N_ALCSPlayer *seqp, u8 chan, u8 prog);
void    n_alCSPSetSeq(N_ALCSPlayer *seqp, ALCSeq *seq);
void    n_alCSPSetTempo(N_ALCSPlayer *seqp, s32 tempo);
void    n_alCSPSetVol(N_ALCSPlayer *seqp, s16 vol);
void    n_alCSPStop(N_ALCSPlayer *seqp);


/*
 * Sound Player stuff
 */
typedef struct {
    ALPlayer            node;           /* note: must be first in structure */
    ALEventQueue        evtq;
    N_ALEvent           nextEvent;
    N_ALSynth           *drvr;          /* reference to the client driver   */
    struct sndstate     *target;
    void                *sndState;
    s32                 maxSounds;
    ALMicroTime         frameTime;
    ALMicroTime         nextDelta;      /* microseconds to next callback    */
    ALMicroTime         curTime;
} N_ALSndPlayer;

void     n_alSndpNew(ALSndpConfig *c);
void     n_alSndpDelete(void);
ALSndId  n_alSndpAllocate(ALSound *sound);
void     n_alSndpDeallocate(ALSndId id);
s32      n_alSndpGetState(void);
void     n_alSndpPlay(void);
void     n_alSndpPlayAt(ALMicroTime delta);
void     n_alSndpSetFXMix(u8 mix);
void     n_alSndpSetPan(ALPan pan);
void     n_alSndpSetPitch(f32 pitch);
void     n_alSndpSetPriority(ALSndId id, u8 priority);
void     n_alSndpSetVol(s16 vol);
void     n_alSndpStop(void);
ALSndId  n_alSndpGetSound(void);
void     n_alSndpSetSound(ALSndId id);


/*
 *  for n_audio micro code
 */
extern long long int    n_aspMainTextStart[], n_aspMainTextEnd[];
extern long long int    n_aspMainDataStart[], n_aspMainDataEnd[];

f32 func0003b9d4(s32 arg0);
void func0003ba64(struct fx *fx, f32 outputrate);
s16 _getRate(f32 vol, f32 tgt, s32 count, u16 *ratel);
s16 _getVol(s16 ivol, s32 samples, s16 ratem, u16 ratel);

#ifdef _LANGUAGE_C_PLUS_PLUS
}
#endif

#endif /* __N_LIBAUDIO__ */
