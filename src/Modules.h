#include <iostream>
#include <thread>
#include "classs/ALLclass.hpp"
#include "UI/window.hpp"
bool choking = true;
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
        if (entity.isPlayer(mem))
        {
            if (!entity.isAlive(mem))
                continue;
            if (entity.isKnocked(mem))
                continue;
            if (entity.ptr == localPTR)
                continue;

            if (entity.Team(mem) == LocalPlayer.Team(mem))
                continue;
            int dist = entity.BasePos(mem).DistTo(LocalPlayer.BasePos(mem));
            switch (settings::GLOW_type)
            {
            case 0:
                if (entity.isGlowing(mem))
                    entity.disableGlow(mem);
                break;
            case 1:
                if (!entity.isGlowing(mem))
                {
                    if (ToMeters(dist) <= settings::GLowDist)
                    {
                        entity.glow(mem, settings::player_Glow_color);
                    }
                }
                break;
            case 2:
                if (!entity.isGlowing(mem))
                {
                    if (ToMeters(dist) <= settings::GLowDist)
                    {
                        Vector team_Glow_color = Vector(0.f, 0.f, 0.f);
                        switch (entity.Team(mem))
                        {
                        case 0:
                            team_Glow_color = Vector(1.0f, 0.0f, 0.f); // Red
                            break;
                        case 1:
                            team_Glow_color = Vector(0.0f, 1.0f, 0.0f); // Green
                            break;
                        case 2:
                            team_Glow_color = Vector(0.0f, 0.0f, 1.0f); // Blue
                            break;
                        case 3:
                            team_Glow_color = Vector(0.0f, 1.0f, 1.0f); // Cyan
                            break;
                        case 4:
                            team_Glow_color = Vector(1.0f, 0.0f, 1.0f); // Magenta
                            break;
                        case 5:
                            team_Glow_color = Vector(1.0f, 1.0f, 0.0f); // Yellow
                            break;
                        case 6:
                            team_Glow_color = Vector(0.0f, 0.0f, 0.0f); // Black
                            break;
                        case 7:
                            team_Glow_color = Vector(1.0f, 1.0f, 1.0f); // White
                            break;
                        case 8:
                            team_Glow_color = Vector(0.5f, 0.5f, 0.5f); // Gray
                            break;
                        case 9:
                            team_Glow_color = Vector(0.75f, 0.75f, 0.75f); // Light Gray
                            break;
                        case 10:
                            team_Glow_color = Vector(1.0f, 0.5f, 0.0f); // Orange
                            break;
                        case 12:
                            team_Glow_color = Vector(0.5f, 0.0f, 0.5f); // Purple
                            break;
                        case 13:
                            team_Glow_color = Vector(0.0f, 0.5f, 0.5f); // Teal
                            break;
                        case 14:
                            team_Glow_color = Vector(1.0f, 0.75f, 0.75f); // Pink
                            break;
                        case 15:
                            team_Glow_color = Vector(0.6f, 0.4f, 0.2f); // Brown
                            break;
                        case 16:
                            team_Glow_color = Vector(0.0f, 0.0f, 0.5f); // Navy
                            break;
                        case 17:
                            team_Glow_color = Vector(0.5f, 0.5f, 0.0f); // Olive
                            break;
                        case 18:
                            team_Glow_color = Vector(0.5f, 0.0f, 0.0f); // Maroon
                            break;
                        case 19:
                            team_Glow_color = Vector(0.0f, 0.5f, 0.0f); // Lime
                            break;
                        case 20:
                            team_Glow_color = Vector(0.0f, 1.0f, 1.0f); // Aqua
                            break;
                        case 21:
                            team_Glow_color = Vector(0.29f, 0.0f, 0.51f); // Indigo
                            break;
                        case 22:
                            team_Glow_color = Vector(1.0f, 0.5f, 0.31f); // Coral
                            break;
                        case 23:
                            team_Glow_color = Vector(1.0f, 0.84f, 0.0f); // Gold
                            break;
                        case 24:
                            team_Glow_color = Vector(0.9f, 0.9f, 0.98f); // Lavender
                            break;
                        default:
                            team_Glow_color = Vector(1.0f, 0.0f, 0.0f); // Red
                            break;
                        }
                        entity.glow(mem, team_Glow_color);
                    }
                }
                break;
            case 3:
                // health
                // Vector health_Glow_color = Vector(0.f, 0.f, 0.f);
                break;
            case 4:
                // rainbow
                if (!entity.isGlowing(mem))
                    entity.glow(mem, settings::RainBow_glow_color);
                break;

            default:
                if (entity.isGlowing(mem))
                    entity.disableGlow(mem);
                break;
            }

            if (!entity.isVisibile(mem))
                continue;

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
    }
}

void localPlayer_function(Memory &mem)
{
    settings::RainBow_glow_color = {std::get<0>(Rainbow(settings::rainbowSpeed)), std::get<1>(Rainbow(settings::rainbowSpeed)), std::get<2>(Rainbow(settings::rainbowSpeed))};
    settings::weapon_Glow_color = {std::get<0>(Rainbow(settings::rainbowSpeed)), std::get<1>(Rainbow(settings::rainbowSpeed)), std::get<2>(Rainbow(settings::rainbowSpeed))};

    Entity LocalPlayer(localPTR);

    if (settings::autojumpPath)
    {
        mem.Write<int>(baseAddress + offsets::in_jump + 0x8, 4);
        auto Gn = mem.Read<int>(localPTR + offsets::m_grapple + offsets::m_grappleAttached);
        if (Gn == 1)
        {
            mem.Write<int>(baseAddress + offsets::in_jump + 0x8, 5);
            mem.Write<int>(baseAddress + 0x0054, 1); // m_grappleActivateTime
        }
    }

    if (LocalPlayer.isAlive(mem) && !LocalPlayer.isKnocked(mem))
    {

        //if (settings::weapon_glow)
        //{
        //    uint64_t WeaponModeHandle = mem.Read<uint64_t>(localPTR + offsets::OFFSET_ViewModels) & 0xFFFF; // m_hViewModels
        //    uint64_t ModelPtr = mem.Read<uint64_t>(baseAddress + offsets::OFFSET_ENTITYLIST + WeaponModeHandle * 0x20);
//
        //    mem.Write<int>(ModelPtr + offsets::OFFSET_GLOW_ENABLE, 1);
        //    mem.Write<int>(ModelPtr + offsets::OFFSET_GLOW_THROUGH_WALLS, 5);
        //    mem.Write<glowMode>(ModelPtr + offsets::GLOW_TYPE, {118, -86, 100, 0});
//
        //    if (settings::rainbow_weapon_glow)
        //        mem.Write<Vector>(ModelPtr + offsets::GLOW_COLOR, settings::weapon_Glow_color);
        //    else
        //        mem.Write<Vector>(ModelPtr + offsets::GLOW_COLOR, settings::weapon_Glow_color);
        //}
        //else
        //{
        //    mem.Write<int>(ModelPtr + offsets::OFFSET_GLOW_ENABLE, 2);
        //    mem.Write<int>(ModelPtr + offsets::OFFSET_GLOW_THROUGH_WALLS, 5);
        //}
//
        //if (settings::hand_glow)
        //{
        //}
        //else
        //{
        //}
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

    if (target != NULL && settings::aimbot)
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

        if (mem.Read<int>(baseAddress + offsets::in_zoom + 0x8) == 5)
        {
            QAngle angle = targ.CalculateBestBoneAim(mem, LocalPlayer, settings::aimBone, settings::AimDist, settings::rcs_aimbot, settings::FOV);
            if (angle.x != 0)
                LocalPlayer.setViewAngles_QAngle(mem, angle);
        }
        else if (mem.Read<int>(baseAddress + offsets::in_jump + 0x8) == 4 && mem.Read<int>(baseAddress + offsets::in_attack + 0x8) == 4 && settings::RSC)
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

inline static void SilentAim(Memory &mem)
{
    auto netChannel = mem.Read<intptr_t>(baseAddress + NetChannel);
    float chokedCommands = mem.Read<float>(netChannel + 0x2028);
    printf("chokedCommands rn%f\n", chokedCommands);
    if (chokedCommands <= 0.f)
        return;

    printf("silent 1\n");
    int current_number = mem.Read<int>(baseAddress + CurrentCommand);
    int iDesiredCmdNumber = current_number + 1;
    auto cmdBase = mem.Read<intptr_t>(baseAddress + Commands + 248);
    auto old_usercmd = mem.Read<uint64_t>(cmdBase + (552 * (((intptr_t)iDesiredCmdNumber - 1) % 750)));
    auto usercmd = mem.Read<uint64_t>(cmdBase + (552 * ((intptr_t)iDesiredCmdNumber % 750)));
    printf("silent 2\n");
    while (mem.Read<int>((intptr_t)usercmd) < iDesiredCmdNumber)
    {
        std::this_thread::yield();
    }
    mem.Write<Vector2D>(old_usercmd + 0xC, settings::calangle);
    mem.Write<double>(netChannel + 0x2108, 0);
    printf("silent!\n");
}
inline static void Choke(Memory &mem, intptr_t netChannel)
{
    mem.Write<float>(netChannel + 0x2108, 69.f);
}
inline static void ChokeLoop(Memory &mem)
{
    // while (true)
    // {
    if (!settings::aimbot)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        return;
    }
    uint64_t netChannel = mem.Read<intptr_t>(baseAddress + NetChannel);

    ulong chokedCommands = mem.Read<ulong>(netChannel + 0x2028);
    if (chokedCommands > 7)
    {
        mem.Write<double>(netChannel + 0x2108, 0);
        return;
    }
    if (settings::aimbot)
    {

        float currentTime = mem.Read<float>(baseAddress + offsets::OFFSET_GlobalVars + 0x28);
        float interval_per_tick = mem.Read<float>(baseAddress + offsets::OFFSET_GlobalVars + 0x30);

        float readyTime = mem.Read<float>(localWeponPTR + m_nextReadyTime) > mem.Read<float>(localWeponPTR + m_nextPrimaryAttackTime) ? mem.Read<float>(localWeponPTR + m_nextReadyTime) : mem.Read<float>(localWeponPTR + m_nextPrimaryAttackTime);
        if (currentTime > readyTime - interval_per_tick && mem.Read<ulong>(netChannel + 0x2028) == 0)
        {
            Choke(mem, netChannel);
        }
    }
}
