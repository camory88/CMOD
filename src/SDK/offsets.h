#include <iostream>
uint64_t baseAddress;
uint64_t UIbaseAddress;
uint64_t LocalPLayer;
uint64_t target;

//aimbot 
float fov = 90.f;
int smoothnes = 200;
int boneLock = 6;
int dist = 80;
int iteamDist = 200;

//extra
int indexNum = 100;
float recoilcontrol = 80.f; // 100 no recoil // 1 recoil full

//Vectors 
Vector old_aimpunch; 
Vector glowColor = Vector(1.f,0.f,0.f);
Vector targetGlowcolor = Vector(255,69,69);

//
// Updated - season 16
//
#define OFFSET_GlobalVars 0x1461fa0//GlobalVars
#define OFFSET_ENTITYLIST 0x1b37938 // cl_entitylist
#define OFFSET_LOCAL_ENT (0x01ee8b70 + 0x8)// LocalPlayer // (.?AVC_GameMovement@@ + 0x8)
#define OFFSET_VIEWANGLES 0x25ac - 0x14 //m_ammoPoolCapacity - 0x14; 
#define OFFSET_AIMPUNCH 0x24b0 // m_currentFrameLocalPlayer.m_vecPunchWeapon_Angle
#define OFFSET_ORIGIN 0x014c          //m_vecAbsOrigin
#define OFFSET_VISIBLE_TIME 0x1a78       //CPlayer!lastVisibleTime
#define OFFSET_CAMERAPOS 0x1f48  //CPlayer!camera_origin
#define OFFSET_BREATH_ANGLES		(OFFSET_VIEWANGLES - 0x10)
#define OFFSET_ITEM_GLOW            0x02c0 //m_highlightFunctionBits
#define OFFSET_AMMO                 0x1644 //m_ammoInClip
#define OFFSET_TIMESCALE            0x014d2910 //host_timescale
#define OFFSET_MATRIX 0x11a210 // view_matrix Dump file
#define OFFSET_RENDER 0x7677288 // view_render Dump file
#define OFFSET_ABS_VELOCITY         0x140 //good?
#define OFFSET_SKIN 0x0e54       /// m_nSkin
#define OFFSET_WEAPON				0x1a1c //m_latestPrimaryWeapons
#define OFFSET_TEAM	0x044c               //m_iTeamNum
#define OFFSET_BLEED_OUT_STATE 0x2740    //m_bleedoutState
#define GLOW_TYPE 0x2C4 // 0x2C4 //OK Script_Highlight_GetState + 4 / m_highlightFunctionBits + 4?
#define OFFSET_GLOW_ENABLE          0x3c8 //7 = enabled, 2 = disabled
#define OFFSET_GLOW_THROUGH_WALLS   0x3d0 //2 = enabled, 5 = disabled
#define OFFSET_HEALTH 0x043c //m_iHealth
#define OFFSET_LIFE_STATE			0x0798 //m_lifeState, >0 = dead
#define OFFSET_ZOOM_FOV             0x16c0  + 0x00b8//m_playerData + m_curZoomFOV
#define OFFSET_BULLET_SPEED			0x1f18 //CWeaponX!m_flProjectileSpeed
#define OFFSET_BULLET_GRAVITY	 (OFFSET_BULLET_SPEED + 0x8)
#define OFFSET_YAW  0x22b4 - 0x8 //m_currentFramePlayer.m_ammoPoolCount - 0x8
#define OFFSET_THIRDPERSON          0x01b1c6d0 + 0x6c //thirdperson_override + 0x6c
#define OFFSET_THIRDPERSON_SV       0x36c8 //m_thirdPersonShoulderView     ---------------- not updated for season 16
#define OFFSET_GLOW_T1              0x262 //16256 = enabled, 0 = disabled 
#define OFFSET_GLOW_T2              0x2dc //1193322764 = enabled, 0 = disabled 
#define OFFSET_GLOW_ENABLE          0x3c8 //7 = enabled, 2 = disabled
#define OFFSET_GLOW_THROUGH_WALLS   0x3d0 //2 = enabled, 5 = disabled
#define in_zoom 0x0bc9f770//---------------- not updated for season 16
#define in_attack 0x076780e8

//player identity
#define OFFSET_SHIELD				0x0170 //m_shieldHealth
#define OFFSET_BONES 0x0e98 + 0x48 ////m_nForceBone + 0x50
#define OFFSET_NAME	0x0589  //m_iName

//screen vars
#define screenX 1920
#define screenY 1080
#define screenMidleX (1920 /2)
#define screenMidleY (1080 /2)




struct glowMode
{
	char GeneralGlowMode, BorderGlowMode, BorderSize, TransparentLevel;
};

struct glowFade
{
	int a, b;
	float c, d, e, f;
};

struct c_matrix
{
	float matrix[16];
};

typedef struct bone_t
{
	uint8_t pad1[0xCC];
	float x;
	uint8_t pad2[0xC];
	float y;
	uint8_t pad3[0xC];
	float z;
}bone_t;

struct ClientClass {
	uint64_t pCreateFn;
	uint64_t pCreateEventFn;
	uint64_t pNetworkName;
	uint64_t pRecvTable;
	uint64_t pNext;
	uint32_t ClassID;
	uint32_t ClassSize;
};

struct CGlobalVars
    {
        /*0x00*/ double realtime;
        /*0x08*/ int32_t framecount;
        /*0x0c*/ float absoluteframetime;
        /*0x10*/ float curtime;
        /*0x14*/ float curtime2;
        /*0x18*/ float curtime3;
        /*0x1c*/ float curtime4;
        /*0x20*/ float frametime;
        /*0x24*/ float curtime5;
        /*0x28*/ float curtime6;
        /*0x2c*/ float zero;
        /*0x30*/ float frametime2;
        /*0x34*/ int32_t maxClients;
        /*0x38*/ int32_t unk38;
        /*0x3c*/ int32_t unk3C;
        /*0x40*/ int32_t tickcount;
        /*0x44*/ float interval_per_tick;
        /*0x48*/ float interpolation_amount;
        // There's more stuff after this but I don't know and I don't care
    };

#define GLOW_FADE 0x388
#define GLOW_DISTANCE 0x3B4
#define GLOW_CONTEXT 0x3C8
#define GLOW_LIFE_TIME 0x3A4
#define GLOW_VISIBLE_TYPE 0x3D0
#define GLOW_TYPE 0x2C4
#define GLOW_COLOR 0x1D0

float ToMeters(float x)
{
	return x / 39.62f;
}

CGlobalVars GlobalVars;
