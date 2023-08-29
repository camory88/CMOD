#include <iostream>
#include "../SDK/Memory/memory.hpp"
#include "../SDK/offsets.h"
#include "../SDK/Memory/patternScan.hpp"

class HitBoxManager
{
private:
public:
	Vector HeadBone{};
	Vector NeckBone{};
	Vector ChestBone{};
	Vector WaistBone{};
	Vector BotmBone{};

	Vector LeftshoulderBone{};
	Vector LeftelbowBone{};
	Vector LeftHandBone{};
	Vector RightshoulderBone{};
	Vector RightelbowBone{};
	Vector RightHandBone{};

	Vector LeftThighsBone{};
	Vector LeftkneesBone{};
	Vector LeftlegBone{};
	Vector RightThighsBone{};
	Vector RightkneesBone{};
	Vector RightlegBone{};

	Vector2D ScreenHeadBone{};
	Vector2D ScreenNeckBone{};
	Vector2D ScreenChestBone{};
	Vector2D ScreenWaistBone{};
	Vector2D ScreenBotmBone{};

	Vector2D ScreenLeftshoulderBone{};
	Vector2D ScreenLeftelbowBone{};
	Vector2D ScreenLeftHandBone{};
	Vector2D ScreenRightshoulderBone{};
	Vector2D ScreenRightelbowBone{};
	Vector2D ScreenRightHandBone{};

	Vector2D ScreenLeftThighsBone{};
	Vector2D ScreenLeftkneesBone{};
	Vector2D ScreenLeftlegBone{};
	Vector2D ScreenRightThighsBone{};
	Vector2D ScreenRightkneesBone{};
	Vector2D ScreenRightlegBone{};
};

class Entity
{
private:
public:
	Entity(uint64_t ptr)
	{
		this->ptr = ptr;
	}
	uint64_t ptr = 0x0;

	bool isAlive(Memory &mem);
	bool isKnocked(Memory &mem);
	bool isPlayer(Memory &mem);
	bool isDummy(Memory &mem);
	bool isItem(Memory &mem);
	bool isVisibile(Memory &mem);
	bool isGlowing(Memory &mem);
	bool isWayPoint(Memory &mem);

	float getLastVisTime(Memory &mem);

	int Team(Memory &mem);
	int Health(Memory &mem);
	int Shield(Memory &mem);
	int MaxHealth(Memory &mem);

	Vector BasePos(Memory &mem);
	Vector V_CamPos(Memory &mem);
	Vector V_BonePos(Memory &mem, int BoneId);
	Vector V_VeiwAngle(Memory &mem);
	Vector V_SwayAngle(Memory &mem);

	QAngle Q_CamPos(Memory &mem);
	QAngle Q_SwayAngle(Memory &mem);
	QAngle Q_VeiwAngle(Memory &mem);
	QAngle Q_BonePos(Memory &mem, int BoneId);
	QAngle CalculateBestBoneAim(Memory &mem, Entity LocaPlayer, int boneLock, int max_dists, int smothnes, float max_fov);

	void enableGlow(Memory &mem);
	void disableGlow(Memory &mem);
	void setViewAngles_Vector(Memory &mem, Vector angle);
	void setViewAngles_QAngle(Memory &mem, QAngle angle);
	void recoilControl(Memory &mem, float smoothnes);
	void get_class_name(Memory &mem, char *out_str);
	void glow(Memory &mem, Vector Color);
};

class WeaponXEntity
{
private:
public:
	WeaponXEntity(uint64_t ptr)
	{
		this->ptr = ptr;
	}
	uint64_t ptr = 0x0;

	float get_projectile_speed(Memory &mem);
	float get_projectile_gravity(Memory &mem);
	float get_zoom_fov(Memory &mem);
	int get_ammo(Memory &mem);
	int get_Fire_type(Memory &mem);

	
};

inline uint64_t getWeapon(Memory &mem)
{
	uint64_t wephandle = mem.Read<uint64_t>(localPTR + offsets::OFFSET_WEAPON);
	wephandle &= 0xffff;

	uint64_t wep_entity = mem.Read<uint64_t>(baseAddress + offsets::OFFSET_ENTITYLIST + (wephandle << 5));
	return wep_entity;
}

inline Vector ExtrapolatePos(const PredictCtx &Ctx, float Time)
{
	return Ctx.TargetPos + (Ctx.TargetVel * Time);
}

inline bool OptimalPitch(const PredictCtx &Ctx, const Vector2D &Dir2D, float *OutPitch)
{
	float Vel = Ctx.BulletSpeed, Grav = Ctx.BulletGravity, DirX = Dir2D.x, DirY = Dir2D.y;
	float Root = Vel * Vel * Vel * Vel - Grav * (Grav * DirX * DirX + 2.f * DirY * Vel * Vel);
	if (Root >= 0.f)
	{
		*OutPitch = atanf((Vel * Vel - sqrt(Root)) / (Grav * DirX));
		return true;
	}
	return false;
}

inline bool SolveTrajectory(PredictCtx &Ctx, const Vector &ExtrPos, float *TravelTime)
{
	Vector Dir = ExtrPos - Ctx.StartPos;
	Vector2D Dir2D = {sqrtf(Dir.x * Dir.x + Dir.y * Dir.y), Dir.z};

	float CurPitch;
	if (!OptimalPitch(Ctx, Dir2D, &CurPitch))
	{
		return false;
	}

	*TravelTime = Dir2D.x / (cosf(CurPitch) * Ctx.BulletSpeed);
	Ctx.AimAngles.y = atan2f(Dir.y, Dir.x);
	Ctx.AimAngles.x = CurPitch;
	return true;
}
inline bool BulletPredict(PredictCtx &Ctx)
{
	float MAX_TIME = 1.f, TIME_STEP = (1.f / 256.f);
	for (float CurrentTime = 0.f; CurrentTime <= MAX_TIME; CurrentTime += TIME_STEP)
	{
		float TravelTime;
		Vector ExtrPos = ExtrapolatePos(Ctx, CurrentTime);
		if (!SolveTrajectory(Ctx, ExtrPos, &TravelTime))
		{
			return false;
		}

		if (TravelTime < CurrentTime)
		{
			Ctx.AimAngles = {-RAD2DEG(Ctx.AimAngles.x), RAD2DEG(Ctx.AimAngles.y)};
			return true;
		}
	}
	return false;
}

inline auto Rainbow(float delay)
{
	static uint32_t cnt = 0;
	float freq = delay;

	if (++cnt >= (uint32_t)-1)
	{
		cnt = 0;
	}

	return std::make_tuple(std::sin(freq * cnt + 0) * 2.f, std::sin(freq * cnt + 2) * 2.3f, std::sin(freq * cnt + 4) * 2.6f);
}

inline void get_class_name(Memory &mem, uint64_t ptr, char *out_str)
{
	uint64_t client_networkable_vtable;
	mem.Read<uint64_t>(ptr + 8 * 3, client_networkable_vtable);

	uint64_t get_client_class;
	mem.Read<uint64_t>(client_networkable_vtable + 8 * 3, get_client_class);

	uint32_t disp;
	mem.Read<uint32_t>(get_client_class + 3, disp);
	const uint64_t client_class_ptr = get_client_class + disp + 7;

	ClientClass client_class;
	mem.Read<ClientClass>(client_class_ptr, client_class);

	mem.ReadArray<char>(client_class.pNetworkName, out_str, 32);
}

inline void get_name(Memory &mem, uint64_t index, char *name)
{
	index *= 0x10;
	uint64_t name_ptr = 0;
	mem.Read<uint64_t>(baseAddress + offsets::OFFSET_NAME_LIST + index, name_ptr);
	mem.ReadArray<char>(name_ptr, name, 32);
}
inline void get_pat(Memory &mem, uint64_t addr, char *name)
{
	uint64_t name_ptr = 0;
	mem.Read<uint64_t>(addr, name_ptr);
	mem.ReadArray<char>(name_ptr, name, 32);
}
