#include <SADXModLoader.h>
#include "multitap.h"
#include "minmax.h"
#include "indicator.h"

enum TextureIndex
{
	arrow, cpu_1, cpu_2,
	p, p1, p2, p3, p4,
	count
};

static const float margin = 0.875;
#define MARGIN_RIGHT	(HorizontalResolution * margin)
#define MARGIN_LEFT		(HorizontalResolution - MARGIN_RIGHT)
#define MARGIN_BOTTOM	(VerticalResolution * margin)
#define MARGIN_TOP		(VerticalResolution - MARGIN_BOTTOM)

#pragma region sprite parameters

static NJS_TEXNAME multicommon_TEXNAME[TextureIndex::count] = {
	{ "arrow",	0, 0 },
	{ "cpu_1",	0, 0 },
	{ "cpu_2",	0, 0 },
	{ "p",		0, 0 },
	{ "p1",		0, 0 },
	{ "p2",		0, 0 },
	{ "p3",		0, 0 },
	{ "p4",		0, 0 }
};

static NJS_TEXLIST multicommon_TEXLIST = { arrayptrandlength(multicommon_TEXNAME) };

static NJS_TEXANIM Indicator_TEXANIM[TextureIndex::count] = {
	// w,	h,	cx,	cy,	u1,	v1,	u2,		v2,	texid,	attr
	// u2 and v2 must be 0xFF
	{ 24,	16,	12,	-16,	0,	0,	0xFF,	0xFF,	arrow,	0 },
	{ 24,	24,	24,	12,		0,	0,	0xFF,	0xFF,	cpu_1,	0 },
	{ 24,	24,	0,	12,		0,	0,	0xFF,	0xFF,	cpu_2,	0 },
	{ 24,	24,	24,	12,		0,	0,	0xFF,	0xFF,	p,		0 },
	{ 24,	24,	0,	12,		0,	0,	0xFF,	0xFF,	p1,		0 },
	{ 24,	24,	0,	12,		0,	0,	0xFF,	0xFF,	p2,		0 },
	{ 24,	24,	0,	12,		0,	0,	0xFF,	0xFF,	p3,		0 },
	{ 24,	24,	0,	12,		0,	0,	0xFF,	0xFF,	p4,		0 }
};

static NJS_SPRITE Indicator_SPRITE = { { 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 0, &multicommon_TEXLIST, Indicator_TEXANIM };

static NJS_ARGB colors[] = {
	{ 1.000f, 0.000f, 0.000f, 1.000f }, // Sonic
	{ 1.000f, 0.500f, 0.000f, 0.000f }, // Eggman
	{ 1.000f, 1.000f, 0.804f, 0.000f }, // Tails
	{ 1.000f, 1.000f, 0.063f, 0.000f }, // Knuckles
	{ 1.000f, 1.000f, 0.545f, 0.322f }, // Tikal
	{ 1.000f, 1.000f, 0.545f, 0.741f }, // Amy
	{ 1.000f, 0.545f, 0.545f, 0.545f }, // Gamma
	{ 1.000f, 0.451f, 0.192f, 0.804f }, // Big
	{ 1.000f, 0.000f, 1.000f, 1.000f }, // Metal Sonic
	{ 0.750f, 0.500f, 0.500f, 0.500f }	// CPU
};

static NJS_SPRITE sprites[PLAYER_COUNT];
static NJS_TEXANIM anims[PLAYER_COUNT][TextureIndex::count];

#pragma endregion

void LoadIndicators()
{
	LoadPVM("multicommon", &multicommon_TEXLIST);
}

void ClampToScreen(NJS_POINT2& p)
{
	p.x = clamp(p.x, (float)MARGIN_LEFT, (float)MARGIN_RIGHT);
	p.y = clamp(p.y, (float)MARGIN_TOP, (float)MARGIN_BOTTOM);
}

void ClampToScreen(NJS_VECTOR& v)
{
	NJS_POINT2 p = { v.x, v.y };
	ClampToScreen(p);
	v = { p.x, p.y, v.z };
}

double GetAngle(NJS_POINT2* source, NJS_POINT2* target)
{
	return atan2(target->y - source->y, target->x - source->x);
}

void DrawElement(Uint32 playerIndex, Uint32 textureIndex)
{
	EntityData1* player = CharObj1Ptrs[playerIndex];

	if (player == nullptr)
		return;

	Uint8 charid = MetalSonicFlag ? Characters_MetalSonic : player->CharID;
	NJS_SPRITE* sp = &sprites[playerIndex];
	NJS_POINT2 projected;
	NJS_VECTOR pos = player->Position;
	pos.y += PhysicsArray[charid].CollisionSize;

	njProjectScreen(nullptr, &pos, &projected);

	sp->p = { projected.x, projected.y - (sp->tanim[0].sy + sp->tanim[1].sy), 0.0f };

	bool isVisible =
		sp->p.x < MARGIN_RIGHT &&
		sp->p.x > MARGIN_LEFT &&
		sp->p.y < MARGIN_BOTTOM &&
		sp->p.y > MARGIN_TOP;

	ClampToScreen(sp->p);
	int flags = NJD_SPRITE_COLOR | NJD_SPRITE_ALPHA;
	sp->ang = 0;

	if (textureIndex == arrow)
	{
		if (!isVisible)
		{
			flags |= NJD_SPRITE_ANGLE;
			sp->ang = NJM_RAD_ANG(GetAngle((NJS_POINT2*)&sp->p, &projected)) - 0x4000;
		}

		// TODO: Ellipse rotation around player number
		sp->tanim[arrow].cy = Indicator_TEXANIM[arrow].cy - ((isVisible) ? 0 : 12);
	}
	
	SetSpriteColor(IsControllerEnabled((Uint8)playerIndex) ? &colors[charid] : &colors[9]);
	Draw2DSprite(sp, textureIndex, -1.0f, flags, 0);
}

void DrawIndicators()
{
	njSetTexture(&multicommon_TEXLIST);
	njSetTextureNum(arrow);

	for (Uint32 i = 0; i < 4; i++)
		DrawElement(i, arrow);

	for (Uint32 i = 0; i < 4; i++)
	{
		TextureIndex index = IsControllerEnabled((Uint8)i) ? p : cpu_1;
		njSetTextureNum(index);
		DrawElement(i, index);
	}

	for (Uint32 i = 0; i < 4; i++)
	{
		TextureIndex index = IsControllerEnabled((Uint8)i) ? (TextureIndex)(p1 + i) : cpu_2;
		njSetTextureNum(index);
		DrawElement(i, index);
	}
}

void InitSprites()
{
	for (Uint32 i = 0; i < PLAYER_COUNT; i++)
	{
		memcpy(anims[i], Indicator_TEXANIM, sizeof(NJS_TEXANIM) * TextureIndex::count);
		sprites[i] = Indicator_SPRITE;
		sprites[i].tanim = anims[i];
	}
}
