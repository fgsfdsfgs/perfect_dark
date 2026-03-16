#include <dirent.h>
#include <sys/stat.h>

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#include "gbiex.h"
#include "types.h"

#include "system.h"
#include "fs.h"
#include "data.h"
#include "romdata.h"
#include "ext_tex.h"

#define EXT_TEX_DIRNAME "ext_tex"
#define FONT_OUTLINES_DIR "outlines"

static char extTexPath[FS_MAXPATH + 1];

#define MAX_EXT_TEX 8192
#define NUM_FONTS 5
const u16 IDMASK_FONT_OUTLINE = MASK_FONT_OUTLINE << 8;


struct ExtTexture
{
	u8 *texdata;
	s32 texnum;
	char extension[5];
};

struct ModelTextures
{
	s16 fileNum;
	s16 numTextures;
	struct ExtTexture *textures;
};

static struct ExtTexture extTextures[MAX_EXT_TEX];

static struct ModelTextures *modelTextures;
static s32 numModels;

#if VERSION == VERSION_PAL_FINAL
#define NCHARS 135
#else
#define NCHARS 94
#endif

static struct ExtTexture fontExtTextures[NUM_FONTS][NCHARS];
static struct ExtTexture fontOutlineExtTextures[NUM_FONTS][NCHARS];

#define FONT_HANDELGOTHICSM 0
#define FONT_HANDELGOTHICMD 1
#define FONT_HANDELGOTHICXS 2
#define FONT_HANDELGOTHICLG 3
#define FONT_NUMERIC 4

s32 fileInfo(const char *filename, s32 *texNum, char extension[5])
{
	char *ext = strrchr(filename, '.');

	// no extension
	if (!ext) return 1;

	++ext;
	strncpy(extension, ext, 5);

	// get the filename without extension
	char basename[16] = { 0 };
	memcpy(basename, filename, strlen(filename) - strlen(ext) - 1);

	*texNum = strtol(basename, NULL, 16);

	return 0;
}

struct ExtTexture *lookupModelTex(u16 fileNum, s32 texNum)
{
	if (fileNum > NUM_FILES) {
		sysLogPrintf(LOG_WARNING, "Invalid fileNum in lookupModelTex: %04x, texNum: %04x", fileNum, texNum);
		return 0;
	}

	struct ModelTextures *modelTex = NULL;
	for (int i = 0; i < numModels; ++i) {
		if (modelTextures[i].fileNum == fileNum) {
			modelTex = &modelTextures[i];
			break;
		}
	}

	if (modelTex == NULL)
		return NULL;

	for (int i = 0; i < modelTex->numTextures; ++i) {
		if (modelTex->textures[i].texnum == texNum)
			return &modelTex->textures[i];
	}

	return NULL;
}

struct ExtTexture *getExtTexture(u8 type, u16 id, s32 texnum)
{
	struct ExtTexture *texlist;
	switch (type) {
		case G_TEXTYPE_NONE:
			return NULL;
		case G_TEXTYPE_GENERAL:
			return &extTextures[texnum];
		case G_TEXTYPE_MODEL:
			return lookupModelTex(id, texnum);
		case G_TEXTYPE_FONT: {
			if (id & IDMASK_FONT_OUTLINE)
				return &fontOutlineExtTextures[id & ~IDMASK_FONT_OUTLINE][texnum];

			return &fontExtTextures[id][texnum];
		}
		default:
			sysLogPrintf(LOG_WARNING, "Invalid Texture type: %d, texnum: %04x", type, texnum);
			return NULL;
	}
}

u8 extTexExists(u8 type, u16 id, s32 texnum)
{
	struct ExtTexture *tex = getExtTexture(type, id, texnum);
	return tex && tex->texnum >= 0;
}

char *resolveFontname(const u8 fontId)
{
	switch (fontId) {
		case FONT_HANDELGOTHICSM: return "fonthandelgothicsm";
		case FONT_HANDELGOTHICMD: return "fonthandelgothicmd";
		case FONT_HANDELGOTHICXS: return "fonthandelgothicxs";
		case FONT_HANDELGOTHICLG: return "fonthandelgothiclg";
		case FONT_NUMERIC: return "fontnumeric";
		default: return "";
	}
}

u8 getTexPath(char *dst, u8 type, u16 id, s32 texnum)
{
	struct ExtTexture *tex;
	const char *name;

	switch (type) {
		case G_TEXTYPE_GENERAL: {
			tex = &extTextures[texnum];
			snprintf(dst, FS_MAXPATH, "%s/%04x.%s", extTexPath, texnum, tex->extension);
			return 0;
		}
		case G_TEXTYPE_FONT: {
			name = resolveFontname(id & ~IDMASK_FONT_OUTLINE);

			if (id & IDMASK_FONT_OUTLINE) {
				tex = &fontOutlineExtTextures[id & ~IDMASK_FONT_OUTLINE][texnum];
				snprintf(dst, FS_MAXPATH, "%s/%s/" FONT_OUTLINES_DIR "/%02x.%s", extTexPath, name, texnum, tex->extension);
				return 0;
			}

			tex = &fontExtTextures[id][texnum];
			snprintf(dst, FS_MAXPATH, "%s/%s/%02x.%s", extTexPath, name, texnum, tex->extension);
			return 0;
		}
		case G_TEXTYPE_MODEL: {
			name = romdataFileGetName(id);
			tex = lookupModelTex(id, texnum);
			snprintf(dst, FS_MAXPATH, "%s/%s/%05x.%s", extTexPath, name, texnum, tex->extension);
			return 0;
		}
		default: return 1;
	}
}

u8 *extTexLoad(u8 type, u16 id, s32 texnum, u32 *width, u32 *height)
{
	char path[FS_MAXPATH];
	u8 err = getTexPath(path, type, id, texnum);
	if (err) {
		sysLogPrintf(LOG_WARNING, "Invalid type in extTexLoad: %d, id: 04x, texnum: %04x", type, id, texnum);
		return 0;
	}

	struct ExtTexture *tex = getExtTexture(type, id, texnum);

	if (!tex) {
		sysLogPrintf(LOG_WARNING, "Unable to load texture: %05x", texnum);
		return NULL;
	}

	u32 channels;
	tex->texdata = stbi_load(path, width, height, &channels, 4);
	return tex->texdata;
}

u8 extTexFontID(struct font *font) {
	if (font == g_FontHandelGothicSm)
		return FONT_HANDELGOTHICSM;
	else if (font == g_FontHandelGothicMd)
		return FONT_HANDELGOTHICMD;
	else if (font == g_FontHandelGothicXs)
		return FONT_HANDELGOTHICXS;
	else if (font == g_FontHandelGothicLg)
		return FONT_HANDELGOTHICLG;
	else if (font == g_FontNumeric)
		return FONT_NUMERIC;

	return 0xff;
}

u8 resolveFontID(const char *fontname)
{
	if (strcmp(fontname, "fonthandelgothicsm") == 0)
		return FONT_HANDELGOTHICSM;
	else if (strcmp(fontname, "fonthandelgothicmd") == 0)
		return FONT_HANDELGOTHICMD;
	else if (strcmp(fontname, "fonthandelgothicxs") == 0)
		return FONT_HANDELGOTHICXS;
	else if (strcmp(fontname, "fonthandelgothiclg") == 0)
		return FONT_HANDELGOTHICLG;
	else if (strcmp(fontname, "fontnumeric") == 0)
		return FONT_NUMERIC;

	return 0xff;
}

void setTex(struct ExtTexture *texlist, s32 index, s32 texNum, char extension[5])
{
	struct ExtTexture *tex = &texlist[index];
	tex->texnum = texNum;
	strcpy(tex->extension, extension);
}

void readModelTextures(const char *path, s16 fileNum, s32 *modelOffset, struct ModelTextures *modelTex)
{
	DIR *dr = opendir(path);
	struct dirent *de;

	s32 MAX_TEX = 16;
	modelTex->textures = sysMemAlloc(MAX_TEX * sizeof(struct ExtTexture));
	modelTex->numTextures = 0;
	modelTex->fileNum = fileNum;

	char extension[5] = { 0 };

	while ((de = readdir(dr)) != NULL) {
		const char *name = de->d_name;
		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;

		s32 texNum;
		s32 err = fileInfo(name, &texNum, extension);
		// no extension: skip
		if (err) continue;

		setTex(modelTex->textures, modelTex->numTextures, texNum, extension);
		modelTex->numTextures++;

		// allocate more memory for model textures if needed
		if (modelTex->numTextures > MAX_TEX) {
			MAX_TEX *= 2;
			modelTex->textures = sysMemRealloc(modelTex->textures, MAX_TEX * sizeof(struct ExtTexture));
		}
	}
	closedir(dr);

	// shrink the textures array to the actual number of textures found
	s32 numTex = modelTex->numTextures;

	if (numTex > 0)
		modelTex->textures = sysMemRealloc(modelTex->textures, numTex * sizeof(struct ExtTexture));

	for (int i = 0; i < modelTex->numTextures; ++i) {
		modelTex->textures[i].texdata = 0;
	}
}

void readFontTextures(const char *path, const char *fontName)
{
	DIR *dr = opendir(path);
	struct dirent *de;

	u8 fontID = resolveFontID(fontName);
	char extension[5] = { 0 };

	char outlinesPath[FS_MAXPATH];
	sprintf(outlinesPath , "%s/" FONT_OUTLINES_DIR, path);
	u8 outlines = false;

	while (true) {
		de = readdir(dr);
		// after done processing the font folder, do the same for the outlines folder if any
		if (de == NULL) {
			if (outlines) break;

			outlines = true;
			closedir(dr);
			dr = opendir(outlinesPath);
			de = readdir(dr);

			if (de == NULL) break;
		}

		const char *name = de->d_name;
		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;

		s32 texNum;
		s32 err = fileInfo(name, &texNum, extension);
		// no extension: skip
		if (err) continue;

		if (outlines)
			setTex(fontOutlineExtTextures[fontID], texNum, texNum, extension);
		else
			setTex(fontExtTextures[fontID], texNum, texNum, extension);
	}

	closedir(dr);
}

void extTexFree()
{
	for (int i = 0; i < MAX_EXT_TEX; ++i) {
		if (extTextures[i].texdata)
			stbi_image_free(extTextures[i].texdata);

		extTextures[i].texdata = 0;
	}

	for (int i = 0; i < NUM_FONTS; ++i) {
		for (int j = 0; j < NCHARS; ++j) {
			if (fontExtTextures[i][j].texdata)
				stbi_image_free(fontExtTextures[i][j].texdata);

			if (fontOutlineExtTextures[i][j].texdata)
				stbi_image_free(fontOutlineExtTextures[i][j].texdata);

			fontExtTextures[i][j].texdata = 0;
			fontOutlineExtTextures[i][j].texdata = 0;
		}
	}

	for (int i = 0; i < numModels; ++i) {
		struct ModelTextures *modelTex = &modelTextures[i];
		for (int j = 0; j < modelTex->numTextures; ++j) {
			if (modelTex->textures[j].texdata)
				stbi_image_free(modelTex->textures[j].texdata);

			modelTex->textures[j].texdata = 0;
		}
	}
}

s32 extTexInit()
{
	const char *path = fsFullPath(EXT_TEX_DIRNAME);
	strcpy(extTexPath, path);

	for (int i = 0; i < MAX_EXT_TEX; ++i) {
		extTextures[i].texnum = -1;
		extTextures[i].texdata = 0;
	}

	for (int i = 0; i < NUM_FONTS; ++i) {
		for (int j = 0; j < NCHARS; ++j) {
			fontExtTextures[i][j].texnum = -1;
			fontExtTextures[i][j].texdata = 0;

			fontOutlineExtTextures[i][j].texnum = -1;
			fontOutlineExtTextures[i][j].texdata = 0;
		}
	}

	struct dirent *de;
	DIR *dr = opendir(extTexPath);

	char filepath[FS_MAXPATH];
	s32 modelOffset = 0;

	s32 MAX_MODELS = 16;
	numModels = 0;
	modelTextures = sysMemAlloc(MAX_MODELS * sizeof(struct ModelTextures));

	while ((de = readdir(dr)) != NULL) {
		const char *name = de->d_name;
		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;

		struct stat stbuf;
		sprintf(filepath , "%s/%s", extTexPath, de->d_name);
		if (stat(filepath, &stbuf) == -1) {
			sysLogPrintf(LOG_WARNING, "Unable to stat file: %s\n", filepath);
			continue;
		}

		// is a directory
		if (S_ISDIR(stbuf.st_mode)) {
			// models
			char s = name[0];
			if (s == 'P' || s == 'C' || s == 'G') {
				s16 fileNum = (s16)romdataFileGetNumForName(name);
				if (fileNum < 0) {
					sysLogPrintf(LOG_WARNING, "extTexInit invalid file: %s\n", name);
					continue;
				}

				struct ModelTextures *modelTex = &modelTextures[numModels++];
				readModelTextures(filepath, fileNum, &modelOffset, modelTex);

				// allocate more memory if necessary
				if (numModels > MAX_MODELS) {
					MAX_MODELS *= 2;
					modelTextures = sysMemRealloc(modelTextures, MAX_MODELS);
				}

			}
			// fonts
			else if (s == 'f') {
				readFontTextures(filepath, name);
			}
		} else {
			s32 texNum = 0;
			char extension[5] = { 0 };
			s32 err = fileInfo(name, &texNum, extension);

			// no extension: skip
			if (err) continue;

			setTex(extTextures, texNum, texNum, extension);
		}
	}

	closedir(dr);

	// shrink this array to the actual number of model folders found
	if (numModels > 0)
		modelTextures = sysMemRealloc(modelTextures, numModels * sizeof(struct ModelTextures));

	return 0;
}
