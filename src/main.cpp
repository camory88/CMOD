#include "window.hpp"
#include <thread>

Inventory *inventory;
ConnectorInstance<> connector;
OsInstance<> os;

int main()
{
    initMemflow(inventory, connector, os);

    Memory mem = GetMemory("r5apex.exe", &os); // ApexLegends

    baseAddress = mem.get_proc_baseaddr();
    printf("Apex base: %llx\n", baseAddress);

    if (RunWindow(mem ) < 0)
        return printf("Cheat crashed!\n");

    return printf("Apex not open or crashed!\n");
}