#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "data.h"
#include "lib/model.h"
#include "lib/mtx.h"
#include "types.h"
#include "platform.h"

struct t0slot {
	u16 unk00;
	u16 unk02;
	u16 unk04;
	u16 unk06;
};

struct t0slot *t0slot;
u8 *t2ptr8;
s32 t3;
u8 *t3ptr8;
s32 t4;
u8 *t6ptr8;
s32 v1;
s32 s0;
s32 s1;
s32 s2;
s32 s3;
s32 s4;
s32 sr8;
s32 f0int;
f32 f0;
f32 f1;
f32 f2;
f32 f3;
f32 f4;
f32 f5;

// f12-f23 are used as rotation matrix
f32 f12;
f32 f13;
f32 f14;
f32 f15;
f32 f16;
f32 f17;
f32 f18;
f32 f19;
f32 f20;
f32 f21;
f32 f22;
f32 f23;

s32 gp;

static bool modelasmIterateThings1(void);
static bool modelasmIterateThings2(void);
static u32 modelasmReadFrameData(void);
static union modelrwdata *modelasmGetNodeRwData(struct model *model, struct modelnode *node, bool is_head);
static void modelasmMathPain1(f32 f30);
static void modelasmMathPain2(void);
static void modelasmPrepareRotMtx180(s32 t2, s32 t3, s32 t4);
static void modelasmPrepareRotMtx360(s32 t2, s32 t3, s32 t4);
static void modelasmMathPain3(void);
static void modelasmMathPain4(void);
static void modelasmMtxMultiply(Mtxf *src, Mtxf *dst);
static Mtxf *modelasmFindNodeMtx(struct model *model, struct modelnode *node);
static f32 modelasmAcosOrAsin(f32 f6);

/**
 * Reads animation data for the given model and applies matrix transformations
 * for each part. It factors merging between two animations too.
 *
 * The game code is able to set g_ModelJointPositionedFunc to a callback,
 * and this function will execute that callback for every positioned joint.
 *
 * Returns true if succeeded, or false if the caller should do some kind of
 * simple/generic calculation.
 */
bool modelasm00018680(struct modelrenderdata *renderdata, struct model *model)
{
	bool sp7f8 = false;
	f32 sp7e8f32;
	struct t0slot *sp7e8slot;
	f32 sp7f4;
	f32 sp7f0;
	f32 sp7ec;
	bool sp7e4;
	struct t0slot sp00[240];
	struct modelnode *node;
	struct anim *anim;
	struct skeleton *skeleton;
	struct modeldef *modeldef;
	union modelrwdata *rwdata;
	f32 f6;
	f32 f7;
	f32 f8;
	f32 f9;
	f32 f10;
	f32 f30;
	Mtxf *t0mtx;
	Mtxf *t1mtx;
	s32 t1;
	s32 t2;
	u8 *s0ptr8;
	s32 i;
	f32 yrot;

	for (i = 0; i < model->definition->nummatrices; i++) {
		sp00[i].unk00 = 0;
	}

	anim = model->anim;

	if (anim) {
		t0slot = &sp00[0];
		t2ptr8 = g_AnimHeaderBytes[g_AnimToHeaderSlot[anim->animnum]];
		t3ptr8 = g_AnimHeaderBytes[g_AnimToHeaderSlot[anim->animnum]] + g_Anims[anim->animnum].headerlen;
		t4 = g_Anims[anim->animnum].framelen;

		if (t2ptr8 != t3ptr8) {
			t6ptr8 = g_AnimFrameBytes[anim->frameslot1];

			if (!modelasmIterateThings1()) {
				return false;
			}

			if (anim->frac != 0.0f) {
				for (i = 0; i < model->definition->nummatrices; i++) {
					sp00[120 + i].unk00 = 0;
				}

				f0int = anim->frac * 4096;

				t0slot = &sp00[0];
				t2ptr8 = g_AnimHeaderBytes[g_AnimToHeaderSlot[anim->animnum]];
				t3ptr8 = g_AnimHeaderBytes[g_AnimToHeaderSlot[anim->animnum]] + g_Anims[anim->animnum].headerlen;
				t4 = g_Anims[anim->animnum].framelen;

				if (t2ptr8 != t3ptr8) {
					t6ptr8 = g_AnimFrameBytes[anim->frameslot2];

					if (!modelasmIterateThings2()) {
						return false;
					}
				}
			}
		}

		f30 = anim->fracmerge;

		if (anim->fracmerge != 0.0f) {
			t0slot = &sp00[120];
			t2ptr8 = g_AnimHeaderBytes[g_AnimToHeaderSlot[anim->animnum2]];
			t3ptr8 = g_AnimHeaderBytes[g_AnimToHeaderSlot[anim->animnum2]] + g_Anims[anim->animnum2].headerlen;
			t4 = g_Anims[anim->animnum2].framelen;

			if (t2ptr8 != t3ptr8) {
				t6ptr8 = g_AnimFrameBytes[anim->frameslot3];

				if (!modelasmIterateThings1()) {
					return false;
				}

				if (anim->frac2 != 0.0f) {
					t0slot = &sp00[120];
					f0int = anim->frac2 * 4096;
					t2ptr8 = g_AnimHeaderBytes[g_AnimToHeaderSlot[anim->animnum2]];
					t3ptr8 = g_AnimHeaderBytes[g_AnimToHeaderSlot[anim->animnum2]] + g_Anims[anim->animnum2].headerlen;
					t4 = g_Anims[anim->animnum2].framelen;

					if (t2ptr8 != t3ptr8) {
						t6ptr8 = g_AnimFrameBytes[anim->frameslot4];

						if (!modelasmIterateThings2()) {
							return false;
						}
					}
				}
			}
		}
	}

	modeldef = model->definition;
	node = modeldef->rootnode;
	skeleton = modeldef->skel;

	while (node) {
		switch (node->type & 0xff) {
		case MODELNODETYPE_POSITION:
			sp7e4 = false;

			if (model) {
				t1 = node->rodata->position.part;

				if (anim) {
					if (f30 != 0.0f) {
						if (anim->flip) {
							t2ptr8 = skeleton->things[t1];
							t0slot = &sp00[t2ptr8[1]];

							sr8 = t0slot->unk00;
							t4 = t0slot->unk06;
							t3 = t0slot->unk04;
							t2 = t0slot->unk02;

							if (t4 != 0) {
								t4 = (0x10000 - t4) & 0xffff;
							}

							if (t3 != 0) {
								t3 = (0x10000 - t3) & 0xffff;
							}
						} else {
							t0slot = &sp00[t1];

							sr8 = t0slot->unk00;
							t2 = t0slot->unk02;
							t3 = t0slot->unk04;
							t4 = t0slot->unk06;
						}

						if (anim->flip2) {
							s0ptr8 = skeleton->things[t1];
							t0slot = &sp00[s0ptr8[1]];

							s4 = t0slot[120].unk06;
							s3 = t0slot[120].unk04;
							s0 = t0slot[0].unk02;
							s2 = t0slot[120].unk02;

							if (s4 != 0) {
								s4 = (0x10000 - s4) & 0xffff;
							}

							if (s3 != 0) {
								s3 = (0x10000 - s3) & 0xffff;
							}
						} else {
							t0slot = &sp00[t1];

							s2 = t0slot[120].unk02;
							s3 = t0slot[120].unk04;
							s4 = t0slot[120].unk06;
						}

						sp7e8slot = t0slot;
						modelasmPrepareRotMtx360(t2, t3, t4);
						modelasmMathPain2();

						f16 = f0;
						f17 = f1;
						f18 = f2;
						f19 = f3;

						t2 = s2;
						t3 = s3;
						t4 = s4;

						modelasmPrepareRotMtx360(t2, t3, t4);
						modelasmMathPain2();
						modelasmMathPain1(f30);

						sp7e4 = true;
						t0slot = sp7e8slot;

						sp7e8f32 = f0;
						sp7ec = f1;
						sp7f0 = f2;
						sp7f4 = f3;

						modelasmMathPain4();
					} else {
						if (anim->flip) {
							t2ptr8 = skeleton->things[t1];
							t0slot = &sp00[t2ptr8[1]];

							sr8 = t0slot->unk00;
							t4 = t0slot->unk06;
							t3 = t0slot->unk04;
							t2 = t0slot->unk02;

							if (t4 != 0) {
								t4 = (0x10000 - t4) & 0xffff;
							}

							if (t3 != 0) {
								t3 = (0x10000 - t3) & 0xffff;
							}
						} else {
							t0slot = &sp00[t1];

							sr8 = t0slot->unk00;
							t4 = t0slot->unk06;
							t3 = t0slot->unk04;
							t2 = t0slot->unk02;
						}

						s0 = t2;

						if ((sr8 & 1) == 0) {
							f12 = 1;
							f13 = 0;
							f14 = 0;

							f15 = 0;
							f16 = 1;
							f17 = 0;

							f18 = 0;
							f19 = 0;
							f20 = 1;
						} else {
							s0 = t2;
							s1 = t3;
							s2 = t4;

							modelasmPrepareRotMtx180(t2, t3, t4);
							modelasmMathPain3();
						}
					}
				} else {
					f12 = 1;
					f13 = 0;
					f14 = 0;

					f15 = 0;
					f16 = 1;
					f17 = 0;

					f18 = 0;
					f19 = 0;
					f20 = 1;

					t0slot = &sp00[t1];
					sr8 = t0slot->unk00;
				}

				if (sr8 & 2) {
					t2 = *(s16 *) &t0slot[60].unk00;
					t3 = *(s16 *) &t0slot[60].unk02;
					t4 = *(s16 *) &t0slot[60].unk04;

					if (node == modeldef->rootnode) {
						f21 = t2 * anim->animscale;
						f22 = t3 * anim->animscale;
						f23 = t4 * anim->animscale;
					} else {
						f0 = t2 * anim->animscale;
						f1 = t3 * anim->animscale;
						f2 = t4 * anim->animscale;

						f21 = f0 + node->rodata->position.pos.x;
						f22 = f1 + node->rodata->position.pos.y;
						f23 = f2 + node->rodata->position.pos.z;
					}
				} else {
					if (node != modeldef->rootnode) {
						f21 = node->rodata->position.pos.x;
						f22 = node->rodata->position.pos.y;
						f23 = node->rodata->position.pos.z;
					} else {
						f21 = 0;
						f22 = 0;
						f23 = 0;
					}
				}

				t0mtx = node->parent ? modelasmFindNodeMtx(model, node->parent) : renderdata->unk00;
				t1mtx = &model->matrices[node->rodata->position.mtxindex0];

				modelasmMtxMultiply(t0mtx, t1mtx);

				if (g_ModelJointPositionedFunc) {
					g_ModelJointPositionedFunc(node->rodata->position.mtxindex0, &model->matrices[node->rodata->position.mtxindex0]);
				}

				if (node->type & MODELNODETYPE_0100) {
					if (sp7e4) {
						f0 = sp7e8f32;
						f1 = sp7ec;
						f2 = sp7f0;
						f3 = sp7f4;
					} else {
						t2 = s0;
						t3 = s1;
						t4 = s2;
						modelasmPrepareRotMtx360(t2, t3, t4);
						modelasmMathPain2();
					}

					f4 = 0;
					f6 = f0;
					f5 = 1;

					if (f6 < 0.0f) {
						f6 = -f6;
						f5 = -f5;
					}

					if (f6 < -0.99994999170303f) {
						f0 *= 0.5f;
						f4 = f5 * 0.5f;
						f1 *= 0.5f;
						f2 *= 0.5f;
						f3 *= 0.5f;
						f0 -= f4;
					} else if (f6 > 0.99994999170303f) {
						f0 *= 0.5f;
						f1 *= 0.5f;
						f2 *= 0.5f;
						f3 *= 0.5f;
						f4 = f5 * 0.5f;
						f0 += f4;
					} else {
						f7 = modelasmAcosOrAsin(f6);
						f17 = f0;
						f12 = f6 * 0.5f;
						s1 = t1;
						f0 = sinf(f12);
						f3 *= f0;
						f2 *= f0;
						t1 = s1;
						f1 *= f0;
						f5 *= f0;
						f0 *= f17;
						f0 += f5;
					}

					modelasmMathPain4();
					t1mtx = &model->matrices[node->rodata->position.mtxindex1];
					modelasmMtxMultiply(t0mtx, t1mtx);
				}
			}
			break;
		case MODELNODETYPE_DISTANCE:
			t0mtx = modelasmFindNodeMtx(model, node);

			rwdata = modelasmGetNodeRwData(model, node, sp7f8);
			f0 = 0;

			if (!g_ModelDistanceDisabled && t0mtx) {
				f0 = -t0mtx->m[3][2] * g_Vars.currentplayer->c_lodscalez * g_ModelDistanceScale;
			}

			if ((node->rodata->distance.near == 0.0f || f0 > node->rodata->distance.near * model->scale)
					&& f0 <= node->rodata->distance.far * model->scale) {
				rwdata->distance.visible = true;
				node->child = node->rodata->distance.target;
			} else {
				rwdata->distance.visible = false;
				node->child = NULL;
			}
			break;
		case MODELNODETYPE_CHRINFO:
			if (model) {
				t1 = node->rodata->chrinfo.animpart;

#ifdef AVOID_UB
				if (anim->animnum)
#else
				if (anim)
#endif
				{
					if (f30 != 0.0f) {
						if (anim->flip) {
							t2ptr8 = skeleton->things[t1];
							t0slot = &sp00[t2ptr8[1]];

							sr8 = t0slot->unk00;
							t4 = t0slot->unk06;
							t3 = t0slot->unk04;
							t2 = t0slot->unk02;

							if (t4 != 0) {
								t4 = (0x10000 - t4) & 0xffff;
							}

							if (t3 != 0) {
								t3 = (0x10000 - t3) & 0xffff;
							}
						} else {
							t0slot = &sp00[t1];

							sr8 = t0slot->unk00;
							t2 = t0slot->unk02;
							t3 = t0slot->unk04;
							t4 = t0slot->unk06;
						}

						if (anim->flip2) {
							s0ptr8 = skeleton->things[t1];
							t0slot = &sp00[s0ptr8[1]];

							s4 = t0slot[120].unk06;
							s3 = t0slot[120].unk04;
							s0 = t0slot[0].unk02;
							s2 = t0slot[120].unk02;

							if (s4 != 0) {
								s4 = (0x10000 - s4) & 0xffff;
							}

							if (s3 != 0) {
								s3 = (0x10000 - s3) & 0xffff;
							}
						} else {
							t0slot = &sp00[t1];

							s2 = t0slot[120].unk02;
							s3 = t0slot[120].unk04;
							s4 = t0slot[120].unk06;
						}

						sp7e8slot = t0slot;
						modelasmPrepareRotMtx360(t2, t3, t4);
						modelasmMathPain2();

						f16 = f0;
						f17 = f1;
						f18 = f2;
						f19 = f3;

						t2 = s2;
						t3 = s3;
						t4 = s4;

						modelasmPrepareRotMtx360(t2, t3, t4);
						modelasmMathPain2();
						modelasmMathPain1(f30);
						t0slot = sp7e8slot;
						modelasmMathPain4();
					} else {
						if (anim->flip) {
							t2ptr8 = skeleton->things[t1];
							t0slot = &sp00[t2ptr8[1]];

							sr8 = t0slot->unk00;
							t4 = t0slot->unk06;
							t3 = t0slot->unk04;
							t2 = t0slot->unk02;

							if (t4 != 0) {
								t4 = (0x10000 - t4) & 0xffff;
							}

							if (t3 != 0) {
								t3 = (0x10000 - t3) & 0xffff;
							}

							modelasmPrepareRotMtx180(t2, t3, t4);
						} else {
							t0slot = &sp00[t1];

							sr8 = t0slot->unk00;
							t4 = t0slot->unk06;
							t3 = t0slot->unk04;
							t2 = t0slot->unk02;

							modelasmPrepareRotMtx180(t2, t3, t4);
						}

						modelasmMathPain3();
					}
				} else {
					f12 = 1;
					f13 = 0;
					f14 = 0;

					f15 = 0;
					f16 = 1;
					f17 = 0;

					f18 = 0;
					f19 = 0;
					f20 = 1;

					t0slot = &sp00[t1];
					sr8 = t0slot->unk00;
				}

				f0 = 0;

				rwdata = modelasmGetNodeRwData(model, node, sp7f8);

				yrot = rwdata->chrinfo.yrot;

				if (rwdata->chrinfo.unk18 != 0.0f) {
					f5 = rwdata->chrinfo.unk1c - rwdata->chrinfo.yrot;

					if (f5 < 0.0f) {
						f5 += M_TAU;
					}

					if (f5 >= M_PI) {
						f5 *= rwdata->chrinfo.unk18;
						yrot += f5;

						if (yrot > M_TAU) {
							yrot -= M_TAU;
						}
					} else {
						f5 = M_TAU - f5;
						f5 *= rwdata->chrinfo.unk18;
						yrot -= f5;

						if (yrot < 0.0f) {
							yrot += M_TAU;
						}
					}
				}

				f1 = sinf(yrot);
				f0 = sinf(yrot + 1.570796251297f);

				f2 = -f1;

				f21 = rwdata->chrinfo.pos.x;
				f22 = rwdata->chrinfo.pos.y;
				f23 = rwdata->chrinfo.pos.z;

				f3 = f0 * f12;
				f4 = f1 * f14;
				f5 = f0 * f15;
				f3 = f3 + f4;
				f6 = f1 * f17;
				f7 = f0 * f18;
				f5 = f5 + f6;
				f8 = f1 * f20;
				f9 = f2 * f12;
				f7 = f7 + f8;
				f10 = f0 * f14;
				f4 = f2 * f15;
				f14 = f9 + f10;
				f6 = f0 * f17;
				f8 = f2 * f18;
				f17 = f4 + f6;
				f10 = f0 * f20;
				f12 = f3;
				f20 = f8 + f10;
				f15 = f5;
				f18 = f7;

				if (model->scale != 1.0f) {
					f12 *= model->scale;
					f13 *= model->scale;
					f14 *= model->scale;
					f15 *= model->scale;
					f16 *= model->scale;
					f17 *= model->scale;
					f18 *= model->scale;
					f19 *= model->scale;
					f20 *= model->scale;
				}

				t0mtx = renderdata->unk00;
				t1mtx = &model->matrices[node->rodata->chrinfo.mtxindex];

				modelasmMtxMultiply(t0mtx, t1mtx);
			}
			break;
		case MODELNODETYPE_HEADSPOT:
			rwdata = modelasmGetNodeRwData(model, node, sp7f8);

			if (rwdata->headspot.headmodeldef) {
				struct modelnode *iternode = rwdata->headspot.headmodeldef->rootnode;
				sp7f8 = true;
				node->child = iternode;

				while (iternode) {
					iternode->parent = node;
					iternode = iternode->next;
				}
			}
			break;
		case MODELNODETYPE_POSITIONHELD:
			t0mtx = node->parent ? modelasmFindNodeMtx(model, node->parent) : renderdata->unk00;

			f12 = 1;
			f13 = 0;
			f14 = 0;

			f15 = 0;
			f16 = 1;
			f17 = 0;

			f18 = 0;
			f19 = 0;
			f20 = 1;

			f21 = node->rodata->positionheld.pos.x;
			f22 = node->rodata->positionheld.pos.y;
			f23 = node->rodata->positionheld.pos.z;

			t1mtx = &model->matrices[node->rodata->positionheld.mtxindex];

			modelasmMtxMultiply(t0mtx, t1mtx);
			break;
		case MODELNODETYPE_REORDER:
			break;
		case MODELNODETYPE_TOGGLE:
			rwdata = modelasmGetNodeRwData(model, node, sp7f8);

			node->child = rwdata->toggle.visible ? node->rodata->toggle.target : NULL;
			break;
		}

		if (node->child) {
			node = node->child;
		} else {
			while (true) {
				if (node->next) {
					node = node->next;
					break;
				} else {
					node = node->parent;

					if (node) {
						if ((node->type & 0xff) == MODELNODETYPE_HEADSPOT) {
							// @bug:
							// sp7f8 tracks whether we are under the head node
							// which allows modelasmGetNodeRwData to avoid
							// searching for the head node if we're already
							// above it. The code here is ascending back out of
							// the head node, so it should be setting it to
							// false here.
							sp7f8 = true;
						}
					} else {
						return true;
					}
				}
			}
		}
	}

	return true;
}

/**
 * Expects:
 * t0 = pointer to stack slots
 * t2 = pointer to anim header bytes
 * t3 = pointer to end of anim header
 * t6 = pointer to anim frame bytes
 */
static bool modelasmIterateThings1(void)
{
	s32 t7;
	s32 t8;
	u32 v0;

	gp = 0;

	do {
		t7 = *t2ptr8;
		t2ptr8++;
		t0slot->unk00 = t7;

		if (t7 > 15) {
			return false;
		}

		if (t7 & 2) {
			// 0, 1, 2
			v1 = t2ptr8[2];
			s0 = modelasmReadFrameData();
			v1 = t2ptr8[2];
			s3 = 1;

			if (v1 < 16) {
				v0 = v1 - 1;
				s3 <<= v0;
				s4 = 16;

				if (s0 & s3) {
					s4 -= v1;
					v0 = (1 << s4) - 1;
					v0 <<= v1;
					s0 |= v0;
				}
			}

			s3 = t2ptr8[0] << 8;
			s4 = t2ptr8[1];
			s3 += s4;
			s0 += s3;
			s0 &= 0xffff;

			// 3, 4, 5
			v1 = t2ptr8[5];
			s1 = modelasmReadFrameData();
			v1 = t2ptr8[5];
			s3 = 1;

			if (v1 < 16) {
				v0 = v1 - 1;
				s3 <<= v0;
				s4 = 16;

				if (s1 & s3) {
					s4 -= v1;
					v0 = (1 << s4) - 1;
					v0 <<= v1;
					s1 |= v0;
				}
			}

			s3 = t2ptr8[3] << 8;
			s4 = t2ptr8[4];
			s3 += s4;
			s1 += s3;
			s1 &= 0xffff;

			// 6, 7, 8
			v1 = t2ptr8[8];
			s2 = modelasmReadFrameData();
			v1 = t2ptr8[8];
			s3 = 1;

			if (v1 < 16) {
				v0 = v1 - 1;
				s3 <<= v0;
				s4 = 16;

				if (s2 & s3) {
					s4 -= v1;
					v0 = (1 << s4) - 1;
					v0 <<= v1;
					s2 |= v0;
				}
			}

			s3 = t2ptr8[6] << 8;
			s4 = t2ptr8[7];
			s3 += s4;
			s2 += s3;
			s2 &= 0xffff;

			t2ptr8 += 9;

			t0slot[60].unk00 = s0;
			t0slot[60].unk02 = s1;
			t0slot[60].unk04 = s2;
		} else {
			s0 = 0;
			s1 = 0;
			s2 = 0;

			if (t7 & 8) {
				v0 = t2ptr8[2] + t2ptr8[5] + t2ptr8[8] + t2ptr8[11];
				v1 = t2ptr8[11];

				if (v0 >= gp) {
					v0 -= gp;
					sr8 = 0;
					t8 = v0 >> 3;
					t6ptr8 += t8;
					t8 <<= 3;
					v0 -= t8;
					sr8 = t6ptr8[0];
					gp = 8;
					t6ptr8++;
					gp -= v0;
				}

				t2ptr8 += 12;
			}
		}

		if (t7 & 1) {
			v1 = t2ptr8[2];
			s0 = modelasmReadFrameData();
			s0 += (t2ptr8[0] << 8) + t2ptr8[1];
			v0 = 16 - t4;
			s0 = (s0 << v0) & 0xffff;

			v1 = t2ptr8[5];
			s1 = modelasmReadFrameData();
			s1 += (t2ptr8[3] << 8) + t2ptr8[4];
			v0 = 16 - t4;
			s1 = (s1 << v0) & 0xffff;

			v1 = t2ptr8[8];
			s2 = modelasmReadFrameData();
			s2 += (t2ptr8[6] << 8) + t2ptr8[7];
			v0 = 16 - t4;
			s2 = (s2 << v0) & 0xffff;

			t2ptr8 += 9;
		} else {
			s0 = 0;
			s1 = 0;
			s2 = 0;
		}

		t0slot->unk02 = s0;
		t0slot->unk04 = s1;
		t0slot->unk06 = s2;
		t0slot++;
	} while (t2ptr8 < t3ptr8);

	return true;
}

/**
 * Expects:
 * t0 = pointer to stack slots
 * t2 = pointer to anim header bytes
 * t3 = pointer to end of anim header
 * t4 = frame length
 * t6 = pointer to anim frame bytes
 * f0 containing an integer
 */
static bool modelasmIterateThings2(void)
{
	s32 t7;
	s32 t8;
	s32 s5;
	u32 v0;

	gp = 0;

	do {
		t7 = t2ptr8[0];
		t2ptr8++;

		if (t7 > 15) {
			return false;
		}

		if ((t7 & 2) == 0) {
			s0 = 0;
			s1 = 0;
			s2 = 0;

			if (t7 & 8) {
				v0 = t2ptr8[2] + t2ptr8[5] + t2ptr8[8] + t2ptr8[11];

				if (v0 >= gp) {
					v0 -= gp;
					sr8 = 0;
					t8 = v0 >> 3;
					t6ptr8 += t8;
					t8 <<= 3;
					v0 -= t8;
					sr8 = t6ptr8[0];
					gp = 8;
					t6ptr8++;
					gp -= v0;
				} else {
					v0 -= gp;
				}

				t2ptr8 += 12;
			}
		} else {
			v0 = t2ptr8[2] + t2ptr8[5] + t2ptr8[8];

			if (v0 >= gp) {
				v0 -= gp;
				sr8 = 0;
				t8 = v0 >> 3;
				t6ptr8 += t8;
				t8 <<= 3;
				v0 -= t8;
				sr8 = t6ptr8[0];
				gp = 8;
				t6ptr8++;
				gp -= v0;
			} else {
				v0 -= gp;
			}

			t2ptr8 += 9;
		}

		if ((t7 & 1) == 0) {
			s0 = 0;
			s1 = 0;
			s2 = 0;
		} else {
			v1 = t2ptr8[2];
			s0 = modelasmReadFrameData();
			s0 += (t2ptr8[0] << 8) + t2ptr8[1];
			v0 = 16 - t4;
			s0 = (s0 << v0) & 0xffff;

			v1 = t2ptr8[5];
			s1 = modelasmReadFrameData();
			s1 += (t2ptr8[3] << 8) + t2ptr8[4];
			v0 = 16 - t4;
			s1 = (s1 << v0) & 0xffff;

			v1 = t2ptr8[8];
			s2 = modelasmReadFrameData();
			s2 += (t2ptr8[6] << 8) + t2ptr8[7];
			v0 = 16 - t4;
			s2 = (s2 << v0) & 0xffff;

			t2ptr8 += 9;
		}

		s3 = f0int;
		v1 = 0x10000;

		// 02
		s4 = t0slot->unk02;
		s5 = s0 - s4;

		if (s5 < 0) {
			s5 += 0x10000;
		}

		if (s5 < 0x8000) {
			s5 *= s3;
			s5 >>= 12;
			s0 = s4 + s5;

			if (s0 >= 0x10000) {
				s0 -= 0x10000;
			}
		} else {
			s5 = 0x10000 - s5;
			s5 *= s3;
			s5 >>= 12;
			s0 = s4 - s5;

			if (s0 < 0) {
				s0 += 0x10000;
			}
		}

		// 04
		s4 = t0slot->unk04;
		s5 = s1 - s4;

		if (s5 < 0) {
			s5 += 0x10000;
		}

		if (s5 < 0x8000) {
			s5 *= s3;
			s5 >>= 12;
			s1 = s4 + s5;

			if (s1 >= 0x10000) {
				s1 -= 0x10000;
			}
		} else {
			s5 = 0x10000 - s5;
			s5 *= s3;
			s5 >>= 12;
			s1 = s4 - s5;

			if (s1 < 0) {
				s1 += 0x10000;
			}
		}

		// 06
		s4 = t0slot->unk06;
		s5 = s2 - s4;

		if (s5 < 0) {
			s5 += 0x10000;
		}

		if (s5 < 0x8000) {
			s5 *= s3;
			s5 >>= 12;
			s2 = s4 + s5;

			if (s2 >= 0x10000) {
				s2 -= 0x10000;
			}
		} else {
			s5 = 0x10000 - s5;
			s5 *= s3;
			s5 >>= 12;
			s2 = s4 - s5;

			if (s2 < 0) {
				s2 += 0x10000;
			}
		}

		t0slot->unk02 = s0;
		t0slot->unk04 = s1;
		t0slot->unk06 = s2;
		t0slot++;
	} while (t2ptr8 < t3ptr8);

	return true;
}

/**
 * Expects: t3 t6 v1 gp s8
 */
static u32 modelasmReadFrameData(void)
{
	u32 v0 = 0;
	s32 s6;

	if (!v1) {
		return 0;
	}

	while (v1 > gp) {
		v1 -= gp;
		v0 |= (((1 << gp) - 1) & sr8) << v1;
		s6 = t3ptr8 - t6ptr8;

		if (s6 >= 4) {
			s6 = 4;
		}

		sr8 = ((uintptr_t) t6ptr8 & 3) - 4;
		sr8 = -sr8;

		if (sr8 != 0 && sr8 < s6) {
			s6 = sr8;
		}

		switch (s6) {
		case 1: // 19c64
			sr8 = t6ptr8[0];
			t6ptr8++;
			gp = 8;
			break;
		case 2: // 19c70
			sr8 = (t6ptr8[0] << 8) | t6ptr8[1];
			t6ptr8 += 2;
			gp = 16;
			break;
		case 3: // 19c88
			sr8 = (t6ptr8[0] << 16) | (t6ptr8[1] << 8) | t6ptr8[2];
			t6ptr8 += 3;
			gp = 24;
			break;
		case 4: // 19cac
			sr8 = (t6ptr8[0] << 24) | (t6ptr8[1] << 16) | (t6ptr8[2] << 8) | t6ptr8[3];
			t6ptr8 += 4;
			gp = 32;
			break;
		}
	}

	gp -= v1;

	if (v1) {
		v0 |= (sr8 >> gp) & ((1 << v1) - 1);
	}

	return v0;
}

/**
 * Indexed by node type.
 *
 * Each value is the byte offset into the rodata struct where that node type's
 * rwdataindex property can be found, or 0xff is there is none.
 */

#ifdef PLATFORM_64BIT
u8 var8005ef90[] = {
		 0xff, 0x08, 0xff, 0xff,
		 0xff, 0xff, 0xff, 0xff,
		 0x10, 0x2a, 0xff, 0x48,
		 0x24, 0xff, 0xff, 0xff,
		 0xff, 0xff, 0x08, 0xff,
		 0xff, 0xff, 0xff, 0x00,
		 0x24, 0x00,
};
#else
u8 var8005ef90[] = {
	0xff, 0x08, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
	0x0c, 0x22, 0xff, 0x44,
	0x20, 0xff, 0xff, 0xff,
	0xff, 0xff, 0x04, 0xff,
	0xff, 0xff, 0xff, 0x00,
	0x14, 0x00,
};
#endif

static union modelrwdata *modelasmGetNodeRwData(struct model *model, struct modelnode *node, bool is_head)
{
	u32 index = 0;
	u32 *rwdatas = model->rwdatas;
	u8 type = node->type & 0xff;

	if (type < ARRAYCOUNT(var8005ef90)) {
		if (var8005ef90[type] != 0xff) {
			index = *(u16 *) ((uintptr_t) node->rodata + var8005ef90[type]);
		}
	}

	if (is_head) {
		while (node->parent) {
			node = node->parent;

			if ((node->type & 0xff) == MODELNODETYPE_HEADSPOT) {
				union modelrwdata *tmp = modelasmGetNodeRwData(model, node, false);
				rwdatas = tmp->headspot.rwdatas;
				break;
			}
		}
	}

	return (union modelrwdata *) &rwdatas[index];
}

#if VERSION < VERSION_NTSC_1_0
void *modelGetNodeRwData(struct model *model, struct modelnode *node)
{
	u32 index = 0;
	u32 *rwdatas = model->rwdatas;
	u8 type = node->type & 0xff;

	if (type < ARRAYCOUNT(var8005ef90)) {
		if (var8005ef90[type] != 0xff) {
			index = *(u16 *) ((uintptr_t) node->rodata + var8005ef90[type]);
		}
	}

	if (model->unk00) {
		while (node->parent) {
			node = node->parent;

			if ((node->type & 0xff) == MODELNODETYPE_HEADSPOT) {
				struct modelrwdata_headspot *tmp = modelGetNodeRwData(model, node);
				rwdatas = tmp->rwdatas;
				break;
			}
		}
	}

	return &rwdatas[index];
}
#endif

/**
 * Expects: f0-f3, f16-f22
 */
static void modelasmMathPain1(f32 f30)
{
	f32 f6;
	f32 f7;
	f32 f8;
	f32 f9;

	f4 = 0;

	while (true) {
		f5 = f16 * f0;
		f6 = f17 * f1;
		f7 = f18 * f2;
		f6 = f5 + f6;
		f5 = f19 * f3;
		f6 = f6 + f7;
		f6 = f6 + f5;

		if (f6 >= 0.0f) {
			break;
		}

		f0 = -f0;
		f1 = -f1;
		f2 = -f2;
		f3 = -f3;
	}

	if (f6 < -0.99994999170303f) {
		f6 = f30 * f16;
		f5 = f5 - f30;
		f7 = f5 * f0;
		f0 = f7 - f6;
		f6 = f30 * f17;
		f7 = f5 * f1;
		f8 = f30 * f18;
		f1 = f7 - f6;
		f9 = f5 * f2;
		f6 = f30 * f19;
		f2 = f9 - f8;
		f7 = f5 * f3;
		f2 = f7 - f6;
		return;
	}

	if (f6 <= 0.99994999170303f) {
		f7 = modelasmAcosOrAsin(f6);
		f20 = f0;
		f21 = sinf(f7);
		f22 = sinf((1.0f - f30) * f7);
		f0 = sinf(f7 * f30);

		f5 = f22 / f21;
		f30 = f0 / f21;
		f0 = f20;
	} else {
		f5 = 1.0f - f30;
	}

	f6 = f5 * f16;
	f7 = f30 * f0;
	f8 = f5 * f17;
	f0 = f6 + f7;
	f9 = f30 * f1;
	f6 = f5 * f18;
	f1 = f8 + f9;
	f7 = f30 * f2;
	f8 = f5 * f19;
	f2 = f6 + f7;
	f9 = f30 * f3;
	f3 = f8 + f9;
}

/**
 * Expects: f0 f1 f2 f3 f4 f5
 */
static void modelasmMathPain2(void)
{
	f32 f6;
	f32 f7;
	f32 f8;
	f32 f9;
	f32 f10;
	f32 f11;
	f32 f26;

	f6 = f0 * f2;
	f7 = f0 * f3;
	f8 = f1 * f2;
	f9 = f1 * f3;
	f0 = f6 * f4;
	f1 = f9 * f5;
	f2 = f8 * f4;
	f0 = f0 + f1;
	f1 = f7 * f5;
	f3 = f7 * f5;
	f10 = f7 * f4;
	f1 = f2 - f1;
	f11 = f8 * f5;
	f26 = f6 * f5;
	f2 = f10 + f11;
	f10 = f9 * f4;
	f3 = f26 - f10;
}

static void modelasmPrepareRotMtx180(s32 t2, s32 t3, s32 t4)
{
	// Very close to: 1.0f / (180 * 180 / M_PI)
	f32 f8 = t4 * 0.000095873801910784f;
	f32 f7 = t3 * 0.000095873801910784f;
	f32 f6 = t2 * 0.000095873801910784f;

	f5 = sinf(f8);
	f4 = sinf(f8 + 1.570796251297f);
	f3 = sinf(f7);
	f2 = sinf(f7 + 1.570796251297f);
	f1 = sinf(f6);
	f0 = sinf(f6 + 1.570796251297f);
}

static void modelasmPrepareRotMtx360(s32 t2, s32 t3, s32 t4)
{
	// Very close to: 1.0f / (360 * 180 / M_PI)
	f32 f8 = t4 * 0.000047936900955392f;
	f32 f7 = t3 * 0.000047936900955392f;
	f32 f6 = t2 * 0.000047936900955392f;

	f5 = sinf(f8);
	f4 = sinf(f8 + 1.5707963705063f);
	f3 = sinf(f7);
	f2 = sinf(f7 + 1.5707963705063f);
	f1 = sinf(f6);
	f0 = sinf(f6 + 1.5707963705063f);
}

/**
 * Expects: f0 f1 f2 f3 f4 f5
 */
static void modelasmMathPain3(void)
{
	f32 f6 = f1 * f5;
	f32 f7 = f0 * f5;
	f32 f8 = f1 * f4;
	f32 f9 = f0 * f4;

	f12 = f2 * f4;
	f13 = f2 * f5;
	f14 = -f3;

	f15 = f8 * f3 - f7;
	f16 = f6 * f3 + f9;
	f17 = f1 * f2;

	f18 = f9 * f3 + f6;
	f19 = f7 * f3 - f8;
	f20 = f0 * f2;
}

/**
 * Expects: f0 f1 f2 f3
 */
static void modelasmMathPain4(void)
{
	f32 f6;
	f32 f7;
	f32 f8;
	f32 f9;
	f32 f10;
	f32 f11;
	f32 f24;
	f32 f25;
	f32 f26;
	f32 f27;
	f32 f28;

	f4 = f0 * f0;
	f5 = f1 * f1;
	f6 = f2 * f2;
	f4 = f4 + f5;
	f5 = f3 * f3;
	f4 = f4 + f6;
	f4 = f4 + f5;
	f4 = 2.0f / f4;
	f5 = f1 * f4;
	f6 = f2 * f4;
	f7 = f3 * f4;
	f8 = f0 * f5;
	f9 = f0 * f6;
	f10 = f0 * f7;
	f11 = f1 * f5;
	f24 = f1 * f6;
	f16 = 1.0f - f11;
	f25 = f1 * f7;
	f20 = 1.0f - f11;
	f26 = f2 * f6;
	f13 = f24 + f10;
	f27 = f2 * f7;
	f12 = 1.0f - f26;
	f28 = f3 * f7;
	f14 = f25 - f9;
	f12 = f12 - f28;
	f15 = f24 - f10;
	f16 = f16 - f28;
	f17 = f27 + f8;
	f18 = f25 + f9;
	f19 = f27 - f8;
	f20 = f20 - f26;
}

/**
 * Expects: f12-f23
 */
static void modelasmMtxMultiply(Mtxf *src, Mtxf *dst)
{
	f32 f0;
	f32 f1;
	f32 f2;
	f32 f3;
	f32 f4;
	f32 f5;
	f32 f6;
	f32 f7;
	f32 f8;
	f32 f9;
	f32 f10;
	s32 i;

	for (i = 0; i < 3; i++) {
		f0 = src->m[0][i];
		f1 = src->m[1][i];
		f2 = src->m[2][i];
		f3 = src->m[3][i];

		f4 = f0 * f12;
		f5 = f1 * f13;
		f6 = f2 * f14;

		dst->m[0][i] = f4 + f5 + f6;

		f8 = f0 * f15;
		f9 = f1 * f16;
		f10 = f2 * f17;

		dst->m[1][i] = f8 + f9 + f10;

		f4 = f0 * f18;
		f5 = f1 * f19;
		f6 = f2 * f20;

		dst->m[2][i] = f4 + f5 + f6;

		f8 = f0 * f21;
		f9 = f1 * f22;
		f10 = f2 * f23;

		dst->m[3][i] = f8 + f9 + f10 + f3;
	}

	dst->m[0][3] = 0;
	dst->m[1][3] = 0;
	dst->m[2][3] = 0;
	dst->m[3][3] = 1;
}

static Mtxf *modelasmFindNodeMtx(struct model *model, struct modelnode *node)
{
	do {
		u8 type = node->type & 0xff;

		if (type == MODELNODETYPE_CHRINFO) {
			return &model->matrices[node->rodata->chrinfo.mtxindex];
		}

		if (type == MODELNODETYPE_POSITION) {
			return &model->matrices[node->rodata->position.mtxindex0];
		}

		if (type == MODELNODETYPE_POSITIONHELD) {
			return &model->matrices[node->rodata->positionheld.mtxindex];
		}

		node = node->parent;
	} while (node);

	return NULL;
}

/**
 * See similar function func0f096890.
 */
static f32 modelasmAcosOrAsin(f32 f6)
{
	s32 t2;
	s32 t3;
	u16 *array;
	s32 shiftamount;
	s32 mask;
	s32 s0;
	s32 s1;
	s32 s2;
	s32 s3;
	s32 s4;

	t2 = f6 * 32767.0f;

	if (t2 > 32767) {
		t2 = 32767;
	} else if (t2 < -32767) {
		t2 = -32767;
	}

	t3 = t2;

	if (t3 < 0) {
		t3 = -t3;
	}

	if (t3 >= 32736) {
		array = &var8006ae90[126];
		t3 -= 32736;
		shiftamount = 3;
		mask = 0x07;
	} else if (t3 >= 30720) {
		array = &var8006ae90[62];
		t3 -= 30720;
		shiftamount = 5;
		mask = 0x1f;
	} else {
		array = &var8006ae90[0];
		shiftamount = 9;
		mask = 0x1ff;
	}

	s0 = t3 >> shiftamount;
	array += s0;
	s1 = array[0];
	s2 = array[1];
	s3 = s1 - s2;
	s4 = t3 & mask;
	s3 *= s4;
	s3 >>= shiftamount;
	t3 = s1 - s3;

	if (t2 < 0) {
		t3 = 0xffff - t3;
	}

	return 0.000047937632189132f * t3;
}

f32 cosf(f32 radians)
{
	return sinf(radians + 1.570796251297f);
}

f32 sinf(f32 radians)
{
	f32 f0;
	f32 f13;
	f32 f14;
	f32 f15;
	s32 t0;
	s32 t1;
	f32 ret;

	t0 = *(u32 *) &radians;
	t0 = (t0 >> 22) & 0x1ff;

	if (t0 < 255) {
		if (t0 >= 230) {
			f14 = radians * radians;

			ret = 0.0000026057805371238f;
			ret *= f14;
			ret += -0.0001980960223591f;
			ret *= f14;
			ret += 0.0083330664783716f;
			ret *= f14;
			ret += -0.16666659712791f;
			ret *= f14;
			ret *= radians;
			ret += radians;
		} else {
			ret = radians;
		}
	} else {
		if (t0 < 310) {
			f14 = radians * 0.31830987334251f;

			t1 = (s32) (f14 > 0.0f ? f14 + 0.5f : f14 - 0.5f);
			f14 = t1;

			f15 = M_PI;
			f15 *= f14;
			radians -= f15;

			f15 = 0.000000031786509424592f;
			f15 *= f14;
			radians -= f15;

			f14 = radians * radians;

			ret = 0.0000026057805371238f;
			ret *= f14;
			ret += -0.0001980960223591f;
			ret *= f14;
			ret += 0.0083330664783716f;
			ret *= f14;
			ret += -0.16666659712791f;
			ret *= f14;
			ret *= radians;
			ret += radians;

			if (t1 & 1) {
				ret = -ret;
			}
		} else {
			ret = 0;
		}
	}

	return ret;
}
