#include <iostream>
#include <limits> // Include the necessary header

#include "math.h"
inline uint64_t baseAddress;
inline uint64_t UIbaseAddress;
inline uint64_t localPTR;
inline uint64_t localWeponPTR;
inline uint64_t target = 0x0;
inline bool UI = true;

inline uint64_t NetChannel = 0x16eec2f;
inline uint64_t Commands = 0x220c650;
inline uint64_t CurrentCommand = 0x17078ec;

inline uint64_t m_nNextThinkTick = 0x05ac;
inline uint64_t m_lastPrimaryAttackTime = 0x1664;
inline uint64_t m_nextReadyTime = 0x1668;
inline uint64_t m_nextPrimaryAttackTime = 0x166c;
inline uint64_t m_attackTimeThisFrame = 0x1670;
inline uint64_t m_ammoInClip = 0x1690;
inline uint64_t m_ammoInStockpile = 0x1694;
inline uint64_t m_infiniteAmmoState = 0x1698;
inline uint64_t m_lifetimeShots = 0x169c;
inline uint64_t m_flTimeWeaponIdle = 0x16a0;
inline uint64_t m_bInReload = 0x16aa;


namespace settings
{
    inline int indexNum = 17000;
    inline char GameMode[32]{};
    inline bool thierdPerson = false;
    inline bool dumpGame = false;
    
    // player
    inline bool autojumpPath = true;


    // Weapon
    inline bool RSC = true;            // RECOIL SENITIVITY CONTROL
    inline float recoilcontrol = 80.f; // 100 no recoil // 1 recoil full

    // aimbot
    inline bool teamCheack = true;
    inline bool aimbot = true;
    inline bool aimbot_noRecoil = true;
    inline float FOV = 90.f;
    inline float rcs_aimbot = 20.f;
    inline int AimDist = 120;
    inline int aimBone = 5;

    inline Vector2D calangle;

    // glow stuff
    inline int GLowDist = 1000;
    inline float rainbowSpeed = .0005f;


    inline int GLOW_type = 0; // 0 none / 1 Player glow / 2 team glow / 3 health glow / 4 rainbow


    inline Vector player_Glow_color = Vector(1.f, 0.f, 0.f);
    inline Vector weapon_Glow_color = Vector(0.f, 1.f, 0.f);
    inline Vector Hands_Glow_color = Vector(0.f, 0.f, 1.f);

    inline Vector health_Glow_color = Vector(1.f, 0.f, 0.f);
    inline Vector RainBow_glow_color = Vector(0.f, 0.f, 0.f);
    inline bool hand_glow = false; inline Vector hand_glow_color = Vector(1.f, 0.f, 0.f);inline bool rainbow_hand_glow = false;
    inline bool weapon_glow = false; inline Vector weapon_glow_color = Vector(1.f, 0.f, 0.f);inline bool rainbow_weapon_glow = false;
    inline bool target_glow = true; inline Vector target_glow_color = Vector(0.f, 1.f, 0.f);
    inline Vector dummy_glow_color = Vector(0.f, 1.f, 1.f);


}

#define screenX 1920
#define screenY 1080
#define screenMidleX (1920 / 2)
#define screenMidleY (1080 / 2)

// Vectors
inline Vector old_aimpunch;

// ReClass.NET helps
namespace offsets
{
    inline uint64_t OFFSET_ENTITYLIST = 0x1e74448;          // cl_entitylist
    inline uint64_t OFFSET_LOCAL_ENT = 0x22245c8;           // LocalPlayer
    inline uint64_t OFFSET_NAME_LIST = 0xc2b0c00;           // NameList
    inline uint64_t OFFSET_THIRDPERSON = 0x01e030e0 + 0x6c; // thirdperson_override + 0x6c
    inline uint64_t OFFSET_THIRDPERSON_SV = 0x3728;         // m_thirdPersonShoulderView     ---------------- not updated for season 16

    inline uint64_t OFFSET_TIMESCALE = 0x017b8000; // host_timescale
    inline uint64_t OFFSET_LEVELNAME = 0x16eed90;  // LEVELNAME

    inline uint64_t OFFSET_TEAM = 0x0480;         // m_iTeamNum
    inline uint64_t OFFSET_HEALTH = 0x0470;       // m_iHealth
    inline uint64_t OFFSET_SHIELD = 0x01a0;       // m_shieldHealth
    inline uint64_t OFFSET_MAXSHIELD = 0x01a4;    // m_shieldHealthMax
    inline uint64_t OFFSET_NAME = 0x05c1;         // m_iName
    inline uint64_t OFFSET_SIGN_NAME = 0x05b8;       // m_iSignifierName
    inline uint64_t OFFSET_ABS_VELOCITY = 0x0170; // m_vecAbsVelocity
    inline uint64_t OFFSET_VISIBLE_TIME = 0x1A70; // CPlayer!lastVisibleTime
    inline uint64_t OFFSET_ZOOMING = 0x1c81;      // m_bZooming

    inline uint64_t OFFSET_LIFE_STATE = 0x07d0;      // m_lifeState, >0 = dead
    inline uint64_t OFFSET_BLEED_OUT_STATE = 0x2790; // m_bleedoutState, >0 = knocked

    inline uint64_t OFFSET_ORIGIN = 0x017c;            // m_vecAbsOrigin
    inline uint64_t OFFSET_BONES = 0x0ec8 + 0x48;      // m_nForceBone + 0x48
    inline uint64_t OFFSET_AIMPUNCH = 0x24e8;          // m_currentFrameLocalPlayer.m_vecPunchWeapon_Angle
    inline uint64_t OFFSET_CAMERAPOS = 0x1f80;         // CPlayer!camera_origin
    inline uint64_t OFFSET_CAMERAANGLES = 0x1f8c;      // CPlayer!camera_angles
    inline uint64_t OFFSET_VIEWANGLES = 0x25e4 - 0x14; // m_ammoPoolCapacity - 0x14
    inline uint64_t OFFSET_BREATH_ANGLES = OFFSET_VIEWANGLES - 0x10;
    inline uint64_t OFFSET_OBSERVER_MODE = 0x3534;    // m_iObserverMode
    inline uint64_t OFFSET_OBSERVING_TARGET = 0x34d0; // m_hObserverTarget
    inline uint64_t OFFSET_ARMORTYPE = 0x4694;        // m_armorType
    inline uint64_t OFFSET_MATRIX = 0x11a350;         // ViewMatrix
    inline uint64_t OFFSET_RENDER = 0x7472a28;        // ViewRender///0x238eed8 ?? //

    inline uint64_t OFFSET_WEAPON = 0x1a44;            // m_latestPrimaryWeapons
    inline uint64_t OFFSET_BULLET_SPEED = 0x0;         // CWeaponX!m_flProjectileSpeed
    inline uint64_t OFFSET_BULLET_SCALE = 0x0;         // CWeaponX!m_flProjectileScale
    inline uint64_t OFFSET_ZOOM_FOV = 0x16e0 + 0x00b8; // m_playerData + m_curZoomFOV
    inline uint64_t OFFSET_AMMO = 0x1664;              // m_ammoInClip
    inline uint64_t OFFSET_WEAPONID = 0x1674;          // m_weaponNameIndex
    inline uint64_t OFFSET_CHARGE_LEVEL = 0x17f0;      // m_lastChargeLevel
    inline uint64_t OFFSET_SEMI_AUTO = 0x1C2C;      // 

    inline uint64_t GLOW_FADE = 0x388;
    inline uint64_t GLOW_CONTEXT = 0x3C8;
    inline uint64_t GLOW_LIFE_TIME = 0x3A4;
    inline uint64_t GLOW_VISIBLE_TYPE = 0x3D0;

    inline uint64_t OFFSET_ITEM_GLOW = 0x02f0; // m_highlightFunctionBits
    inline uint64_t OFFSET_GLOW_FADE = 0x03a0;
    inline uint64_t OFFSET_GLOW_T1 = 0x292;            // 16256 = enabled, 0 = disabled
    inline uint64_t OFFSET_GLOW_T2 = 0x30c;            // 1193322764 = enabled, 0 = disabled
    inline uint64_t OFFSET_GLOW_ENABLE = 0x3f8;        // 7 = enabled, 2 = disabled
    inline uint64_t OFFSET_GLOW_THROUGH_WALLS = 0x400; // 2 = enabled, 5 = disabled
    inline uint64_t GLOW_TYPE = 0x02f0 + 4;            // 0x2C4 //OK Script_Highlight_GetState + 4 / m_highlightFunctionBits + 4?
    inline uint64_t GLOW_COLOR = 0x01e8 + 24;          // OK Script_CopyHighlightState mov tcx nº7 / m_highlightParams + 24 (0x18)
    inline uint64_t GLOW_DISTANCE = 0x3B4;             // OK Script_Highlight_SetFarFadeDist / m_highlightServerFadeEndTimes + 52(0x34)

    inline uint64_t OFFSET_IN_ATTACK = 0x07472e98; // in_attack=0x074bae60
    inline uint64_t OFFSET_IN_FORWARD = 0x0759c0f8;
    inline uint64_t OFFSET_FLAGS = 0x00c8;            // m_fFlags
    inline uint64_t OFFSET_MODEL = 0x0030;            // TO BE RESEARCHED
    inline uint64_t OFFSET_ViewModels = 0x2dc0;       // m_hViewModels
    inline uint64_t OFFSET_STUDIOHDR = 0x1118;        // CBaseAnimating!m_pStudioHdr
    inline uint64_t OFFSET_BONECLASS = 0x0ec8 + 0x48; ////m_nForceBone + 0x48
    inline uint64_t OFFSET_FLOCALTIME = 0x04BC;
    inline uint64_t OFFSET_CROSSHAIR_TARGET_START_TIME = 0x1B18;
    inline uint64_t OFFSET_CROSSHAIR_TARGET_END_TIME = 0x1B1C;
    inline uint64_t OFFSET_SKIN = 0x0e84;       /// m_nSkin
    inline uint64_t OFFSET_SKIN_MODLE = 0x0e88; /// m_skinMod
    inline uint64_t OFFSET_FYAW = 0x22ec - 0x8; // m_currentFramePlayer.m_ammoPoolCount - 0x8
    inline uint64_t OFFSET_FPITCH = OFFSET_FYAW - 0x4;
    inline uint64_t OFFSET_FROLL = OFFSET_FYAW + 0x4;
    inline uint64_t OFFSET_INPUT_SYSTEM = 0x17c3c00; // InputSystem

    inline uint64_t in_zoom = 0x07473138;     // in_zoom=0x0bc1cba0
    inline uint64_t in_attack = 0x07472f98;     // iin_attack=0x07472f98
    inline uint64_t in_jump = 0x07473098;
    inline uint64_t m_grapple = 0x2ce8;
    inline uint64_t m_grappleAttached = 0x0048;

    inline uint64_t OFFSET_GlobalVars = 0x16ee8d0;


   inline uint64_t OFFSET_GAMEMODE  = 0x0225d440; //mp_gamemode
};



namespace ModuleOffsets
{
    inline uintptr_t NetChannel = 0x01608560; // net_channels
    inline uintptr_t CurrentCommand = 0x16524BC; // Replace with the actual offset
    inline uintptr_t Commands = 0x20A9960; // Replace with the actual offset
};

/*
    pCommands=0x220c5b0
    LatestCommandNumber=0x17078ec
    NetChannel=0x16eec30

            char       GameMode[32]{};
        Driver->ReadProcess(Driver->BaseAddress + OFFSET_GAMEMODE, GameMode, sizeof(GameMode));
        Engine->DrawString(50.f, static_cast<float>(screen_size_y) - 50.f, IM_COL32_WHITE, EncryptA("[+] Game Mode: %hs."), GameMode);
*/
struct glowMode
{
    char GeneralGlowMode /*-147 -85 0 10 12 13 76 77 118 136 142 171 */, BorderGlowMode /*-86 6 77*/, BorderSize, TransparentLevel;
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
} bone_t;

struct ClientClass
{
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
inline CGlobalVars GlobalVars;

struct PredictCtx
{
    Vector StartPos;
    Vector TargetPos;
    Vector TargetVel;
    float BulletSpeed;
    float BulletGravity;

    Vector2D AimAngles;
};
//error: redefinition of ‘float ToMeters(float)’
inline float ToMeters(float x)
{
    return x / 39.62f;
}


    struct CUserCmd {
    	int command_number; //0x0000
    	int tick_count; //0x0004
    	float command_time; //0x0008
    	Vector viewangles; //0x000C
    	char pad_0x0018[0x14]; //0x0018 //BYTE
    	float forwardmove; //0x002C
    	float sidemove; //0x0030
    	float upmove; //0x0034
    	int buttons; //0x0038
    	char impulse; //0x003C //BYTE
    };