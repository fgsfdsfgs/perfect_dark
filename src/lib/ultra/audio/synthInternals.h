/*====================================================================
 * audioInternals.h
 *
 * Synopsis:
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

#ifndef __audioInternals__
#define __audioInternals__

#include <libaudio.h>

/*
 * filter message ids
 */
enum {
	AL_FILTER_FREE_VOICE,
	AL_FILTER_SET_SOURCE,
	AL_FILTER_ADD_SOURCE,
	AL_FILTER_ADD_UPDATE,
	AL_FILTER_RESET,
	AL_FILTER_SET_WAVETABLE,
	/*    AL_FILTER_SET_DMA_PROC,*/
	/*    AL_FILTER_SKIP_LOOP,*/
	AL_FILTER_SET_DRAM,
	AL_FILTER_SET_PITCH,
	AL_FILTER_SET_UNITY_PITCH,
	AL_FILTER_START,
	/*    AL_FILTER_SET_DECAY,*/
	/*    AL_FILTER_SET_FC,*/
	AL_FILTER_SET_STATE,
	AL_FILTER_SET_VOLUME,
	AL_FILTER_SET_PAN,
	AL_FILTER_START_VOICE_ALT,
	AL_FILTER_START_VOICE,
	AL_FILTER_STOP_VOICE,
	AL_FILTER_SET_FXAMT,
	AL_FILTER_11,
	AL_FILTER_12,
	AL_FILTER_13
};

#define AL_MAX_RSP_SAMPLES      160

/*
 * buffer locations based on AL_MAX_RSP_SAMPLES
 */
#define AL_DECODER_IN	        0
#define	AL_RESAMPLER_OUT	0
#define AL_TEMP_0	        0
#define	AL_DECODER_OUT	        320
#define	AL_TEMP_1	        320
#define	AL_TEMP_2	        640
#define	AL_MAIN_L_OUT	        1088
#define	AL_MAIN_R_OUT	        1408
#define	AL_AUX_L_OUT	        1728
#define	AL_AUX_R_OUT	        2048

/*
 * filter types
 */
enum {
	AL_ADPCM,
	AL_RESAMPLE,
	AL_BUFFER,
	AL_SAVE,
	AL_ENVMIX,
	AL_FX,
	AL_AUXBUS,
	AL_MAINBUS
};

typedef struct ALParam_s {
	struct ALParam_s    *next;
	s32                 delta;
	s16                 type;
	union {
		f32             f;
		s32             i;
	} data;
	union {
		f32             f;
		s32             i;
	} moredata;
	union {
		f32             f;
		s32             i;
	} stillmoredata;
	union {
		f32             f;
		s32             i;
	} yetstillmoredata;
	s32 unk1c;
	s32 unk20;
#ifdef PLATFORM_64BIT
	u8 _pad_[8];
#endif
} ALParam;

typedef struct {
	struct ALParam_s            *next;
	s32                         delta;
	s16                         type;
	s16                         unity;  /* disable resampler */
	f32                         pitch;
	s16                         volume;
	ALPan                       pan;
	u8                          fxMix;
	u8                          unk14;
	u8                          unk15;
	f32                         unk18;
	s32                         samples;
	struct ALWaveTable_s        *wave;
} ALStartParamAlt;

typedef struct {
	struct ALParam_s            *next;
	s32                         delta;
	s16                         type;
	s16                         unity;  /* disable resampler */
	struct ALWaveTable_s        *wave;
} ALStartParam;

typedef struct {
	struct ALParam_s    *next;
	s32                 delta;
	s16                 type;
	struct PVoice_s     *pvoice;
} ALFreeParam;

typedef Acmd *(*ALCmdHandler)(void *, s16 *, s32, s32, Acmd *);
typedef s32   (*ALSetParam)(void *, s32, void *);

typedef struct ALFilter_s {
	struct ALFilter_s   *source;
	ALCmdHandler        handler;
	ALSetParam          setParam;
	s16                 inp;
	s16                 outp;
	s32                 type;
} ALFilter;

void    alFilterNew(ALFilter *f, ALCmdHandler h, ALSetParam s, s32 type);

#define AL_MAX_ADPCM_STATES     3

typedef struct {
	ALFilter                    filter;
	ADPCM_STATE                 *state;
	ADPCM_STATE                 *lstate;
	ALRawLoop                   loop;
	struct ALWaveTable_s        *table;
	s32                         bookSize;
	ALDMAproc                   dma;
	void                        *dmaState;
	s32                         sample;
	s32                         lastsam;
	s32                         first;
	s32                         memin;
} ALLoadFilter;

void    alLoadNew(ALLoadFilter *f, ALDMANew dma, ALHeap *hp);
Acmd    *alAdpcmPull(void *f, s16 *outp, s32 byteCount, s32 sampleOffset, Acmd *p);
Acmd    *alRaw16Pull(void *f, s16 *outp, s32 byteCount, s32 sampleOffset, Acmd *p);
s32     alLoadParam(void *filter, s32 paramID, void *param);

typedef struct ALResampler_s {
	ALFilter            filter;
	RESAMPLE_STATE      *state[2];
	f32                 ratio;
	s32			upitch;
	f32		        delta;
	s32			first;
	ALParam		*ctrlList;
	ALParam		*ctrlTail;
	s32                 motion;
} ALResampler;

typedef struct {
	s16		        fc;
	s16		        fgain;
	union {
		s16		fccoef[16];
		s64             force_aligned;
	} fcvec;
	s32			first;
	POLEF_STATE *fstate[2];
	s32 unk34;
} ALLowPass;

typedef struct {
	u32		input;
	u32		output;
	s16		ffcoef;
	s16		fbcoef;
	s16		gain;
	f32		rsinc;
	f32		rsval;
	s32		rsdelta;
	f32		rsgain;
	ALLowPass	*lp;
	ALResampler	*rs;
} ALDelay;

typedef s32   (*ALSetFXParam)(void *, s32, void *);
typedef struct {
	u32			length; // 0
	ALDelay		*delay; // 4
	u8			section_count; // 8
	struct ALFilter_s   filter;
	s16			*base[2]; // 20
	s16			*input[2]; // 28
} ALFx;

void    alFxNew(ALFx *r, ALSynConfig *c, ALHeap *hp);
Acmd    *alFxPull(void *f, s16 *outp, s32 out, s32 sampleOffset, Acmd *p);
s32     alFxParam(void *filter, s32 paramID, void *param);
s32     alFxParamHdl(void *filter, s32 paramID, void *param);

#define AL_MAX_MAIN_BUS_SOURCES       1
typedef struct ALMainBus_s {
	ALFilter            filter;
	s32                 sourceCount;
	s32                 maxSources;
	ALFilter            **sources;
} ALMainBus;

void    alMainBusNew(ALMainBus *m, void *ptr, s32 len);
Acmd    *alMainBusPull(void *f, s16 *outp, s32 outCount, s32 sampleOffset, Acmd *p);
s32     alMainBusParam(void *filter, s32 paramID, void *param);

#define AL_MAX_AUX_BUS_SOURCES       8
#define AL_MAX_AUX_BUS_FX	     1
typedef struct ALAuxBus_s {
	ALFilter            filter;
	s32                 sourceCount;
	s32                 maxSources;
	ALFilter            **sources;
	ALFx		fx[AL_MAX_AUX_BUS_FX];
} ALAuxBus;

void    alAuxBusNew(ALAuxBus *m, void *ptr, s32 len);
Acmd    *alAuxBusPull(void *f, s16 *outp, s32 outCount, s32 sampleOffset, Acmd *p);
s32     alAuxBusParam(void *filter, s32 paramID, void *param);

void    alResampleNew(ALResampler *r, ALHeap *hp);
Acmd    *alResamplePull(void *f, s16 *outp, s32 out, s32 sampleOffset, Acmd *p);
s32     alResampleParam(void *f, s32 paramID, void *param);

typedef struct ALSave_s {
	ALFilter            filter;
	s32	       		dramout;
	s32                 first;
} ALSave;

void    alSaveNew(ALSave *r);
Acmd    *alSavePull(void *f, s16 *outp, s32 outCount, s32 sampleOffset, Acmd *p);
s32     alSaveParam(void *f, s32 paramID, void *param);

typedef struct ALEnvMixer_s {
	ALFilter            filter;
	ENVMIX_STATE	*state;
	s16		        pan;
	s16		        volume;
	s16		        cvolL;
	s16		        cvolR;
	s16		        dryamt;
	s16		        wetamt;
	u16                 lratl;
	s16                 lratm;
	s16                 ltgt;
	u16                 rratl;
	s16                 rratm;
	s16                 rtgt;
	s32                 delta;
	s32                 segEnd;
	s32			first;
	ALParam		*ctrlList;
	ALParam		*ctrlTail;
	ALFilter            **sources;
	s32                 motion;
} ALEnvMixer;

void    alEnvmixerNew(ALEnvMixer *e, ALHeap *hp);
Acmd    *alEnvmixerPull(void *f, s16 *outp, s32 out, s32 sampleOffset, Acmd *p);
s32     alEnvmixerParam(void *filter, s32 paramID, void *param);


/*
 * heap stuff
 */
typedef struct {
	s32         magic;  /* check structure integrety                    */
	s32         size;   /* size of this allocated block                 */
	u8          *file;  /* file that this alloc was called from         */
	s32         line;   /* line that it was called from                 */
	s32         count;  /* heap call number                             */
	s32         pad0;
	s32         pad1;
	s32         pad2;   /* Make it 32 bytes                             */
} HeapInfo;

#define AL_CACHE_ALIGN  15

/*
 * synth stuff
 */

typedef struct PVoice_s {
	ALLink               node;
	struct ALVoice_s    *vvoice;
	ALFilter            *channelKnob;
	ALLoadFilter        decoder;
	ALResampler         resampler;
	ALEnvMixer		envmixer;
	s32                 offset;
} PVoice;

/*
 * prototypes for private driver functions
 */
ALParam         *__allocParam(void);
void            __freeParam(ALParam *param);
void            _freePVoice(ALSynth *drvr, PVoice *pvoice);
void            _collectPVoices(ALSynth *drvr);

s32             _timeToSamples(ALSynth *ALSynth, s32 micros);
ALMicroTime     _samplesToTime(ALSynth *synth, s32 samples);

void            _init_lpfilter(ALLowPass *lp);

#endif

