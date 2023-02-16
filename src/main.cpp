#include "module.hpp"
#include <thread>

Inventory *inventory;
ConnectorInstance<> connector;
OsInstance<> os;



int main()
{
    initMemflow(inventory, connector, os);

    Memory mem = GetMemory("r5apex.exe",&os);//ApexLegends

    baseAddress = mem.get_proc_baseaddr();
    printf("Apex base: %llx\n",baseAddress);
    
    while (true)
    {
        //localPLayer
        LocalPLayer = mem.Read<uint64_t>(baseAddress + OFFSET_LOCAL_ENT);
        int LocalTeam = mem.Read<uint64_t>(LocalPLayer + OFFSET_TEAM);

        //gloabls
        CGlobalVars GlobalVars = mem.Read<CGlobalVars>(baseAddress + OFFSET_GlobalVars);
        mem.Read<CGlobalVars>(baseAddress + OFFSET_GlobalVars, GlobalVars );
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        target = NULL;
        for(int i = 0; i < indexNum; ++i)
        {
            uint64_t player = mem.Read<uint64_t>(baseAddress + OFFSET_ENTITYLIST+ (i << 5));
            if(player == 0)continue;
            if(player == LocalPLayer)continue;

            int team = mem.Read<uint64_t>(player + OFFSET_TEAM);
            if(team == LocalTeam)continue;

            if(isPlayer(mem,player) == true) 
            {
                if(isAlive(mem,player)== true)
                {
                    BestTarg(mem,player,LocalTeam);

                    if(isGlowing(mem,player)== false)
                    {
                        if(player != target)
                        {
                            start_glowing(mem,player,glowColor);
                        }
                    }
                }
            }
        }
        
        QAngle Angles = CalculateBestBoneAim(mem, target,dist,smoothnes, fov);
		if (Angles.x != 0 && Angles.y != 0)
        {
            if(mem.Read<int>(baseAddress+in_zoom+0x8)==5)
            {
                mem.Write<QAngle>(LocalPLayer+OFFSET_VIEWANGLES, Angles);
            }
        }
        if(target == NULL)
            recoilControl(mem, recoilcontrol);

        //thirdPerson(mem);
        //charge_rifle_hack(mem);

    }

    return 0;
}

