#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "lib/snd.h"
#include "lib/lib_317f0.h"
#include "data.h"
#include "types.h"

void psStop(void)
{
	if (g_SndDisabled) return;
	s32 i;

	for (i = 0; i < (IS4MB() ? 30 : 40); i++) {
		if (g_PsChannels[i].flags & PSFLAG_ISMP3) {
			sndStopMp3(g_PsChannels[i].soundnum26);
			g_PsChannels[i].flags &= ~PSFLAG_ISMP3;
			g_PsChannels[i].flags &= ~PSFLAG_REPEATING;
		} else if (g_PsChannels[i].audiohandle && sndGetState(g_PsChannels[i].audiohandle) != AL_STOPPED) {
			audioStop(g_PsChannels[i].audiohandle);
			g_PsChannels[i].flags &= ~PSFLAG_REPEATING;
		}
	}
}
