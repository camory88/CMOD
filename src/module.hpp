#include "memory.hpp"
#include "SDK/math.h"
#include "SDK/offsets.h"

void start_glowing(Memory& mem, uint64_t ent,Vector glow_col)
{
	//   GeneralGlowMode, BorderGlowMode, BorderSize, TransparentLevel
	//0, 4, 69, 125 outline neonstripe
	//0, 118, 100, 100  outline colored
	glowMode glowStyle = { 0, 118, 125, 100 }; //Default Glow is Outline

	float time = 5000.f;

	//Vector glow_col = { 255,6,6 };
	mem.Write<glowMode>(ent + GLOW_TYPE,glowStyle);
	mem.Write<Vector>(ent + GLOW_COLOR,glow_col);
	mem.Write<float>(ent + GLOW_DISTANCE,40000.f);
	mem.Write<float>(ent + GLOW_LIFE_TIME,time);
	time -= 1.f;
	mem.Write<int>(ent + GLOW_CONTEXT,1);
	mem.Write<int>(ent + GLOW_VISIBLE_TYPE,1);
	//glowFade sus = glowFade( 872415232, 872415232, time, time, time, time);
	mem.Write<glowFade>(ent + GLOW_FADE,{872415232, 872415232, time, time, time, time});
}



bool isGlowing(Memory& mem,uint64_t ent)
{
	return mem.Read<int>(ent + OFFSET_ITEM_GLOW) == 1363184265;
}

void enableGlow(Memory& mem,uint64_t ent)
{
	mem.Write<int>(ent + OFFSET_ITEM_GLOW, 1363184265);
}

void disableGlow(Memory& mem,uint64_t ent)
{
	mem.Write<int>(ent + OFFSET_ITEM_GLOW, 1411417991);
}

Vector getBonePosition(Memory& mem, uint64_t player, int id)
{
	Vector position = mem.Read<Vector>(player + OFFSET_ORIGIN);
	uintptr_t boneArray = mem.Read<uintptr_t>(player + OFFSET_BONES);
	Vector bone = Vector();
	uint32_t boneloc = (id * 0x30);
	bone_t bo = {};
	mem.Read<bone_t>(boneArray + boneloc, bo);
	bone.x = bo.x + position.x;
	bone.y = bo.y + position.y;
	bone.z = bo.z + position.z;
	return bone;
}

inline c_matrix get_matrix(Memory& mem)
{
	uint64_t viewRenderer = mem.Read<uint64_t>(baseAddress + OFFSET_RENDER);
	uint64_t viewMatrix = mem.Read<uint64_t>(viewRenderer + OFFSET_MATRIX);

	return mem.Read<c_matrix>(viewMatrix);
}

bool world_to_screen(Memory& mem, Vector in, Vector2D& out)
{

	float* m_vMatrix = get_matrix(mem).matrix;
	float w = m_vMatrix[12] * in.x + m_vMatrix[13] * in.y + m_vMatrix[14] * in.z + m_vMatrix[15];

	if (w < 0.01f)
		return false;

	out.x = m_vMatrix[0] * in.x + m_vMatrix[1] * in.y + m_vMatrix[2] * in.z + m_vMatrix[3];
	out.y = m_vMatrix[4] * in.x + m_vMatrix[5] * in.y + m_vMatrix[6] * in.z + m_vMatrix[7];

	float invw = 1.0f / w;
	out.x *= invw;
	out.y *= invw;

	float xx = 1920 / 2;
	float xy = 1080 / 2;

	xx += 0.5 * out.x * 1920 + 0.5;
	xy -= 0.5 * out.y * 1080 + 0.5;

	out.x = xx;
	out.y = xy;

	if (out.x > 1920 || out.x < 0 || out.y > 1080 || out.y < 0)
		return false;

	return true;
}

float getLastVisTime(Memory& mem, uint64_t ptr)
{
    return mem.Read<float>(ptr + OFFSET_VISIBLE_TIME);
}

bool isVisibile(Memory& mem, uint64_t ptr)
{
   return GlobalVars.curtime - getLastVisTime(mem,ptr) < GlobalVars.interval_per_tick;
}


void thirdPerson(Memory& mem)
{
	if(mem.Read<int>(baseAddress+in_zoom+0x8)==5)
	{
	    mem.Write<int>(baseAddress + OFFSET_THIRDPERSON,-1);
	    mem.Write<int>(LocalPLayer + OFFSET_THIRDPERSON_SV,0);
	}
	else
	{
	   mem.Write<int>(baseAddress + OFFSET_THIRDPERSON,1);
	   mem.Write<int>(LocalPLayer + OFFSET_THIRDPERSON_SV,1);
	}
}

void get_class_name(Memory& mem, uint64_t entity_ptr, char* out_str)
{
	uint64_t client_networkable_vtable;
	mem.Read<uint64_t>(entity_ptr + 8 * 3, client_networkable_vtable);

	uint64_t get_client_class;
	mem.Read<uint64_t>(client_networkable_vtable + 8 * 3, get_client_class);

	uint32_t disp;
	mem.Read<uint32_t>(get_client_class + 3, disp);
	const uint64_t client_class_ptr = get_client_class + disp + 7;

	ClientClass client_class;
	mem.Read<ClientClass>(client_class_ptr, client_class);

	mem.ReadArray<char>(client_class.pNetworkName, out_str, 32);
}

bool isDummy(Memory& mem, uint64_t entity_ptr)
{
	char class_name[33] = {};
	get_class_name( mem,entity_ptr, class_name);

	return strncmp(class_name, "CAI_BaseNPC", 11) == 0;
}

bool isKnocked(Memory& mem, uint64_t entity_ptr)
{
	return mem.Read<int>(entity_ptr + OFFSET_BLEED_OUT_STATE) > 0;
}

bool isPlayer(Memory& mem, uint64_t entity_ptr)
{
	return mem.Read<uint64_t>(entity_ptr + OFFSET_NAME) == 125780153691248;
}







class WeaponXEntity
{
public:
	void update(Memory& mem);
	float get_projectile_speed();
	float get_projectile_gravity();
	float get_zoom_fov();
	int get_ammo();

private:
	float projectile_scale;
	float projectile_speed;
	float zoom_fov;
	int ammo;
};

void WeaponXEntity::update(Memory& mem)
{
	extern uint64_t g_Base;
	uint64_t entitylist = mem.Read<uint64_t>(baseAddress + OFFSET_ENTITYLIST);
	uint64_t wephandle = mem.Read<uint64_t>(LocalPLayer + OFFSET_WEAPON);
	wephandle &= 0xffff;

	uint64_t wep_entity = 0;
	mem.Read<uint64_t>(entitylist + (wephandle << 5), wep_entity);

	mem.Read<float>(wep_entity + OFFSET_BULLET_SPEED, projectile_speed);
	mem.Read<float>(wep_entity + OFFSET_BULLET_GRAVITY, projectile_scale);
	mem.Read<float>(wep_entity + OFFSET_ZOOM_FOV, zoom_fov);
	
    mem.Read<int>(wep_entity + OFFSET_AMMO, ammo);
}

float WeaponXEntity::get_projectile_speed()
{
	return projectile_speed;
}

int  WeaponXEntity::get_ammo()
{
	return ammo;
}

float WeaponXEntity::get_projectile_gravity()
{
	return 750.0f * projectile_scale;
}

float WeaponXEntity::get_zoom_fov()
{
	return zoom_fov;
}

Vector prediction(Memory& mem,Vector LPlayerpos, Vector Targetpos,  uint64_t Ttarget)
{
	WeaponXEntity curweap = WeaponXEntity();
	curweap.update(mem);
	float BulletSpeed = curweap.get_projectile_speed();
	//printf("%f\n", BulletSpeed);
	float BulletGrav = curweap.get_projectile_gravity();
	//printf("%f\n", BulletGrav);

	float distance = LPlayerpos.DistTo(mem.Read<Vector>(Ttarget+OFFSET_ORIGIN));
	float time = distance / BulletSpeed;
	Vector BulletGravChange = { 0, BulletGrav * time, 0 };
	Vector TargetVel = mem.Read<Vector>(Ttarget+ OFFSET_ABS_VELOCITY);
	Vector MovementChange = TargetVel * time;
	Vector FinalPos = Targetpos + MovementChange + BulletGravChange;
	return FinalPos;
}
struct PredictCtx
{
	Vector StartPos;
	Vector TargetPos;
	Vector TargetVel;
	float BulletSpeed;
	float BulletGravity;

	Vector2D AimAngles;
};

Vector ExtrapolatePos(const PredictCtx& Ctx, float Time)
{
	return Ctx.TargetPos + (Ctx.TargetVel * Time);
}

bool OptimalPitch(const PredictCtx& Ctx, const Vector2D& Dir2D, float* OutPitch)
{
	float Vel = Ctx.BulletSpeed, Grav = Ctx.BulletGravity, DirX = Dir2D.x, DirY = Dir2D.y;
	float Root = Vel * Vel * Vel * Vel - Grav * (Grav * DirX * DirX + 2.f * DirY * Vel * Vel);
	if (Root >= 0.f) { *OutPitch = atanf((Vel * Vel - sqrt(Root)) / (Grav * DirX)); return true; }
	return false;
}

bool SolveTrajectory(PredictCtx& Ctx, const Vector& ExtrPos, float* TravelTime)
{
	Vector Dir = ExtrPos - Ctx.StartPos;
	Vector2D Dir2D = { sqrtf(Dir.x * Dir.x + Dir.y * Dir.y), Dir.z };

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
bool BulletPredict(PredictCtx& Ctx)
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
			Ctx.AimAngles = { -RAD2DEG(Ctx.AimAngles.x), RAD2DEG(Ctx.AimAngles.y) };
			return true;
		}
	}
	return false;
}



QAngle CalculateBestBoneAim(Memory&mem,uint64_t aimbotTarg,int max_dists, int smothnes, float max_fov)
{
	if (target == NULL)
	{
		return QAngle(0,0,0);
	}
	Vector LocalCamera = mem.Read<Vector>(LocalPLayer+OFFSET_CAMERAPOS);
	//Vector TargetBonePosition = target.getBonePosition(bone);
	//get TartgetBonePosition by using hitboxpos
	Vector TargetBonePosition = getBonePosition(mem,aimbotTarg,boneLock);
	QAngle CalculatedAngles = QAngle(0, 0, 0);

	WeaponXEntity curweap = WeaponXEntity();
	curweap.update(mem);
	float BulletSpeed = curweap.get_projectile_speed();
	float BulletGrav = curweap.get_projectile_gravity();
	float zoom_fov = curweap.get_zoom_fov();

	if (zoom_fov != 0.0f && zoom_fov != 1.0f)
	{
		max_fov *= zoom_fov / 90.0f;
	}

	// more accurate prediction
	if (BulletSpeed > 1.f)
	{
		PredictCtx Ctx;
		Ctx.StartPos = LocalCamera;
		Ctx.TargetPos = TargetBonePosition;
		Ctx.BulletSpeed = BulletSpeed - (BulletSpeed * 0.08);
		Ctx.BulletGravity = BulletGrav + (BulletGrav * 0.05);
		Ctx.TargetVel = mem.Read<Vector>(aimbotTarg+OFFSET_ABS_VELOCITY);

		if (BulletPredict(Ctx))
			CalculatedAngles = QAngle{Ctx.AimAngles.x, Ctx.AimAngles.y, 0.f};
	}

	if (CalculatedAngles == QAngle(0, 0, 0))
		CalculatedAngles = CalcAngle(LocalCamera, TargetBonePosition);

	QAngle ViewAngles = mem.Read<QAngle>(LocalPLayer+OFFSET_VIEWANGLES);
	QAngle SwayAngles = mem.Read<QAngle>(LocalPLayer+OFFSET_BREATH_ANGLES);

	// remove sway and recoil
	//if (aim_no_recoil)
	CalculatedAngles -= SwayAngles - ViewAngles;

	NormalizeAngles(CalculatedAngles);
	QAngle Delta = CalculatedAngles - ViewAngles;
	double fov = GetFov(SwayAngles, CalculatedAngles);

	if (fov > max_fov)
	{
		return QAngle(0, 0, 0);
	}

	NormalizeAngles(Delta);

	QAngle SmoothedAngles = ViewAngles + Delta / smothnes;
	start_glowing(mem,target,targetGlowcolor);

	return SmoothedAngles;
}

bool check_in_fov(Vector2D screen_body, float FOVmax)
{
	float Dist = Utils::GetCrossDistance(screen_body.x, screen_body.y, (screenX / 2), (screenY / 2));

	if (Dist < FOVmax)
	{
		float Radius = fov;

		if (screen_body.x <= ((screenX / 2) + Radius) &&
			screen_body.x >= ((screenX / 2) - Radius) &&
			screen_body.y <= ((screenY / 2) + Radius) &&
			screen_body.y >= ((screenY / 2) - Radius))
		{
			FOVmax = Dist;
			return true;
		}
		return false;
	}
}

bool isAlive(Memory& mem, uint64_t ent)
{
	return mem.Read<int>(ent + OFFSET_LIFE_STATE) == 0;
}

void BestTarg(Memory& mem, uint64_t player,int LocalTeam)
{
	if(isVisibile(mem,player)==true)
	{
		if(isKnocked(mem,player)==false)
		{
			Vector LPOS = mem.Read<Vector>(LocalPLayer+OFFSET_ORIGIN);

			Vector2D screenPos;
			Vector POS = getBonePosition(mem,player,boneLock);
			if(ToMeters(POS.DistTo(LPOS)) <= dist)
	        {
	        	Vector2D screenPos;
				world_to_screen(mem,POS,screenPos);
				if(check_in_fov(screenPos,360)==true)
				{
					target = player;
					
				}
	        }
			
		}
	}
}

void charge_rifle_hack(Memory& mem)
{
	int shooting = mem.Read<int>(baseAddress+in_attack+0x8);
	WeaponXEntity curweap = WeaponXEntity();
	curweap.update(mem);
	float BulletSpeed = curweap.get_projectile_speed();
	int ammo = curweap.get_ammo();

	if (ammo != 0 && BulletSpeed <= 1.f && shooting == 5)
	{
		mem.Write<float>(baseAddress + OFFSET_TIMESCALE + 0x68, std::numeric_limits<float>::min());
	}
	else
	{
		mem.Write<float>(baseAddress + OFFSET_TIMESCALE + 0x68, 1.f);
	}
}

void recoilControl(Memory& mem, float smoothnes)
{
	Vector view_angles = mem.Read<Vector>(LocalPLayer + OFFSET_VIEWANGLES);
	Vector punchAngle = mem.Read<Vector>(LocalPLayer + OFFSET_AIMPUNCH);

	Vector newAngle = view_angles + (old_aimpunch - punchAngle) * (smoothnes / 100.f);;

	newAngle.Normalize();

	mem.Write<Vector>(LocalPLayer+OFFSET_VIEWANGLES,newAngle);
	old_aimpunch = punchAngle;
}

//bool IsInCrossHair(Memory& mem, uint64_t player) 
//{
//	auto tmp_lastCrosshairTargetTime = mem.Read<float>(baseAddress, player + OFFSET_CROSSHAIR_LAST);
//
//	
//	if (!tmp_lastCrosshairTargetTime)return false;
//	if (tmp_lastCrosshairTargetTime == -1.f)return false;
//	
//	auto isCrosshair = tmp_lastCrosshairTargetTime > iter->lastCrossHair;
//	iter->lastCrossHair = tmp_lastCrosshairTargetTime;
//	return isCrosshair;
//}