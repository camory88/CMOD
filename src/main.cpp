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

// class NET_SignonState
//{
// public:
//	void* VTablePtr; //0x0000
//	int32_t m_nGroup; //0x0008
//	bool m_bReliable; //0x000C
//	char unk_00d[3]; //0x000D
//	CNetChan* m_NetChannel; //0x0010
//	INetMessageHandler* m_pMessageHandler; //0x0018
//	int32_t State; //0x20
//	int32_t Count; //0x24
//	char unk_0028[168]; //0x0028
// }; //Size: 0x00D0

// 0x256ED4CBD7B //cl_player.gnut.client
int start = 0;

void Choke(Memory &mem, int netChannel)
{
    mem.Write<double>(netChannel + 0x2108, std::numeric_limits<float>::max());
}

void ChokeLoop(Memory &mem)
{
    while (true)
    {
        // if (!MenuSettings.bUseSilentAim)
        //{
        //
        //     continue;
        // }
        int netChannel = mem.Read<int>(baseAddress + ModuleOffsets::NetChannel);

        ulong chokedCommands = mem.Read<ulong>(netChannel + 0x2028);
        if (chokedCommands > 7)
        {
            mem.Write<double>(0, netChannel + 0x2108);
            continue;
        }
        if (settings::aimbot)
        {
            float currentTime = GlobalVars.curtime;
            float interval_per_tick = GlobalVars.interval_per_tick;
            uint64_t weapon = WeaponXEntity(getWeapon(mem)).ptr;
            // float readyTime = weapon.m_nextReadyTime > weapon.m_nextPrimaryAttackTime ? weapon.m_nextReadyTime : weapon.m_nextPrimaryAttackTime;
            if (true)
            {
                Choke(mem, netChannel);
            }
        }
    }
}
void update(Memory &mem)
{
    uint64_t netChannel = mem.Read<int>(baseAddress + ModuleOffsets::NetChannel);
    uint64_t chokedCommands = mem.Read<ulong>(netChannel + 0x2028);
    printf("CHOKE comands num = :%i\n", chokedCommands);
    if (chokedCommands <= 0)
    {
        start = start + 1;
        printf("Chocked! count:%i\n", start);
        // choking = false;
        return;
    }
    else
    {
        printf("could work?????????????????????????????????\n");
    }
}
bool silly(Memory &mem)
{
    ChokeLoop(mem);
    return true;
}
char pat[35];

bool isMatchSTR(const std::string &str, const std::string &pattern)
{
    size_t n = 0;
    while (n < str.length() && (str[n] == pattern[n] || pattern[n] == '?'))
    {
        if (pattern[n] == '?')
        {
            // If the current character in pattern is '?', treat it as a wildcard
            n++;
        }
        else
        {
            n++;
        }
    }
    return n == pattern.length(); // Return true if all characters in pattern were matched
}
uint64_t addr = 0x000000;
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
        
        entity_loop(mem);
        localPlayer_function(mem);
    }
    printf("Apex not open or crashed!\n");
    return 0;
}