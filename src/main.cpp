#include "Modules.h"
#include <thread>

Inventory *inventory;
ConnectorInstance<> connector;
OsInstance<> os;


bool SUS()
{
    if (Window::RunWindow() > 0)
    {
    }
    else
        return false;
}


int main()
{
    initMemflow(inventory, connector, os);

    Memory mem = GetMemory("r5apex.exe", &os); // ApexLegends

    baseAddress = mem.get_proc_baseaddr();
    printf("Apex base: %llx\n", baseAddress);

    std::thread ui;
    ui = std::thread(SUS);
    ui.detach();

    
    while (mem.heartbeat())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        CGlobalVars GlobalVars = mem.Read<CGlobalVars>(baseAddress + offsets::OFFSET_GlobalVars);
        mem.Read<CGlobalVars>(baseAddress + offsets::OFFSET_GlobalVars, GlobalVars);
        localPTR = mem.Read<uint64_t>(baseAddress + offsets::OFFSET_LOCAL_ENT);
        localWeponPTR = WeaponXEntity(getWeapon(mem)).ptr;

        //printf("------------------------\n");
        entity_loop(mem);
        localPlayer_function(mem);
        
        //ChokeLoop(mem);
        //SilentAim(mem);
        if (UI == false)
        {
            return printf("UI crashed!\n");
        }
    }
    return printf("Apex not open or crashed!\n");
}