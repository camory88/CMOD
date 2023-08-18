#include <iostream>
#include <thread>
#include "ALLclass.hpp"
#include "UI/window.hpp"

c_matrix get_matrix(Memory& mem)
{
	uint64_t viewRenderer = mem.Read<uint64_t>(baseAddress + offsets::OFFSET_RENDER);
	uint64_t viewMatrix = mem.Read<uint64_t>(viewRenderer + offsets::OFFSET_MATRIX);

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
namespace Utils
{
	float sqrtf_(float x)
	{
		union { float f; uint32_t i; } z = { x };
		z.i = 0x5f3759df - (z.i >> 1);
		z.f *= (1.5f - (x * 0.5f * z.f * z.f));
		z.i = 0x7EEEEEEE - z.i;
		return z.f;
	}


	double powf_(double x, int y)
	{
		double temp;
		if (y == 0)
			return 1;
		temp = powf_(x, y / 2);
		if ((y % 2) == 0) {
			return temp * temp;
		}
		else {
			if (y > 0)
				return x * temp * temp;
			else
				return (temp * temp) / x;
		}
	}
    double GetCrossDistance(double x1, double y1, double x2, double y2)
	{
		return sqrtf_(powf_((float)(x1 - x2), (float)2) + powf_((float)(y1 - y2), (float)2));
	}
}

bool check_in_fov(Vector2D screen_body, float FOVmax)
{
	float Dist = Utils::GetCrossDistance(screen_body.x, screen_body.y, (screenX / 2), (screenY / 2));

	if (Dist < FOVmax)
	{
		float Radius = settings::FOV;

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
    return false;
}

void entity_loop(Memory& mem)
{
    Entity LocalPlayer = Entity(localPTR);
    target = NULL;
    
    for(int i = 0; i < settings::indexNum; ++i)
    {
        uint64_t entityPTR = mem.Read<uint64_t>(baseAddress + offsets::OFFSET_ENTITYLIST+ (i << 5));
        if(entityPTR == 0x0) continue;
        Entity entity(entityPTR);
        if(settings::Lteam_Glow)
        {
            if(entity.Team(mem) == LocalPlayer.Team(mem) && entity.isPlayer(mem))
            {
                entity.glow(mem,settings::Lteam_Glow_color);
            }
            else if(entity.isGlowing(mem))
            {
                entity.disableGlow(mem);
            }
            
        }
        else
        {
            if(entity.isGlowing(mem))
            {
                entity.disableGlow(mem);
            }
        }

        if(entity.ptr == localPTR) continue;
        if(entity.isDummy(mem))
        {   
            if(settings::player_Glow)
            {
                entity.glow(mem,settings::dummy_glow_color);
            }
            else
            {
                if(entity.isGlowing(mem))
                {
                    entity.disableGlow(mem);
                }
            }
            if(!entity.isVisibile(mem)) continue; 
            if(entity.Health(mem) <= 1) continue;
            Vector2D screen;
            Vector pos = entity.V_BonePos(mem,settings::aimBone);
            world_to_screen(mem,pos,screen);
            if(check_in_fov(screen,settings::FOV))
            {
                target = entity.ptr;
                if(settings::target_glow)
                {
                    entity.glow(mem,settings::target_glow_color);
                }
                else
                {
                    if(entity.isGlowing(mem))
                    {
                        entity.disableGlow(mem);
                    }
                }
            }
        }
        else if (entity.isPlayer(mem))
        {   
            
            if(!entity.isAlive(mem)) continue;
            if(entity.isKnocked(mem)) continue;
            if(entity.Team(mem) == LocalPlayer.Team(mem)) continue;
            
            if(settings::player_Glow)
            {
                int dist = entity.BasePos(mem).DistTo(LocalPlayer.BasePos(mem));
                if(ToMeters(dist) <= settings::GLowDist)
                {
                    entity.glow(mem,settings::player_Glow_color);
                }
            }
            else
            {
                if(entity.isGlowing(mem))
                {
                    entity.disableGlow(mem);
                }
            }

            if(!entity.isVisibile(mem)) continue; 
            int dist = entity.BasePos(mem).DistTo(LocalPlayer.BasePos(mem));
            if(ToMeters(dist) >= settings::AimDist) continue;

            Vector2D screen;
            Vector pos = entity.V_BonePos(mem,settings::aimBone);
            world_to_screen(mem,pos,screen);
            if(check_in_fov(screen,settings::FOV))
            {
                target = entity.ptr;
                if(settings::target_glow)
                {
                    entity.glow(mem,settings::target_glow_color);
                }
                else
                {
                    if(entity.isGlowing(mem))
                    {
                        entity.disableGlow(mem);
                    }
                }
            }
            
        }
        else if (entity.isItem(mem))
        {
            if(settings::loot_Glow)
            {
                mem.Write<int>(entity.ptr + 0x02f0, 1363184265); 
            }
            else
            {
                if(entity.isGlowing(mem))
                {
                    entity.disableGlow(mem);
                }
            }




        }
        
        
            
        
    }
}

void localPlayer_function(Memory& mem)
{
    Entity LocalPlayer(localPTR);
    
    if(settings::Skin_changer)
    {
        uint64_t g_Base;
	    uint64_t entitylist = mem.Read<uint64_t>(baseAddress + offsets::OFFSET_ENTITYLIST);
	    uint64_t wephandle = mem.Read<uint64_t>(localPTR + offsets::OFFSET_WEAPON);
	    wephandle &= 0xffff;

	    uint64_t wep_entity = 0;
	    mem.Read<uint64_t>(entitylist + (wephandle << 5), wep_entity);
	    if (wep_entity != 0)
            mem.Write<int>(wep_entity + offsets::OFFSET_SKIN, 4);
    }
    if(!settings::thierdPerson)
    {
        int a = mem.Read<int>(baseAddress + offsets::OFFSET_THIRDPERSON);
	    int b = mem.Read<int>(localPTR + offsets::OFFSET_THIRDPERSON_SV);
        if(a != -1 && b != 0)
        {
            mem.Write<int>(baseAddress + offsets::OFFSET_THIRDPERSON,-1);
	        mem.Write<int>(localPTR + offsets::OFFSET_THIRDPERSON_SV,0);
        }
    }
    else
    {
        int a = mem.Read<int>(baseAddress + offsets::OFFSET_THIRDPERSON);
	    int b = mem.Read<int>(localPTR + offsets::OFFSET_THIRDPERSON_SV);
        if(a != 1 && b != 1)
        {
            mem.Write<int>(baseAddress + offsets::OFFSET_THIRDPERSON,1);
	        mem.Write<int>(localPTR + offsets::OFFSET_THIRDPERSON_SV,1);
        }
    }

    
    if(target != NULL)
    {
        QAngle CalculatedAngles = QAngle(0, 0, 0);
        Entity targ(target);
        QAngle angle = targ.CalculateBestBoneAim(mem,LocalPlayer,settings::aimBone,settings::AimDist,settings::rcs_aimbot,settings::FOV);
        if(mem.Read<int>(baseAddress + offsets::OFFSET_IS_ZOOM + 0x8) ==5)
        {
            if(angle.x != 0)
                LocalPlayer.setViewAngles_QAngle(mem,angle);
        }
    }
    else
    {
        LocalPlayer.recoilControl(mem,100.f);
    }
}
