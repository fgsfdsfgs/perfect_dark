#ifndef _GU_H_
#define _GU_H_

/**************************************************************************
 *									  *
 *		 Copyright (C) 1994, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/**************************************************************************
 *
 *  $Revision: 1.46 $
 *  $Date: 1997/11/26 00:30:53 $
 *  $Source: /hosts/gate3/exdisk2/cvs/N64OS/Master/cvsmdev2/PR/include/gu.h,v $
 *
 **************************************************************************/

#include <PR/mbi.h>
#include <PR/ultratypes.h>
#include <PR/sptask.h>

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#define OS_M_PI		3.14159265358979323846
#define M_DTOR		(3.14159265358979323846/180.0)

#define	FTOFIX32(x)	(int)((x) * (float)0x00010000)
#define	FIX32TOF(x)	((float)(x) * (1.0f / (float)0x00010000))
#define	FTOFRAC8(x)	((int) MIN(((x) * (128.0f)), 127.0f) & 0xff)

#define  FILTER_WRAP 0
#define  FILTER_CLAMP 1

#define RAND(x) (guRandom()%x)	/* random number between 0 to x */

/*
 * Data Structures
 */
typedef struct {
	unsigned char   *base;
	int             fmt, siz;
	int             xsize, ysize;
	int             lsize;
	/* current tile info */
	int             addr;
	int             w, h;
	int             s, t;
} Image;

typedef struct {
	float	col[3];
	float	pos[3];
	float	a1, a2;		/* actual color = col/(a1*dist + a2) */
} PositionalLight;


/*
 * Function Prototypes
 */

extern int guLoadTextureBlockMipMap(Gfx **glist, unsigned char *tbuf, Image *im,
		unsigned char startTile, unsigned char pal, unsigned char cms,
		unsigned char cmt, unsigned char masks, unsigned char maskt,
		unsigned char shifts, unsigned char shiftt, unsigned char cfs,
		unsigned char cft);

extern int 	guGetDPLoadTextureTileSz (int ult, int lrt);
extern void 	guDPLoadTextureTile (Gfx *glistp, void *timg,
			int texl_fmt, int texl_size,
			int img_width, int img_height,
			int uls, int ult, int lrs, int lrt,
			int palette,
			int cms, int cmt,
			int masks, int maskt,
			int shifts, int shiftt);


/*
 * matrix operations:
 *
 * The 'F' version is floating point, in case the application wants
 * to do matrix manipulations and convert to fixed-point at the last
 * minute.
 */
extern void guMtxIdent(Mtx *m);
extern void guMtxIdentF(float mf[4][4]);
extern void guOrtho(Mtx *m, float l, float r, float b, float t,
		    float n, float f, float scale);
extern void guOrthoF(float mf[4][4], float l, float r, float b, float t,
		     float n, float f, float scale);
extern void guFrustum(Mtx *m, float l, float r, float b, float t,
		      float n, float f, float scale);
extern void guFrustumF(float mf[4][4], float l, float r, float b, float t,
		       float n, float f, float scale);
extern void guPerspective(Mtx *m, u16 *perspNorm, float fovy,
			  float aspect, float near, float far, float scale);
extern void guPerspectiveF(float mf[4][4], u16 *perspNorm, float fovy,
			   float aspect, float near, float far, float scale);
extern void guLookAt(Mtx *m,
			float xEye, float yEye, float zEye,
			float xAt,  float yAt,  float zAt,
			float xUp,  float yUp,  float zUp);
extern void guLookAtF(float mf[4][4], float xEye, float yEye, float zEye,
		      float xAt,  float yAt,  float zAt,
		      float xUp,  float yUp,  float zUp);
extern void guLookAtReflect(Mtx *m, LookAt *l,
			float xEye, float yEye, float zEye,
			float xAt,  float yAt,  float zAt,
			float xUp,  float yUp,  float zUp);
extern void guLookAtReflectF(float mf[4][4], LookAt *l,
		      float xEye, float yEye, float zEye,
		      float xAt,  float yAt,  float zAt,
		      float xUp,  float yUp,  float zUp);
extern void guLookAtHilite(Mtx *m, LookAt *l, Hilite *h,
                float xEye, float yEye, float zEye,
                float xAt,  float yAt,  float zAt,
                float xUp,  float yUp,  float zUp,
                float xl1,  float yl1,  float zl1,
                float xl2,  float yl2,  float zl2,
		int   twidth, int theight);
extern void guLookAtHiliteF(float mf[4][4], LookAt *l, Hilite *h,
		float xEye, float yEye, float zEye,
		float xAt,  float yAt,  float zAt,
		float xUp,  float yUp,  float zUp,
		float xl1,  float yl1,  float zl1,
		float xl2,  float yl2,  float zl2,
		int twidth, int theight);
extern void guLookAtStereo(Mtx *m,
			float xEye, float yEye, float zEye,
			float xAt,  float yAt,  float zAt,
			float xUp,  float yUp,  float zUp,
			float eyedist);
extern void guLookAtStereoF(float mf[4][4],
		      	float xEye, float yEye, float zEye,
		      	float xAt,  float yAt,  float zAt,
		      	float xUp,  float yUp,  float zUp,
			float eyedist);
extern void guRotate(Mtx *m, float a, float x, float y, float z);
extern void guRotateF(float mf[4][4], float a, float x, float y, float z);
extern void guRotateRPY(Mtx *m, float r, float p, float y);
extern void guRotateRPYF(float mf[4][4], float r, float p, float h);
extern void guAlign(Mtx *m, float a, float x, float y, float z);
extern void guAlignF(float mf[4][4], float a, float x, float y, float z);
extern void guScale(Mtx *m, float x, float y, float z);
extern void guScaleF(float mf[4][4], float x, float y, float z);
extern void guTranslate(Mtx *m, float x, float y, float z);
extern void guTranslateF(float mf[4][4], float x, float y, float z);
extern void guPosition(Mtx *m, float r, float p, float h, float s,
		       float x, float y, float z);
extern void guPositionF(float mf[4][4], float r, float p, float h, float s,
			float x, float y, float z);
extern void guMtxF2L(float mf[4][4], Mtx *m);
extern void guMtxL2F(float mf[4][4], Mtx *m);
extern void guMtxCatF(float m[4][4], float n[4][4], float r[4][4]);
extern void guMtxCatL(Mtx *m, Mtx *n, Mtx *res);
extern void guMtxXFMF(float mf[4][4], float x, float y, float z,
		      float *ox, float *oy, float *oz);
extern void guMtxXFML(Mtx *m, float x, float y, float z,
		      float *ox, float *oy, float *oz);

/* vector utility: */
extern void guNormalize(float *x, float *y, float *z);

/* light utilities: */
void guPosLight(PositionalLight *pl, Light *l,
                float xOb, float yOb, float zOb);
void guPosLightHilite(PositionalLight *pl1, PositionalLight *pl2,
                Light *l1, Light *l2,
                LookAt *l, Hilite *h,
                float xEye, float yEye, float zEye,
                float xOb,  float yOb,  float zOb,
                float xUp,  float yUp,  float zUp,
                int twidth, int theight);
extern int guRandom(void);

/*
 *  Math functions
 */
extern float sinf(float angle);
extern float cosf(float angle);
extern signed short sins (unsigned short angle);
extern signed short coss (unsigned short angle);
extern float sqrtf(float value);

/*
 *  Dump routines for low-level display lists
 */
/* flag values for guParseRdpDL() */
#define GU_PARSERDP_VERBOSE		1
#define GU_PARSERDP_PRAREA		2
#define GU_PARSERDP_PRHISTO		4
#define GU_PARSERDP_DUMPONLY           32  /* doesn't need to be same as */
                                           /* GU_PARSEGBI_DUMPOLNY, but this */
                                           /* allows app to use interchangeably */

extern void guParseRdpDL(u64 *rdp_dl, u64 nbytes, u8 flags);
extern void guParseString(char *StringPointer, u64 nbytes);

/*
 * NO LONGER SUPPORTED,
 * use guParseRdpDL with GU_PARSERDP_DUMPONLY flags
 */
/* extern void guDumpRawRdpDL(u64 *rdp_dl, u64 nbytes); */

/* flag values for guBlinkRdpDL() */
#define GU_BLINKRDP_HILITE		1
#define GU_BLINKRDP_EXTRACT		2

extern void
guBlinkRdpDL(u64 *rdp_dl_in, u64 nbytes_in,
             u64 *rdp_dl_out, u64 *nbytes_out,
             u32 x, u32 y, u32 radius,
             u8  red, u8 green, u8 blue,
             u8 flags);

/* flag values for guParseGbiDL() */
#define GU_PARSEGBI_ROWMAJOR	        1
#define GU_PARSEGBI_NONEST		2
#define GU_PARSEGBI_FLTMTX		4
#define GU_PARSEGBI_SHOWDMA		8
#define GU_PARSEGBI_ALLMTX		16
#define GU_PARSEGBI_DUMPONLY		32
/*
#define GU_PARSEGBI_HANGAFTER		64
#define GU_PARSEGBI_NOTEXTURES		128
*/
extern void guParseGbiDL(u64 *gbi_dl, u32 nbytes, u8 flags);
extern void guDumpGbiDL(OSTask *tp,u8 flags);

#define  GU_PARSE_GBI_TYPE    1
#define  GU_PARSE_RDP_TYPE    2
#define  GU_PARSE_READY       3
#define  GU_PARSE_MEM_BLOCK   4
#define  GU_PARSE_ABI_TYPE    5
#define  GU_PARSE_STRING_TYPE 6

typedef struct {
    int    dataSize;
    int    dlType;
    int    flags;
    u32    paddr;
} guDLPrintCB;

void guSprite2DInit(uSprite *SpritePointer,
		    void *SourceImagePointer,
		    void *TlutPointer,
		    int Stride,
		    int SubImageWidth,
		    int SubImageHeight,
		    int SourceImageType,
		    int SourceImageBitSize,
		    int SourceImageOffsetS,
		    int SourceImageOffsetT);

#endif /* !_GU_H_ */
