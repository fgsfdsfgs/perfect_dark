#define gunscript_end \
	{ GUNCMD_END },

#define gunscript_showpart(keyframe, part) \
	{ GUNCMD_SHOWPART, 0, keyframe, part },

#define gunscript_hidepart(keyframe, part) \
	{ GUNCMD_HIDEPART, 0, keyframe, part },

#define gunscript_waitforzreleased(keyframe) \
	{ GUNCMD_WAITFORZRELEASED, 0, keyframe },

#define gunscript_waittime(keyframe, time) \
	{ GUNCMD_WAITTIME, 0, keyframe, time },

#define gunscript_playsound(keyframe, sound) \
	{ GUNCMD_PLAYSOUND, 0, keyframe, sound },

#define gunscript_include(unk1, address) \
	{ GUNCMD_INCLUDE, unk1, 0, (uintptr_t)address },

#define gunscript_random(probability, address) \
	{ GUNCMD_RANDOM, 0, probability, (uintptr_t)address },

#define gunscript_repeatuntilfull(triggerkey, dontloop, gototrigger) \
	{ GUNCMD_REPEATUNTILFULL, 0, triggerkey, (dontloop << 16) | gototrigger },

#define gunscript_popoutsackofpills(unk1) \
	{ GUNCMD_POPOUTSACKOFPILLS, 0, unk1 },

#define gunscript_playanimation(animation, direction, speed) \
	{ GUNCMD_PLAYANIMATION, 0, animation, (direction << 16) | speed },

#define gunscript_setsoundspeed(keyframe, speed) \
	{ GUNCMD_SETSOUNDSPEED, 0, keyframe, speed },

#define gunviscmd_end \
	{ GUNVISCMD_END },

#define gunviscmd_sethidden(modelpart) \
	{ GUNVISCMD_ALWAYSTRUE, 0, GUNVISOP_IFTRUE_SETHIDDEN, modelpart, 0 },

#define gunviscmd_checkupgrade(upgrade, operator, modelpart) \
	{ GUNVISCMD_CHECKUPGRADE, upgrade, operator, modelpart, 0 },

#define gunviscmd_checkinlefthand(operator, modelpart) \
	{ GUNVISCMD_CHECKINLEFTHAND, 0, operator, modelpart, 0 },

#define gunviscmd_checkinrighthand(operator, modelpart) \
	{ GUNVISCMD_CHECKINRIGHTHAND, 0, operator, modelpart, 0 },
