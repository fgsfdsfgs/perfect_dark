#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libaudio.h"

#include "preprocess/common.h"

struct n64_adpcm_waveinfo {
	u32 loop; // ptr to ALADPCMloop
	u32 book; // ptr to ALADPCMBook
};

struct n64_raw_waveinfo {
	u32 loop; // ptr to ALRawLoop
};

struct n64_wavetable {
	u32 base;
	s32 len;
	u8 type;
	u8 flags;
	union {
		struct n64_adpcm_waveinfo adpcmWave;
		struct n64_raw_waveinfo rawWave;
	} waveInfo;
};

struct n64_sound {
	u32 envelope; // ptr to ALEnvelope
	u32 keyMap; // ptr to ALKeyMap
	u32 wavetable; // ptr to struct n64_wavetable
	u8 samplePan;
	u8 sampleVolume;
	u8 flags;
};

struct n64_instrument {
	u8 volume;
	u8 pan;
	u8 priority;
	u8 flags;
	u8 tremType;
	u8 tremRate;
	u8 tremDepth;
	u8 tremDelay;
	u8 vibType;
	u8 vibRate;
	u8 vibDepth;
	u8 vibDelay;
	s16 bendRange;
	s16 soundCount;
	u32 soundArray[1]; // ptr to struct n64_sound
};

struct n64_bank {
	s16 instCount;
	u8 flags;
	u8 pad;
	s32 sampleRate;
	u32 percussion; // ptr to struct n64_instrument
	u32 instArray[1]; // ptr to struct n64_instrument
};

struct n64_bankfile {
	s16 revision;
	s16 bankCount;
	u32 bankArray[1]; // ptr to struct n64_bank
};

// only proceeds to convert the next item if it's not already converted
#define AL_NEXT_ITEM(field, func) { \
	struct ptrmarker *marker = ptrFind(srcpos); \
	if (marker == NULL) { \
		field = (void *)(uintptr_t)(dstpos); \
		ptrAdd(srcpos, (uintptr_t)(field)); \
		dstpos = func(dst, dstpos, src, srcpos); \
	} else { \
		field = (void *)marker->ptr_host; } \
}

static u32 convertAudioEnvelope(u8 *dst, u32 dstpos, u8 *src, u32 srcpos)
{
	ALEnvelope *n64_envelope = (ALEnvelope *) &src[srcpos];
	ALEnvelope *host_envelope = (ALEnvelope *) &dst[dstpos];

	dstpos += sizeof(ALEnvelope);

	host_envelope->attackTime = PD_BE32(n64_envelope->attackTime);
	host_envelope->decayTime = PD_BE32(n64_envelope->decayTime);
	host_envelope->releaseTime = PD_BE32(n64_envelope->releaseTime);
	host_envelope->attackVolume = n64_envelope->attackVolume;
	host_envelope->decayVolume = n64_envelope->decayVolume;

	return dstpos;
}

static u32 convertAudioKeyMap(u8 *dst, u32 dstpos, u8 *src, u32 srcpos)
{
	ALKeyMap *n64_keymap = (ALKeyMap *) &src[srcpos];
	ALKeyMap *host_keymap = (ALKeyMap *) &dst[dstpos];

	dstpos += sizeof(ALKeyMap);

	host_keymap->velocityMin = n64_keymap->velocityMin;
	host_keymap->velocityMax = n64_keymap->velocityMax;
	host_keymap->keyMin = n64_keymap->keyMin;
	host_keymap->keyMax = n64_keymap->keyMax;
	host_keymap->keyBase = n64_keymap->keyBase;
	host_keymap->detune = n64_keymap->detune;

	return dstpos;
}

static u32 convertAudioAdpcmLoop(u8 *dst, u32 dstpos, u8 *src, u32 srcpos)
{
	ALADPCMloop *n64_loop = (ALADPCMloop *) &src[srcpos];
	ALADPCMloop *host_loop = (ALADPCMloop *) &dst[dstpos];

	dstpos += sizeof(ALADPCMloop);

	host_loop->start = PD_BE32(n64_loop->start);
	host_loop->end = PD_BE32(n64_loop->end);
	host_loop->count = PD_BE32(n64_loop->count);

	for (int i = 0; i < ARRAYCOUNT(host_loop->state); i++) {
		host_loop->state[i] = PD_BE16(n64_loop->state[i]);
	}

	return dstpos;
}

static u32 convertAudioAdpcmBook(u8 *dst, u32 dstpos, u8 *src, u32 srcpos)
{
	ALADPCMBook *n64_book = (ALADPCMBook *) &src[srcpos];
	ALADPCMBook *host_book = (ALADPCMBook *) &dst[dstpos];

	dstpos += sizeof(ALADPCMBook);

	host_book->order = PD_BE32(n64_book->order);
	host_book->npredictors = PD_BE32(n64_book->npredictors);

	for (int i = 0; i < ARRAYCOUNT(host_book->book); i++) {
		host_book->book[i] = PD_BE16(n64_book->book[i]);
	}

	return dstpos;
}

static u32 convertAudioRawLoop(u8 *dst, u32 dstpos, u8 *src, u32 srcpos)
{
	ALRawLoop *n64_loop = (ALRawLoop *) &src[srcpos];
	ALRawLoop *host_loop = (ALRawLoop *) &dst[dstpos];

	dstpos += sizeof(ALRawLoop);

	host_loop->start = PD_BE32(n64_loop->start);
	host_loop->end = PD_BE32(n64_loop->end);
	host_loop->count = PD_BE32(n64_loop->count);

	return dstpos;
}

static u32 convertAudioWaveTable(u8 *dst, u32 dstpos, u8 *src, u32 srcpos)
{
	struct n64_wavetable *n64_wavetable = (struct n64_wavetable *) &src[srcpos];
	ALWaveTable *host_wavetable = (ALWaveTable *) &dst[dstpos];

	dstpos += sizeof(ALWaveTable);

	host_wavetable->base = (void *)(uintptr_t)PD_BE32(n64_wavetable->base);
	host_wavetable->len = PD_BE32(n64_wavetable->len);
	host_wavetable->type = n64_wavetable->type;
	host_wavetable->flags = n64_wavetable->flags;

	if (host_wavetable->type == AL_ADPCM_WAVE) {
		if (n64_wavetable->waveInfo.adpcmWave.loop) {
			srcpos = PD_BE32(n64_wavetable->waveInfo.adpcmWave.loop);
			AL_NEXT_ITEM(host_wavetable->waveInfo.adpcmWave.loop, convertAudioAdpcmLoop);
		} else {
			host_wavetable->waveInfo.adpcmWave.loop = NULL;
		}

		if (n64_wavetable->waveInfo.adpcmWave.book) {
			srcpos = PD_BE32(n64_wavetable->waveInfo.adpcmWave.book);
			AL_NEXT_ITEM(host_wavetable->waveInfo.adpcmWave.book, convertAudioAdpcmBook);
		} else {
			host_wavetable->waveInfo.adpcmWave.book = NULL;
		}
	} else if (host_wavetable->type == AL_RAW16_WAVE) {
		if (n64_wavetable->waveInfo.rawWave.loop) {
			srcpos = PD_BE32(n64_wavetable->waveInfo.rawWave.loop);
			AL_NEXT_ITEM(host_wavetable->waveInfo.rawWave.loop, convertAudioRawLoop);
		} else {
			host_wavetable->waveInfo.rawWave.loop = NULL;
		}
	}

	return dstpos;
}

static u32 convertAudioSound(u8 *dst, u32 dstpos, u8 *src, u32 srcpos)
{
	struct n64_sound *n64_sound = (struct n64_sound *) &src[srcpos];
	ALSound *host_sound = (ALSound *) &dst[dstpos];

	dstpos += sizeof(ALSound);

	if (n64_sound->envelope) {
		srcpos = PD_BE32(n64_sound->envelope);
		AL_NEXT_ITEM(host_sound->envelope, convertAudioEnvelope);
	} else {
		host_sound->envelope = NULL;
	}

	if (n64_sound->keyMap) {
		srcpos = PD_BE32(n64_sound->keyMap);
		AL_NEXT_ITEM(host_sound->keyMap, convertAudioKeyMap);
	} else {
		host_sound->keyMap = NULL;
	}

	if (n64_sound->wavetable) {
		srcpos = PD_BE32(n64_sound->wavetable);
		AL_NEXT_ITEM(host_sound->wavetable, convertAudioWaveTable);
	} else {
		host_sound->wavetable = NULL;
	}

	host_sound->samplePan = n64_sound->samplePan;
	host_sound->sampleVolume = n64_sound->sampleVolume;
	host_sound->flags = n64_sound->flags;

	return dstpos;
}

static u32 convertAudioInstrument(u8 *dst, u32 dstpos, u8 *src, u32 srcpos)
{
	struct n64_instrument *n64_instrument = (struct n64_instrument *) &src[srcpos];
	ALInstrument *host_instrument = (ALInstrument *) &dst[dstpos];
	const s16 soundCount = PD_BE16(n64_instrument->soundCount);

	host_instrument->volume = n64_instrument->volume;
	host_instrument->pan = n64_instrument->pan;
	host_instrument->priority = n64_instrument->priority;
	host_instrument->flags = n64_instrument->flags;
	host_instrument->tremType = n64_instrument->tremType;
	host_instrument->tremRate = n64_instrument->tremRate;
	host_instrument->tremDepth = n64_instrument->tremDepth;
	host_instrument->tremDelay = n64_instrument->tremDelay;
	host_instrument->vibType = n64_instrument->vibType;
	host_instrument->vibRate = n64_instrument->vibRate;
	host_instrument->vibDepth = n64_instrument->vibDepth;
	host_instrument->vibDelay = n64_instrument->vibDelay;
	host_instrument->bendRange = PD_BE16(n64_instrument->bendRange);
	host_instrument->soundCount = (soundCount);

	dstpos = dstpos + sizeof(ALInstrument) + sizeof(uintptr_t) * (soundCount - 1);

	for (int i = 0; i < soundCount; i++) {
		srcpos = PD_BE32(n64_instrument->soundArray[i]);
		AL_NEXT_ITEM(host_instrument->soundArray[i], convertAudioSound);
	}

	return dstpos;
}

static u32 convertAudioBank(u8 *dst, u32 dstpos, u8 *src, u32 srcpos)
{
	struct n64_bank *n64_bank = (struct n64_bank *) &src[srcpos];
	ALBank *host_bank = (ALBank *) &dst[dstpos];
	const s16 instCount = PD_BE16(n64_bank->instCount);

	host_bank->instCount = (instCount);
	host_bank->flags = n64_bank->flags;
	host_bank->pad = n64_bank->pad;
	host_bank->sampleRate = PD_BE32(n64_bank->sampleRate);

	dstpos = dstpos + sizeof(ALBank) + sizeof(uintptr_t) * (instCount - 1);

	if (n64_bank->percussion) {
		srcpos = PD_BE32(n64_bank->percussion);
		AL_NEXT_ITEM(host_bank->percussion, convertAudioInstrument);
	} else {
		host_bank->percussion = 0;
	}

	for (int i = 0; i < instCount; i++) {
		srcpos = PD_BE32(n64_bank->instArray[i]);
		AL_NEXT_ITEM(host_bank->instArray[i], convertAudioInstrument);
	}

	return dstpos;
}

static u32 convertAudioBankFile(u8 *dst, u8 *src)
{
	struct n64_bankfile *n64_bankfile = (struct n64_bankfile *)src;
	ALBankFile *host_bankfile = (ALBankFile *)dst;
	const s16 bankCount = PD_BE16(n64_bankfile->bankCount);

	host_bankfile->revision = PD_BE16(n64_bankfile->revision);
	host_bankfile->bankCount = (bankCount);

	u32 dstpos = sizeof(ALBankFile) + sizeof(uintptr_t) * (bankCount - 1);

	for (int i = 0; i < bankCount; i++) {
		host_bankfile->bankArray[i] = (void *)(uintptr_t)(dstpos);
		u32 srcpos = PD_BE32(n64_bankfile->bankArray[i]);
		dstpos = convertAudioBank(dst, dstpos, src, srcpos);
	}

	return dstpos;
}

u8 *preprocessALBankFile(u8 *src, u32 size, u32 *outSize)
{
	ptrReset();

	const u32 dstlen = size * 3; // this should overshoot any possible bank size, but * 2 also works for vanilla banks
	u8 *dst = sysMemZeroAlloc(dstlen);

	u32 reallen = convertAudioBankFile(dst, src);
	if (reallen > dstlen || ALIGN16(reallen) > dstlen) {
		sysFatalError("overflow when trying to preprocess an ALBankFile, size %u dstlen %u reallen %u", size, dstlen, reallen);
	}

	reallen = ALIGN16(reallen);

	if (reallen < dstlen) {
		dst = sysMemRealloc(dst, reallen);
	}

	*outSize = reallen;

	return dst;
}


u8 *preprocessALCMidiHdr(u8 *data, u32 size, u32 *outSize)
{
	ALCMidiHdr *hdr = (ALCMidiHdr *)data;
	PD_SWAP_VAL(hdr->division);
	for (s32 i = 0; i < ARRAYCOUNT(hdr->trackOffset); ++i) {
		PD_SWAP_VAL(hdr->trackOffset[i]);
	}
	return NULL;
}

u8 *preprocessSequences(u8* data, u32 size, u32 *outSize)
{
	struct seqtable *seq = (struct seqtable *)data;
	PD_SWAP_VAL(seq->count);

	for (s16 i = 0; i < seq->count; ++i) {
		PD_SWAP_VAL(seq->entries[i].binlen);
		PD_SWAP_VAL(seq->entries[i].ziplen);
		PD_SWAP_VAL(seq->entries[i].romaddr);
	}

	return NULL;
}
