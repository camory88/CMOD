#include <iostream>
#include <thread>
#include "classs/ALLclass.hpp"
#include "UI/window.hpp"

c_matrix get_matrix(Memory &mem)
{
    uint64_t viewRenderer = mem.Read<uint64_t>(baseAddress + offsets::OFFSET_RENDER);
    uint64_t viewMatrix = mem.Read<uint64_t>(viewRenderer + offsets::OFFSET_MATRIX);

    return mem.Read<c_matrix>(viewMatrix);
}

bool world_to_screen(Memory &mem, Vector in, Vector2D &out)
{

    float *m_vMatrix = get_matrix(mem).matrix;
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
        union
        {
            float f;
            uint32_t i;
        } z = {x};
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
        if ((y % 2) == 0)
        {
            return temp * temp;
        }
        else
        {
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

void entity_loop(Memory &mem)
{

    Entity LocalPlayer = Entity(localPTR);
    target = NULL;

    for (int i = 0; i < settings::indexNum; ++i)
    {
        uint64_t entityPTR = mem.Read<uint64_t>(baseAddress + offsets::OFFSET_ENTITYLIST + (i << 5));
        if (entityPTR == 0x0)
            continue;
        Entity entity(entityPTR);
        if (entity.isDummy(mem) || entity.isPlayer(mem))
        {
            if (entity.isAlive(mem))
            {
                char name[32];
                get_name(mem, i - 1, name);
                players[i] = {
                    true,
                    entity.isAlive(mem),
                    ToMeters(entity.BasePos(mem).DistTo(LocalPlayer.BasePos(mem))),
                    entity.Team(mem),
                    std::string(name)};
            }
            else
            {
                char name[32];
                get_name(mem, i - 1, name);
                players[i] = {
                    false,
                    entity.isAlive(mem),
                    ToMeters(entity.BasePos(mem).DistTo(LocalPlayer.BasePos(mem))),
                    entity.Team(mem),
                    std::string(name)};
            }

            if (settings::nameSpoof)
            {
            }
        }

        if (settings::Lteam_Glow)
        {
            if (entity.Team(mem) == LocalPlayer.Team(mem) && entity.isPlayer(mem))
            {
                entity.glow(mem, settings::Lteam_Glow_color);
            }
            else if (entity.isGlowing(mem))
            {
                entity.disableGlow(mem);
            }
        }
        else
        {
            // if(entity.isGlowing(mem))
            //{
            //     entity.disableGlow(mem);
            // }
        }
        if (entity.ptr == localPTR)
            continue;

        if (entity.isDummy(mem))
        {
            if (settings::player_Glow)
            {
                entity.glow(mem, settings::dummy_glow_color);
            }
            else
            {
                if (entity.isGlowing(mem))
                {
                    entity.disableGlow(mem);
                }
            }
            if (!entity.isVisibile(mem))
                continue;
            if (entity.Health(mem) <= 1)
                continue;
            Vector2D screen;
            Vector pos = entity.V_BonePos(mem, settings::aimBone);
            world_to_screen(mem, pos, screen);
            if (check_in_fov(screen, settings::FOV))
            {
                target = entity.ptr;
                if (settings::target_glow)
                {
                    entity.glow(mem, settings::target_glow_color);
                }
                else
                {
                    if (entity.isGlowing(mem))
                    {
                        entity.disableGlow(mem);
                    }
                }
            }
        }
        else if (entity.isPlayer(mem))
        {

            if (!entity.isAlive(mem))
                continue;
            if (entity.isKnocked(mem))
                continue;

            if (settings::team_glow)
            {
                int dist = entity.BasePos(mem).DistTo(LocalPlayer.BasePos(mem));
                if (ToMeters(dist) <= settings::GLowDist)
                {

                    switch (entity.Team(mem))
                    {
                    case 0:
                        settings::team_Glow_color = Vector4(1.0f, 0.0f, 0.0f, 1.f); // Red
                        break;
                    case 1:
                        settings::team_Glow_color = Vector4(0.0f, 1.0f, 0.0f, 1.f); // Green
                        break;
                    case 2:
                        settings::team_Glow_color = Vector4(0.0f, 0.0f, 1.0f, 1.f); // Blue
                        break;
                    case 3:
                        settings::team_Glow_color = Vector4(0.0f, 1.0f, 1.0f, 1.f); // Cyan
                        break;
                    case 4:
                        settings::team_Glow_color = Vector4(1.0f, 0.0f, 1.0f, 1.f); // Magenta
                        break;
                    case 5:
                        settings::team_Glow_color = Vector4(1.0f, 1.0f, 0.0f, 1.f); // Yellow
                        break;
                    case 6:
                        settings::team_Glow_color = Vector4(0.0f, 0.0f, 0.0f, 1.f); // Black
                        break;
                    case 7:
                        settings::team_Glow_color = Vector4(1.0f, 1.0f, 1.0f, 1.f); // White
                        break;
                    case 8:
                        settings::team_Glow_color = Vector4(0.5f, 0.5f, 0.5f, 1.f); // Gray
                        break;
                    case 9:
                        settings::team_Glow_color = Vector4(0.75f, 0.75f, 0.75f, 1.f); // Light Gray
                        break;
                    case 10:
                        settings::team_Glow_color = Vector4(0.25f, 0.25f, 0.25f, 1.f); // Dark Gray
                        break;
                    case 11:
                        settings::team_Glow_color = Vector4(1.0f, 0.5f, 0.0f, 1.f); // Orange
                        break;
                    case 12:
                        settings::team_Glow_color = Vector4(0.5f, 0.0f, 0.5f, 1.f); // Purple
                        break;
                    case 13:
                        settings::team_Glow_color = Vector4(0.0f, 0.5f, 0.5f, 1.f); // Teal
                        break;
                    case 14:
                        settings::team_Glow_color = Vector4(1.0f, 0.75f, 0.75f, 1.f); // Pink
                        break;
                    case 15:
                        settings::team_Glow_color = Vector4(0.6f, 0.4f, 0.2f, 1.f); // Brown
                        break;
                    case 16:
                        settings::team_Glow_color = Vector4(0.0f, 0.0f, 0.5f, 1.f); // Navy
                        break;
                    case 17:
                        settings::team_Glow_color = Vector4(0.5f, 0.5f, 0.0f, 1.f); // Olive
                        break;
                    case 18:
                        settings::team_Glow_color = Vector4(0.5f, 0.0f, 0.0f, 1.f); // Maroon
                        break;
                    case 19:
                        settings::team_Glow_color = Vector4(0.0f, 0.5f, 0.0f, 1.f); // Lime
                        break;
                    case 20:
                        settings::team_Glow_color = Vector4(0.0f, 1.0f, 1.0f, 1.f); // Aqua
                        break;
                    case 21:
                        settings::team_Glow_color = Vector4(0.29f, 0.0f, 0.51f, 1.f); // Indigo
                        break;
                    case 22:
                        settings::team_Glow_color = Vector4(1.0f, 0.5f, 0.31f, 1.f); // Coral
                        break;
                    case 23:
                        settings::team_Glow_color = Vector4(1.0f, 0.84f, 0.0f, 1.f); // Gold
                        break;
                    case 24:
                        settings::team_Glow_color = Vector4(0.9f, 0.9f, 0.98f, 1.f); // Lavender
                        break;
                    default:
                        settings::team_Glow_color = Vector4(1.0f, 0.0f, 0.0f, 1.f); // Red
                        break;
                    }

                    entity.glow(mem, settings::team_Glow_color);
                }
            }
            else if (settings::player_Glow)
            {
                int dist = entity.BasePos(mem).DistTo(LocalPlayer.BasePos(mem));
                if (ToMeters(dist) <= settings::GLowDist)
                {
                    entity.glow(mem, settings::player_Glow_color);
                }
            }
            else
            {
                if (entity.isGlowing(mem))
                {
                    entity.disableGlow(mem);
                }
            }

            if (entity.Team(mem) == LocalPlayer.Team(mem))
                continue;

            if (!entity.isVisibile(mem))
                continue;
            int dist = entity.BasePos(mem).DistTo(LocalPlayer.BasePos(mem));
            if (ToMeters(dist) >= settings::AimDist)
                continue;

            Vector2D screen;
            Vector pos = entity.V_BonePos(mem, settings::aimBone);
            world_to_screen(mem, pos, screen);
            if (check_in_fov(screen, settings::FOV))
            {
                target = entity.ptr;
            }
        }
        else if (entity.isItem(mem))
        {
            if (settings::loot_Glow)
            {
                mem.Write<int>(entity.ptr + 0x02f0, 1363184265);
            }
            else
            {
                if (entity.isGlowing(mem))
                {
                    mem.Write<int>(entity.ptr + 0x02f0, 0);
                }
            }
        }
        else if (entity.isWayPoint(mem))
        {
            settings::way_poit_pos = entity.BasePos(mem); // CPlayerWaypoint

            if (settings::way_poit_pos.x == 0 && settings::way_poit_pos.y == 0 && settings::way_poit_pos.z == 0)
                continue;
        }
    }
}

void localPlayer_function(Memory &mem)
{

    Entity LocalPlayer(localPTR);
    if (true)
    {

        uint64_t WeaponModeHandle = mem.Read<uint64_t>(localPTR + offsets::OFFSET_ViewModels) & 0xFFFF; // m_hViewModels
        uint64_t ModelPtr = mem.Read<uint64_t>(baseAddress + offsets::OFFSET_ENTITYLIST + WeaponModeHandle * 0x20);

        if (settings::weapon_glow)
        {
            mem.Write<int>(ModelPtr + offsets::OFFSET_GLOW_ENABLE, 1);
            mem.Write<int>(ModelPtr + offsets::OFFSET_GLOW_THROUGH_WALLS, 5);
            // GeneralGlowMode, BorderGlowMode, BorderSize, TransparentLevel;
            // isSemiAuto = 0x1C2C

            mem.Write<glowMode>(ModelPtr + offsets::GLOW_TYPE, {118, -86, 100, 0});
            if (settings::rainbow_weapon_glow)
                settings::weapon_glow_color = {std::get<0>(Rainbow(settings::rainbowSpeed)), std::get<1>(Rainbow(settings::rainbowSpeed)), std::get<2>(Rainbow(settings::rainbowSpeed)), 1.f};
            mem.Write<Vector4>(ModelPtr + offsets::GLOW_COLOR, settings::weapon_glow_color);
        }
        else
        {
            mem.Write<int>(ModelPtr + offsets::OFFSET_GLOW_ENABLE, 2);
            mem.Write<int>(ModelPtr + offsets::OFFSET_GLOW_THROUGH_WALLS, 2);
        }

        if (settings::hand_glow)
        {
        }
        else
        {
        }
    }

    if (!settings::thierdPerson)
    {
        int a = mem.Read<int>(baseAddress + offsets::OFFSET_THIRDPERSON);
        int b = mem.Read<int>(localPTR + offsets::OFFSET_THIRDPERSON_SV);
        if (a != -1 && b != 0)
        {
            mem.Write<int>(baseAddress + offsets::OFFSET_THIRDPERSON, -1);
            mem.Write<int>(localPTR + offsets::OFFSET_THIRDPERSON_SV, 0);
        }
    }
    else
    {
        int a = mem.Read<int>(baseAddress + offsets::OFFSET_THIRDPERSON);
        int b = mem.Read<int>(localPTR + offsets::OFFSET_THIRDPERSON_SV);
        if (a != 1 && b != 1)
        {
            mem.Write<int>(baseAddress + offsets::OFFSET_THIRDPERSON, 1);
            mem.Write<int>(localPTR + offsets::OFFSET_THIRDPERSON_SV, 1);
        }
    }

    if (target != NULL)
    {
        QAngle CalculatedAngles = QAngle(0, 0, 0);
        Entity targ(target);

        if (settings::target_glow)
        {
            targ.glow(mem, settings::target_glow_color);
        }
        else
        {
            if (targ.isGlowing(mem))
            {
                targ.disableGlow(mem);
            }
        }

        if (mem.Read<int>(baseAddress + offsets::OFFSET_IS_ZOOM + 0x8) == 5 && mem.Read<int>(baseAddress + offsets::OFFSET_IS_Shooting + 0x8) == 5)
        {
            QAngle angle = targ.CalculateBestBoneAim(mem, LocalPlayer, settings::aimBone, settings::AimDist, settings::rcs_aimbot, settings::FOV);
            if (angle.x != 0)
                LocalPlayer.setViewAngles_QAngle(mem, angle);
        }
        else if (mem.Read<int>(baseAddress + offsets::OFFSET_IS_ZOOM + 0x8) == 4 && mem.Read<int>(baseAddress + offsets::OFFSET_IS_Shooting + 0x8) == 5)
        {
            QAngle angle = targ.CalculateBestBoneAim(mem, LocalPlayer, settings::aimBone, settings::AimDist / 4, settings::rcs_aimbot / 2, settings::FOV);
            if (angle.x != 0)
                LocalPlayer.setViewAngles_QAngle(mem, angle);
        }
        else if (mem.Read<int>(baseAddress + offsets::OFFSET_IS_ZOOM + 0x8) == 4 && mem.Read<int>(baseAddress + offsets::OFFSET_IS_Shooting + 0x8) == 4 && settings::RSC)
        {
            LocalPlayer.recoilControl(mem, 100.f);
        }
    }
    else
    {
        if (settings::RSC)
        {
            LocalPlayer.recoilControl(mem, 100.f);
        }
    }

    if (settings::goToWaypoint)
    {
    }
}

void printMemoryLocation(Memory &apex, uint64_t location, uint64_t size)
{
    uint8_t *scriptReadBytes = new uint8_t[size];

    apex.ReadArray(location, scriptReadBytes, size);

    int lineNum = 0;

    printf("Script:\n---------------------------------------------------------------------------\n");
    for (int i = 0; i < size; i++)
    {
        // print as string but add line numbers on the left
        if (scriptReadBytes[i] == '\n')
        {
            printf("%c%d: ", scriptReadBytes[i], lineNum);
            lineNum++;
        }
        else
        {
            printf("%c", scriptReadBytes[i]);
        }
    }
    printf("\n---------------------------------------------------------------------------\n");
    printf("End Script\n\n");
}

void ReadScriptFinder(Memory &mem)
{
}

void script_load(Memory &mem, char *scriptArry, size_t size)
{

    uint64_t base_addr = baseAddress + 0x29276597A00; // 0x29276597A00
    bool found = false;
    while (!found)
    {
        base_addr = base_addr + sizeof(char);

        mem.ReadArray<char>(base_addr, scriptArry, size);
        printf("addr %p\n", base_addr);
    }
    printf("----------------\n");
    printf(" script found\n");
}
