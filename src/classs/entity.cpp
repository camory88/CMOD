#include "ALLclass.hpp"

bool Entity::isDummy(Memory &mem)
{
	char class_name[33] = {};
	get_class_name(mem, class_name);

	return strncmp(class_name, "CAI_BaseNPC", 11) == 0;
}

bool Entity::isWayPoint(Memory &mem)
{
	char class_name[33] = {};
	get_class_name(mem, class_name);

	return strncmp(class_name, "CPlayerWaypoint", 15) == 0;
}

bool Entity::isItem(Memory &mem)
{
	char class_name[33] = {};
	get_class_name(mem, class_name);

	return strncmp(class_name, "CPropSurvival", 13) == 0;
}

bool Entity::isKnocked(Memory &mem)
{
	return mem.Read<int>(ptr + offsets::OFFSET_BLEED_OUT_STATE) > 0;
}

bool Entity::isPlayer(Memory &mem)
{
	return mem.Read<uint64_t>(ptr + offsets::OFFSET_NAME) == 125780153691248;
}

bool Entity::isAlive(Memory &mem)
{
	return mem.Read<int>(ptr + offsets::OFFSET_LIFE_STATE) == 0;
}

bool Entity::isVisibile(Memory &mem)
{
	return GlobalVars.curtime - getLastVisTime(mem) < GlobalVars.interval_per_tick;
}

int Entity::Team(Memory &mem)
{
	return mem.Read<int>(ptr + offsets::OFFSET_TEAM);
}

int Entity::Health(Memory &mem)
{
	return mem.Read<int>(ptr + offsets::OFFSET_HEALTH);
}
int Entity::Shield(Memory &mem)
{
	return mem.Read<int>(ptr + offsets::OFFSET_SHIELD);
}
int Entity::MaxHealth(Memory &mem)
{
	return mem.Read<int>(ptr + offsets::OFFSET_MAXSHIELD);
}
float Entity::getLastVisTime(Memory &mem)
{
	return mem.Read<float>(ptr + offsets::OFFSET_VISIBLE_TIME);
}

Vector Entity::V_BonePos(Memory &mem, int BoneId)
{
	Vector position = mem.Read<Vector>(ptr + offsets::OFFSET_ORIGIN);
	uintptr_t boneArray = mem.Read<uintptr_t>(ptr + offsets::OFFSET_BONES);
	Vector bone = Vector();
	uint32_t boneloc = (BoneId * 0x30);
	bone_t bo = {};
	mem.Read<bone_t>(boneArray + boneloc, bo);
	bone.x = bo.x + position.x;
	bone.y = bo.y + position.y;
	bone.z = bo.z + position.z;
	return bone;
}

QAngle Entity::Q_BonePos(Memory &mem, int BoneId)
{
	QAngle position = mem.Read<QAngle>(ptr + offsets::OFFSET_ORIGIN);
	uintptr_t boneArray = mem.Read<uintptr_t>(ptr + offsets::OFFSET_BONES);
	QAngle bone = QAngle();
	uint32_t boneloc = (BoneId * 0x30);
	bone_t bo = {};
	mem.Read<bone_t>(boneArray + boneloc, bo);
	bone.x = bo.x + position.x;
	bone.y = bo.y + position.y;
	bone.z = bo.z + position.z;
	return bone;
}

Vector Entity::BasePos(Memory &mem)
{
	return mem.Read<Vector>(ptr + offsets::OFFSET_ORIGIN);
}
Vector Entity::V_CamPos(Memory &mem)
{
	return mem.Read<Vector>(ptr + offsets::OFFSET_CAMERAPOS);
}
QAngle Entity::Q_CamPos(Memory &mem)
{
	return mem.Read<QAngle>(ptr + offsets::OFFSET_CAMERAPOS);
}

Vector Entity::V_VeiwAngle(Memory &mem)
{
	return mem.Read<Vector>(ptr + offsets::OFFSET_VIEWANGLES);
}

QAngle Entity::Q_VeiwAngle(Memory &mem)
{
	return mem.Read<QAngle>(ptr + offsets::OFFSET_VIEWANGLES);
}

Vector Entity::V_SwayAngle(Memory &mem)
{
	return mem.Read<Vector>(localPTR + offsets::OFFSET_BREATH_ANGLES);
}

QAngle Entity::Q_SwayAngle(Memory &mem)
{
	return mem.Read<QAngle>(localPTR + offsets::OFFSET_BREATH_ANGLES);
}

void Entity::setViewAngles_Vector(Memory &mem, Vector angle)
{
	mem.Write<Vector>(ptr + offsets::OFFSET_VIEWANGLES, angle);
}
void Entity::setViewAngles_QAngle(Memory &mem, QAngle angle)
{
	mem.Write<QAngle>(ptr + offsets::OFFSET_VIEWANGLES, angle);
}

void Entity::recoilControl(Memory &mem, float smoothnes)
{
	Vector view_angles = mem.Read<Vector>(ptr + offsets::OFFSET_VIEWANGLES);
	Vector punchAngle = mem.Read<Vector>(ptr + offsets::OFFSET_AIMPUNCH);

	Vector newAngle = view_angles + (old_aimpunch - punchAngle) * (smoothnes / 100.f);
	;

	newAngle.Normalize();

	mem.Write<Vector>(ptr + offsets::OFFSET_VIEWANGLES, newAngle);
	old_aimpunch = punchAngle;
}

void Entity::get_class_name(Memory &mem, char *out_str)
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
QAngle Entity::CalculateBestBoneAim(Memory &mem, Entity LocaPlayer, int boneLock, int max_dists, int smothnes, float max_fov)
{
	if (target == NULL)
	{
		return QAngle(0, 0, 0);
	}
	Vector LocalCamera = LocaPlayer.V_CamPos(mem);
	// Vector TargetBonePosition = target.getBonePosition(bone);
	// get TartgetBonePosition by using hitboxpos
	Vector TargetBonePosition = V_BonePos(mem, boneLock);
	QAngle CalculatedAngles = QAngle(0, 0, 0);

	WeaponXEntity curweap = WeaponXEntity(getWeapon(mem));
	
	// float BulletSpeed = curweap.get_projectile_speed();
	// float BulletGrav = curweap.get_projectile_gravity();
	float zoom_fov = curweap.get_zoom_fov(mem);

	if (zoom_fov != 0.0f && zoom_fov != 1.0f)
	{
		max_fov *= zoom_fov / 90.0f;
	}

	// more accurate prediction
	// if (BulletSpeed > 1.f)
	//{
	//	PredictCtx Ctx;
	//	Ctx.StartPos = LocalCamera;
	//	Ctx.TargetPos = TargetBonePosition;
	//	Ctx.BulletSpeed = BulletSpeed - (BulletSpeed * 0.08);
	//	Ctx.BulletGravity = BulletGrav + (BulletGrav * 0.05);
	//	Ctx.TargetVel = mem.Read<Vector>(ptr+offsets::OFFSET_ABS_VELOCITY);
	//
	//	if (BulletPredict(Ctx))
	//		CalculatedAngles = QAngle{Ctx.AimAngles.x, Ctx.AimAngles.y, 0.f};
	//}

	if (CalculatedAngles == QAngle(0, 0, 0))
		CalculatedAngles = CalcAngle(LocalCamera, TargetBonePosition);

	QAngle ViewAngles = LocaPlayer.Q_VeiwAngle(mem);
	QAngle SwayAngles = LocaPlayer.Q_SwayAngle(mem);

	// remove sway and recoil
	if (settings::aimbot_noRecoil)
		CalculatedAngles -= SwayAngles - ViewAngles;

	NormalizeAngles(CalculatedAngles);
	QAngle Delta = CalculatedAngles - ViewAngles;
	double fov = GetFov(SwayAngles, CalculatedAngles);

	if (fov > max_fov)
	{
		return QAngle(0, 0, 0);
	}

	NormalizeAngles(Delta);
	
	settings::calangle = Vector2D(Delta.x,Delta.y);

	QAngle SmoothedAngles = ViewAngles + Delta / smothnes;

	return SmoothedAngles;
}

void Entity::glow(Memory &mem, Vector Color)
{

	//   GeneralGlowMode, BorderGlowMode, BorderSize, TransparentLevel
	// 0, 4, 69, 125 outline neonstripe
	// 0, 118, 100, 100  outline colored
	// 2, 108, 40, 96
	// glowMode glowStyle = { 0, 118, 125, 100 }; //Default Glow is Outline

	//mem.Write<Vector>(ptr + offsets::GLOW_COLOR, Color); // red
	//mem.Write<int>(ptr + offsets::OFFSET_GLOW_ENABLE, 1);							 // Enable Glow
	//mem.Write<int>(ptr + offsets::OFFSET_GLOW_THROUGH_WALLS, 2);					 // Enable Glow Through Walls
	//mem.Write<glowMode>(ptr + offsets::GLOW_TYPE, {118, -86, 50, 125});				 // Glow Mode
}



bool Entity::isGlowing(Memory &mem)
{
	//return mem.Read<int>(ptr + offsets::OFFSET_ITEM_GLOW) == 1363184265;
}

void Entity::enableGlow(Memory &mem)
{
	//mem.Write<int>(ptr + offsets::OFFSET_ITEM_GLOW, 1363184265);
}

void Entity::disableGlow(Memory &mem)
{
	//mem.Write<int>(ptr + offsets::OFFSET_ITEM_GLOW, 1411417991);
}


