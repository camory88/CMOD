#include "ALLclass.hpp"

float WeaponXEntity::get_projectile_speed(Memory &mem)
{
	return mem.Read<float>(ptr + offsets::OFFSET_BULLET_SPEED);
}

float WeaponXEntity::get_projectile_gravity(Memory &mem)
{
	return 750.0f * mem.Read<float>(ptr + offsets::OFFSET_BULLET_SCALE);
}

float WeaponXEntity::get_zoom_fov(Memory &mem)
{
	return mem.Read<float>(ptr + offsets::OFFSET_ZOOM_FOV);
}

int WeaponXEntity::get_ammo(Memory &mem)
{
	return mem.Read<int>(ptr + offsets::OFFSET_AMMO);
}

int WeaponXEntity::get_Fire_type(Memory &mem)
{
	return mem.Read<int>(ptr + offsets::OFFSET_SEMI_AUTO);
}