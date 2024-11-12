#include <ultra64.h>
#include "constants.h"
#include "game/atan2f.h"
#include "game/tex.h"
#include "game/playermgr.h"
#include "game/bg.h"
#include "game/texdecompress.h"
#include "bss.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

s32 texGetMask(s32 value)
{
	if (value < 2) {
		return 0;
	}

	if (value < 3) {
		return 1;
	}

	if (value < 5) {
		return 2;
	}

	if (value < 9) {
		return 3;
	}

	if (value < 17) {
		return 4;
	}

	if (value < 33) {
		return 5;
	}

	if (value < 65) {
		return 6;
	}

	if (value < 129) {
		return 7;
	}

	return 8;
}

s32 tex0f0b33f8(s32 width, s32 height, s32 lod)
{
	s32 sum = 0;

	if (lod <= 0) {
		lod = 1;
	}

	while (lod > 0) {
		sum += (width + 15) / 16 * 4 * height;

		lod--;

		if (width >= 2) {
			width >>= 1;
		}

		if (height >= 2) {
			height >>= 1;
		}
	}

	return sum;
}

s32 tex0f0b3468(s32 width, s32 height, s32 lod)
{
	s32 sum = 0;

	if (lod <= 0) {
		lod = 1;
	}

	while (lod > 0) {
		sum += (width + 7) / 8 * 4 * height;

		lod--;

		if (width >= 2) {
			width >>= 1;
		}

		if (height >= 2) {
			height >>= 1;
		}
	}

	return sum;
}

s32 tex0f0b34d8(s32 width, s32 height, s32 lod)
{
	s32 sum = 0;

	if (lod <= 0) {
		lod = 1;
	}

	while (lod > 0) {
		sum += (width + 3) / 4 * 4 * height;

		lod--;

		if (width >= 2) {
			width >>= 1;
		}

		if (height >= 2) {
			height >>= 1;
		}
	}

	return sum;
}

s32 tex0f0b3548(s32 width, s32 height, s32 lod)
{
	s32 sum = 0;

	if (lod <= 0) {
		lod = 1;
	}

	while (lod > 0) {
		sum += (width + 3) / 4 * 4 * height;

		lod--;

		if (width >= 2) {
			width >>= 1;
		}

		if (height >= 2) {
			height >>= 1;
		}
	}

	return sum;
}

void texSetRenderMode(Gfx **gdlptr, s32 arg1, s32 numcycles, s32 arg3)
{
	Gfx *gdl = *gdlptr;

	if (numcycles == 1) {
		gDPPipeSync(gdl++);
		gDPSetCycleType(gdl++, G_CYC_1CYCLE);

		switch (arg1) {
		default:
		case 1:
			if (arg3) {
				if (arg3 >= 2) {
					gDPSetRenderMode(gdl++, G_RM_AA_ZB_OPA_DECAL, G_RM_AA_ZB_OPA_DECAL2);
				} else {
					gDPSetRenderMode(gdl++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
				}
			} else {
				gDPSetRenderMode(gdl++, G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2);
			}
			break;
		case 2:
			if (arg3) {
				if (arg3 >= 2) {
					gDPSetRenderMode(gdl++, G_RM_AA_ZB_XLU_DECAL, G_RM_AA_ZB_XLU_DECAL2);
				} else {
					gDPSetRenderMode(gdl++, G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2);
				}
			} else {
				gDPSetRenderMode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
			}
			break;
		case 3:
			if (arg3) {
				gDPSetRenderMode(gdl++, G_RM_AA_ZB_TEX_EDGE, G_RM_AA_ZB_TEX_EDGE2);
			} else {
				gDPSetRenderMode(gdl++, G_RM_AA_TEX_EDGE, G_RM_AA_TEX_EDGE2);
			}
			break;
		case 4:
			if (arg3) {
				gDPSetRenderMode(gdl++, G_RM_ZB_CLD_SURF, G_RM_ZB_CLD_SURF2);
			} else {
				gDPSetRenderMode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
			}
			break;
		}
	} else {
		gDPPipeSync(gdl++);
		gDPSetCycleType(gdl++, G_CYC_2CYCLE);

		switch (arg1) {
		default:
		case 1:
			if (arg3) {
				if (arg3 >= 2) {
					gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_AA_ZB_OPA_DECAL2);
				} else {
					gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_AA_ZB_OPA_SURF2);
				}
			} else {
				gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_AA_OPA_SURF2);
			}
			break;
		case 2:
			if (arg3) {
				if (arg3 >= 2) {
					gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_AA_ZB_XLU_DECAL2);
				} else {
					gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_AA_ZB_XLU_SURF2);
				}
			} else {
				gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_AA_XLU_SURF2);
			}
			break;
		case 3:
			if (arg3) {
				gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_AA_ZB_TEX_EDGE2);
			} else {
				gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_AA_TEX_EDGE2);
			}
			break;
		case 4:
			if (arg3) {
				gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_ZB_CLD_SURF2);
			} else {
				gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_CLD_SURF2);
			}
			break;
		}
	}

	*gdlptr = gdl;
}

void texLoadFromConfig(struct textureconfig *config)
{
	if ((u32)config->texturenum < NUM_TEXTURES) {
		texLoadFromConfigs(config, 1, NULL, 0);
	}
}

void texSelect(Gfx **gdlptr, struct textureconfig *tconfig, u32 arg2, s32 arg3, u32 ulst, bool arg5, struct texpool *pool)
{
	struct tex *tex;
	Gfx *gdl;
	s32 tile;

	gdl = *gdlptr;

	if (tconfig == NULL) {
		texSetRenderMode(&gdl, arg2, 1, arg3);

		if (arg3 >= 2) {
			gSPTextureL(gdl++, 0xffff, 0xffff, 0, arg3, G_TX_RENDERTILE, G_ON);
		} else {
			gSPTextureL(gdl++, 0xffff, 0xffff, 0, 0, G_TX_RENDERTILE, G_ON);
		}

		gDPSetCombineMode(gdl++, G_CC_SHADE, G_CC_SHADE);
	} else {
		s32 width = tconfig->width;
		s32 height = tconfig->height;
		u16 *ptr;
		s32 index;
		u8 format;
		u8 depth;
		s32 lutmode;
		s32 depth2;
		s32 lrs;
		s32 line;
		u16 texturenum;

		tex = NULL;

		if ((u32)tconfig->texturenum < NUM_TEXTURES) {
			texLoadFromConfigs(tconfig, 1, pool, 0);
		}

		if (tconfig->unk0b == 1) {
			ptr = (u16 *)tconfig->textureptr;
			texturenum = ((u16 *)PHYS_TO_K0(ptr))[-4];

			// GCC has problems with this area because it seems to think that
			// registers are 64 bits wide. To do the index < g_TexNumConfigs
			// comparison, it shifts index left and adds to it repeatedly,
			// creating a 64-bit value, then uses a neg before the comparison.
			// When a tconfig is passed that is not in the preset configs list,
			// the index < g_TexNumConfigs check passes when it shouldn't,
			// causing a read from g_TexWords using an invalid index.
			// Casting the pointers to integers makes gcc emit different code
			// which works around this issue.
#ifdef __sgi
			index = tconfig - g_TexWallhitConfigs;
#else
			index = ((uintptr_t) tconfig - (uintptr_t) g_TexWallhitConfigs) / sizeof(struct textureconfig);
#endif

			if (index >= 0 && index < g_TexNumConfigs) {
				tex = g_TexWords[index];
			}

			if (tex == NULL) {
				tex = texFindInPool(texturenum, pool);

				if (index >= 0 && index < g_TexNumConfigs) {
					g_TexWords[index] = tex;
				}
			}
		}

		if (tconfig->level == 0) {
			if (tex) {
				format = tex->gbiformat;
				depth = tex->depth;
				lutmode = tex->lutmodeindex << G_MDSFT_TEXTLUT;
			} else {
				format = tconfig->format;
				depth = tconfig->depth;
			}

			switch (depth) {
			default:
				break;
			case G_IM_SIZ_32b:
				depth2 = G_IM_SIZ_32b;
				lrs = tex0f0b3548(width, height, 1) - 1;
				line = (width + 3) >> 2;
				break;
			case G_IM_SIZ_16b:
				depth2 = G_IM_SIZ_16b;
				lrs = tex0f0b34d8(width, height, 1) - 1;
				line = (width + 3) >> 2;
				break;
			case G_IM_SIZ_8b:
				depth2 = G_IM_SIZ_16b;
				lrs = tex0f0b3468(width, height, 1) - 1;
				line = (width + 7) >> 3;
				break;
			case G_IM_SIZ_4b:
				depth2 = G_IM_SIZ_16b;
				lrs = tex0f0b33f8(width, height, 1) - 1;
				line = (width + 15) >> 4;
				break;
			}

			if (arg5) {
				texSetRenderMode(&gdl, arg2, 1, arg3);

				if (arg3 >= 2) {
					gSPTextureL(gdl++, 0xffff, 0xffff, 0, arg3, G_TX_RENDERTILE, G_ON);
				} else {
					gSPTextureL(gdl++, 0xffff, 0xffff, 0, 0, G_TX_RENDERTILE, G_ON);
				}

				gDPSetTextureLOD(gdl++, G_TL_TILE);

				switch (format) {
				case G_IM_FMT_RGBA:
					gDPSetCombineMode(gdl++, G_CC_MODULATEIA, G_CC_MODULATEIA);
					break;
				case G_IM_FMT_IA:
					gDPSetCombineMode(gdl++, G_CC_MODULATEIA, G_CC_MODULATEIA);
					break;
				case G_IM_FMT_I:
					gDPSetCombineMode(gdl++, G_CC_MODULATEI, G_CC_MODULATEI);
					break;
				case G_IM_FMT_CI:
					switch (lutmode) {
					case G_TT_RGBA16:
						gDPSetCombineMode(gdl++, G_CC_MODULATEIA, G_CC_MODULATEIA);
						break;
					case G_TT_IA16:
						gDPSetCombineMode(gdl++, G_CC_MODULATEIA, G_CC_MODULATEIA);
						break;
					}
					break;
				}
			}

			gDPSetTextureImage(gdl++, format, depth2, 1, tconfig->textureptr);

			if (depth2 == G_IM_SIZ_16b) {
				gDPLoadSync(gdl++);
				gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, lrs, 0);
			} else {
				gDPSetTile(gdl++, G_IM_FMT_RGBA, depth2, 0, 0x0000, 5, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
				gDPLoadSync(gdl++);
				gDPLoadBlock(gdl++, 5, 0, 0, lrs, 0);
			}

			gDPPipeSync(gdl++);

			if (format == G_IM_FMT_CI) {
				u32 a3 = lrs + 1;
				u32 t0 = (0x3ff - tex->unk0a) < a3 ? (0x3ff - tex->unk0a) : 0;

				a3 -= t0;

				gDPLoadSync(gdl++);
				gDPLoadTLUT06(gdl++, a3, t0, tex->unk0a + a3, t0);
				gDPPipeSync(gdl++);

				if (arg5) {
					gDPSetTextureLUT(gdl++, lutmode);
				}
			} else {
				if (arg5) {
					gDPSetTextureLUT(gdl++, G_TT_NONE);
				}
			}

			if (arg5) {
				gDPSetTile(gdl++, format, depth, line, 0x0000, G_TX_RENDERTILE, 0,
						tconfig->t, texGetMask(height), G_TX_NOLOD,
						tconfig->s, texGetMask(width), G_TX_NOLOD);

				gDPSetTileSize(gdl++, G_TX_RENDERTILE, ulst, ulst, ((width - 1) << 2) + ulst, ((height - 1) << 2) + ulst);
			}
		} else {
			s32 tmem = 0;
			s32 lod = tconfig->level;
			u8 format;
			u8 depth;
			s32 lutmode;
			s32 depth2;
			s32 lrs;

			if (lod > 6) {
				lod = 6;
			}

			if (tex) {
				format = tex->gbiformat;
				depth = tex->depth;
				lutmode = tex->lutmodeindex << G_MDSFT_TEXTLUT;
			} else {
				format = tconfig->format;
				depth = tconfig->depth;
			}

			if (tex && tex->hasloddata) {
				texGetDepthAndSize(tex, &depth2, &lrs);
			} else {
				switch (depth) {
				case G_IM_SIZ_32b:
					depth2 = G_IM_SIZ_32b;
					lrs = tex0f0b3548(width, height, lod) - 1;
					break;
				case G_IM_SIZ_16b:
					depth2 = G_IM_SIZ_16b;
					lrs = tex0f0b34d8(width, height, lod) - 1;
					break;
				case G_IM_SIZ_8b:
					depth2 = G_IM_SIZ_16b;
					lrs = tex0f0b3468(width, height, lod) - 1;
					break;
				case G_IM_SIZ_4b:
					depth2 = G_IM_SIZ_16b;
					lrs = tex0f0b33f8(width, height, lod) - 1;
					break;
				}
			}

			if (arg5) {
				texSetRenderMode(&gdl, arg2, 2, arg3);

				if (arg3 >= 2) {
					gSPTextureL(gdl++, 0xffff, 0xffff, lod - 1, arg3, G_TX_RENDERTILE, G_ON);
				} else {
					gSPTexture(gdl++, 0xffff, 0xffff, lod - 1, G_TX_RENDERTILE, G_ON);
				}

				gDPSetTextureLOD(gdl++, G_TL_LOD);

				switch (format) {
				case G_IM_FMT_RGBA:
					gDPSetCombineMode(gdl++, G_CC_TRILERP, G_CC_MODULATEIA2);
					break;
				case G_IM_FMT_IA:
					gDPSetCombineMode(gdl++, G_CC_TRILERP, G_CC_MODULATEIA2);
					break;
				case G_IM_FMT_I:
					gDPSetCombineMode(gdl++, G_CC_TRILERP, G_CC_MODULATEI2);
					break;
				case G_IM_FMT_CI:
					switch (lutmode) {
					case G_TT_RGBA16:
						gDPSetCombineMode(gdl++, G_CC_MODULATEIA, G_CC_MODULATEIA);
						break;
					case G_TT_IA16:
						gDPSetCombineMode(gdl++, G_CC_MODULATEIA, G_CC_MODULATEIA);
						break;
					}
					break;
				}
			}

			gDPSetTextureImage(gdl++, format, depth2, 1, tconfig->textureptr);

			if (depth2 == G_IM_SIZ_16b) {
				gDPLoadSync(gdl++);
				gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, lrs, 0);
			} else {
				gDPSetTile(gdl++, G_IM_FMT_RGBA, depth2, 0, 0x0000, 5, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
				gDPLoadSync(gdl++);
				gDPLoadBlock(gdl++, 5, 0, 0, lrs, 0);
			}

			gDPPipeSync(gdl++);

			if (format == G_IM_FMT_CI) {
				u32 a2 = lrs + 1;
				u32 a3 = (0x3ff - tex->unk0a) < a2 ? (0x3ff - tex->unk0a) : 0;

				a2 -= a3;

				gDPLoadSync(gdl++);
				gDPLoadTLUT06(gdl++, a2, a3, tex->unk0a + a2, a3);
				gDPPipeSync(gdl++);

				if (arg5) {
					gDPSetTextureLUT(gdl++, lutmode);
				}
			} else {
				if (arg5) {
					gDPSetTextureLUT(gdl++, G_TT_NONE);
				}
			}

			for (tile = 0; tile < lod; tile++) {
				s32 line;

				if (tile > 0) {
					if (tex && tex->hasloddata) {
						width = texGetWidthAtLod(tex, tile);
						height = texGetHeightAtLod(tex, tile);
					} else {
						if (width >= 2) {
							width >>= 1;
						}

						if (height >= 2) {
							height >>= 1;
						}
					}
				}

				switch (depth) {
				default:
					break;
				case G_IM_SIZ_32b:
					line = (width + 3) / 4;
					break;
				case G_IM_SIZ_16b:
					line = (width + 3) / 4;
					break;
				case G_IM_SIZ_8b:
					line = (width + 7) / 8;
					break;
				case G_IM_SIZ_4b:
					line = (width + 15) / 16;
					break;
				}

				if (arg5) {
					gDPSetTile(gdl++, format, depth, line, tmem, tile, 0,
							tconfig->t, texGetMask(height), tile,
							tconfig->s, texGetMask(width), tile);

					gDPSetTileSize(gdl++, tile, ulst, ulst, ((width - 1) << 2) + ulst, ((height - 1) << 2) + ulst);
				}

				tmem += line * height;
			}
		}
	}

	*gdlptr = gdl;
}
