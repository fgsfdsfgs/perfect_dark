#include <os.h>
#include <os_internal.h>
#include "n_synthInternals.h"

#define RANGE 2.0f

Acmd *_n_loadOutputBuffer(ALFx *r, ALDelay *d, s32 arg2, s32 buff, Acmd *p);
Acmd *_n_loadBuffer(ALFx *r, s32 arg1, s16 *curr_ptr, s32 buff,s32 count, Acmd *p);
Acmd *_n_saveBuffer(ALFx *r, s32 arg1, s16 *curr_ptr, s32 buff, Acmd *p);
Acmd *_n_filterBuffer(ALLowPass *lp, s32 buff, s32 count, Acmd *p);
f32 _doModFunc(ALDelay *d, s32 count);

Acmd *n_alFxPull(s32 sampleOffset, Acmd *p, s32 arg2)
{
	Acmd *ptr = p;
	ALFx *r = (ALFx *)n_syn->auxBus[arg2].fx;
	s16 i, buff1, buff2, input, output;
	s16 *in_ptr, *out_ptr, *prev_out_ptr = 0;
	ALDelay *d;
	s32 sp58 = 1;
	u32 j;

	/*
	 * pull channels going into this effect first
	 */
	ptr = n_alAuxBusPull(sampleOffset, p, arg2, &sp58);

	input  = N_AL_AUX_L_OUT;
	output = N_AL_AUX_R_OUT;
	buff1  = N_AL_TEMP_0;
	buff2  = N_AL_TEMP_1;

	if (var8009c344[arg2] == 0) {
		aMix(ptr++, 0, 0xc000, N_AL_AUX_L_OUT, input);
		aMix(ptr++, 0, 0x4000, N_AL_AUX_R_OUT, input);
	}

	/* and write the mixed value to the delay line at r->input */
	ptr = _n_saveBuffer(r, 0, r->input[0], input, ptr);

	if (var8009c344[arg2]) {
		ptr = _n_saveBuffer(r, 1, r->input[1], 0x930, ptr);
	}

	for (j = 0; j <= var8009c344[arg2]; j++) {
		aClearBuffer(ptr++, output, FIXED_SAMPLE << 1); /* clear the AL_AUX_R_OUT */

		for (i = 0; i < r->section_count; i++) {
			d = &r->delay[i];  /* get the ALDelay structure */
			in_ptr = &r->input[j][(s32)-d->input];
			out_ptr = &r->input[j][(s32)-d->output];

			if (var8009c346[arg2] && var8009c344[arg2]) {
				d->ffcoef = -d->ffcoef;
				d->fbcoef = -d->fbcoef;
			}

			if (in_ptr == prev_out_ptr) {
				s16 t = buff2;
				buff2 = buff1;
				buff1 = t;
			} else {  /* load data at in_ptr into buff1 */
				ptr = _n_loadBuffer(r, j, in_ptr, buff1, FIXED_SAMPLE, ptr);
			}

			ptr = _n_loadOutputBuffer(r, d, j, buff2, ptr);

			if (d->ffcoef) {
				aMix(ptr++, 0, (u16)d->ffcoef, buff1, buff2);

				if (!d->rs && !d->lp) {
					ptr = _n_saveBuffer(r, j, out_ptr, buff2, ptr);
				}
			}

			if (d->fbcoef) {
				aMix(ptr++, 0, (u16)d->fbcoef, buff2, buff1);
				ptr = _n_saveBuffer(r, j, in_ptr, buff1, ptr);
			}

			if (d->lp) {
				ptr = _n_filterBuffer(d->lp, j, buff2, ptr);
			}

			if (!d->rs) {
				ptr = _n_saveBuffer(r, j, out_ptr, buff2, ptr);
			}

			if (d->gain) {
				if (var8009c344[arg2]) {
					aMix(ptr++, 0, (u16)d->gain, buff2, output);
				} else {
					u32 sp34 = d->gain * 1.4141999483109f;

					if (sp34 > 0x7fff) {
						sp34 = 0x7fff;
					}

					aMix(ptr++, 0, (u16)sp34, buff2, output);
				}
			}

			prev_out_ptr = &r->input[j][d->output];
		}

		if (var8009c344[arg2] && j == 0) {
			ptr = _n_loadBuffer(r, 1, r->input[1], input, FIXED_SAMPLE, ptr);

			if (var8009c346[arg2]) {
				aMix(ptr++, 0, 0x5a82, output, 0x650);
			} else {
				aMix(ptr++, 0, 0x5a82, output, 0x4e0);
			}
		} else {
			// empty
		}

		/*
		 * output already in AL_AUX_R_OUT
		 *      just copy to AL_AUX_L_OUT
		 */
		aDMEMMove(ptr++, output, N_AL_AUX_L_OUT, FIXED_SAMPLE << 1);

		/*
		 * bump the master delay line input pointer
		 * modulo the length
		 */
		r->input[j] += FIXED_SAMPLE;

		if (r->input[j] > &r->base[j][r->length]) {
			r->input[j] -= r->length;
		}
	}

	return ptr;
}

/*
 * This routine gets called by alSynSetFXParam. No checking takes place to
 * verify the validity of the paramID or the param value. input and output
 * values must be 8 byte aligned, so round down any param passed.
 */
s32 n_alFxParamHdl(void *filter, s32 paramID, void *param)
{
	ALFx *f = (ALFx *) filter;
	s32 p = paramID & 7;
	s32 s = paramID >> 3;
	s32 val = *(s32*)param;
	f32 rsgain;

	if (s >= f->section_count) {
		return 0;
	}

#define INPUT_PARAM         0
#define OUTPUT_PARAM        1
#define FBCOEF_PARAM        2
#define FFCOEF_PARAM        3
#define GAIN_PARAM          4
#define CHORUSRATE_PARAM    5
#define CHORUSDEPTH_PARAM   6
#define LPFILT_PARAM        7

	switch (p) {
	case INPUT_PARAM:
		f->delay[s].input = ((s32)val * n_syn->outputRate / 1000) & 0xfffffff8;
		break;
	case OUTPUT_PARAM:
		f->delay[s].output = ((s32)val * n_syn->outputRate / 1000) & 0xfffffff8;
		break;
	case FBCOEF_PARAM:
		f->delay[s].fbcoef = (s16)val;
		break;
	case FFCOEF_PARAM:
		f->delay[s].ffcoef = (s16)val;
		break;
	case GAIN_PARAM:
		f->delay[s].gain = (s16)val;
		break;
	case CHORUSRATE_PARAM:
		f->delay[s].rsinc = ((((f32)val)/1000) * RANGE)/n_syn->outputRate;
		break;
	case CHORUSDEPTH_PARAM:
		rsgain = val;
		break;
	case LPFILT_PARAM:
		if (f->delay[s].lp) {
			f->delay[s].lp->fc = (s16)val;
			_init_lpfilter(f->delay[s].lp);
		}
		break;
	}

	if (f->delay[s].input >= f->length - 16) {
		f->delay[s].input = f->length - 16;
	}

	if (f->delay[s].input >= f->length - 8) {
		f->delay[s].input = f->length - 8;
	}

	if (f->delay[s].input >= f->delay[s].output) {
		f->delay[s].output = f->delay[s].input + 8;
	}

	/**
	 * the following constant is derived from:
	 *
	 *      ratio = 2^(cents/1200)
	 *
	 * and therefore for hundredths of a cent
	 *                     x
	 *      ln(ratio) = ---------------
	 *              (120,000)/ln(2)
	 * where
	 *      120,000/ln(2) = 173123.40...
	 */
#define CONVERT 173123.404906676f
#define LENGTH  (f->delay[s].output - f->delay[s].input)

	if (f->delay[s].rs) {
		if (p != 6) {
			if (LENGTH != 0) {
				rsgain = (f32)f->delay[s].rsgain / (f->delay[s].output - f->delay[s].input) * CONVERT;
			} else {
				rsgain = 0;
			}
		}

		f->delay[s].rsgain = (f->delay[s].output - f->delay[s].input) * (rsgain / CONVERT);
	}

	return 0;
}

Acmd *_n_loadOutputBuffer(ALFx *r, ALDelay *d, s32 arg2, s32 buff, Acmd *p)
{
	Acmd *ptr = p;
	s32 ratio, count, rbuff = N_AL_TEMP_2;
	s16 *out_ptr;
	f32 fincount, fratio, delta;
	s32 ramalign = 0, length;
	s32 incount = FIXED_SAMPLE;
	s16 tmp;

	if (d->rs) {
		length = d->output - d->input;
		delta = _doModFunc(d, incount);
		delta /= length;
		delta = (s32)(delta * UNITY_PITCH);
		delta = delta / UNITY_PITCH;
		fratio = 1.0f - delta;
		fincount = d->rs->delta + (fratio * (f32)incount);
		count = (s32) fincount;
		d->rs->delta = fincount - (f32)count;
		out_ptr = &r->input[arg2][(s32)-(d->output - d->rsdelta)];
		ramalign = ((intptr_t)out_ptr & 0x7) >> 1;
		ptr = _n_loadBuffer(r, arg2, out_ptr - ramalign, rbuff, count + ramalign, ptr);

		ratio = (s32)(fratio * UNITY_PITCH);

		tmp = buff >> 8;
		n_aResample(ptr++, osVirtualToPhysical(d->rs->state[arg2]), d->rs->first, ratio, rbuff + (ramalign << 1), tmp);

		d->rs->first = 0;
		d->rsdelta += count - incount;
	} else {
		out_ptr = &r->input[arg2][(s32)-d->output];
		ptr = _n_loadBuffer(r, arg2, out_ptr, buff, FIXED_SAMPLE, ptr);
	}

	return ptr;
}

Acmd *_n_loadBuffer(ALFx *r, s32 arg1, s16 *curr_ptr, s32 buff,s32 count, Acmd *p)
{
	Acmd *ptr = p;
	s32 after_end, before_end;
	s16 *updated_ptr, *delay_end;

	delay_end = &r->base[arg1][r->length];

	if (curr_ptr < r->base[arg1]) {
		curr_ptr += r->length;
	}

	updated_ptr = curr_ptr + count;

	if (updated_ptr > delay_end) {
		after_end = updated_ptr - delay_end;
		before_end = delay_end - curr_ptr;

		n_aLoadBuffer(ptr++, before_end << 1, buff, osVirtualToPhysical(curr_ptr));
		n_aLoadBuffer(ptr++, after_end << 1, buff + (before_end << 1), osVirtualToPhysical(r->base[arg1]));
	} else {
		n_aLoadBuffer(ptr++, count << 1, buff, osVirtualToPhysical(curr_ptr));
	}

	return ptr;
}

Acmd *_n_saveBuffer(ALFx *r, s32 arg1, s16 *curr_ptr, s32 buff, Acmd *p)
{
	Acmd *ptr = p;
	s32 after_end, before_end;
	s16 *updated_ptr, *delay_end;

	delay_end = &r->base[arg1][r->length];

	if (curr_ptr < r->base[arg1]) {    /* probably just security */
		curr_ptr += r->length;         /* shouldn't occur */
	}

	updated_ptr = curr_ptr + FIXED_SAMPLE;

	if (updated_ptr > delay_end) { /* if the data wraps past end of r->base */
		after_end = updated_ptr - delay_end;
		before_end = delay_end - curr_ptr;

		n_aSaveBuffer(ptr++, before_end << 1, buff, osVirtualToPhysical(curr_ptr));
		n_aSaveBuffer(ptr++, after_end << 1, buff + (before_end << 1), osVirtualToPhysical(r->base[arg1]));
	} else {
		n_aSaveBuffer(ptr++, FIXED_SAMPLE << 1, buff, osVirtualToPhysical(curr_ptr));
	}

	return ptr;
}

Acmd *_n_filterBuffer(ALLowPass *lp, s32 buff, s32 count, Acmd *p)
{
	Acmd *ptr = p;
	s16 tmp = count >> 8;

	n_aLoadADPCM(ptr++, 32, osVirtualToPhysical(lp->fcvec.fccoef));
	n_aPoleFilter(ptr++, lp->first, lp->fgain, tmp, osVirtualToPhysical(lp->fstate[buff]));
	lp->first = 0;

	return ptr;
}

/**
 * Generate a triangle wave from -1 to 1, and find the current position
 * in the wave. (Rate of the wave is controlled by d->rsinc, which is chorus
 * rate) Multiply the current triangle wave value by d->rsgain, (chorus depth)
 * which is expressed in number of samples back from output pointer the chorus
 * should go at it's full chorus. In otherwords, this function returns a number
 * of samples the output pointer should modulate backwards.
 */
f32 _doModFunc(ALDelay *d, s32 count)
{
	f32 val;

	/*
	 * generate bipolar sawtooth
	 * from -RANGE to +RANGE
	 */
	d->rsval += d->rsinc * count;
	d->rsval = (d->rsval > RANGE) ? d->rsval - (RANGE * 2) : d->rsval;

	/*
	 * convert to monopolar triangle
	 * from 0 to RANGE
	 */
	val = d->rsval;
	val = (val < 0) ? -val : val;

	/*
	 * convert to bipolar triangle
	 * from -1 to 1
	 */
	val -= RANGE/2;

	return d->rsgain * val;
}
