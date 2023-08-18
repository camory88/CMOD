#include "memory.hpp"

#include <thread>
#include <mutex>

uint64_t Memory::GetModuleBase(const char *module_name)
{
    ModuleInfo buffer;
    return this->proc.module_by_name(CSliceRef<uint8_t>(module_name), &buffer) ? 0 : buffer.base;
}
uint64_t Memory::GetModuleSize(const char *module)
{
    ModuleInfo buffer;
    this->proc.module_by_name(CSliceRef<uint8_t>(module), &buffer);
    return buffer.size;
}
ModuleInfo Memory::GetPrimaryModule()
{
    ModuleInfo buffer;
    this->proc.primary_module(&buffer);
    return buffer;
}
Memory::Memory(const char *name, OsInstance<> *os)
{

    //printf("Searching for process \"%s\"\n", name);
    while (os->process_by_name(CSliceRef<uint8_t>(name), &this->proc))
        std::this_thread::sleep_for(std::chrono::seconds(1));

    this->proc_info = this->proc.info();

    //printf("Found process: [%d] %s\n", this->proc_info->pid, this->proc_info->name);

    ModuleInfo mod;
    proc.primary_module(&mod);

    this->base = mod.base;
    ProcessState test = proc.state();
    test.tag;

}
Memory GetMemory(const char *name, OsInstance<> *os)
{
    Memory mem = Memory(name, os);
    return mem;
}
int initMemflow(Inventory *inventory, ConnectorInstance<> &connector, OsInstance<> &os)
{
    log_init(LevelFilter::LevelFilter_Error);
    inventory = inventory_scan();

    if (!inventory)
    {
        log_error("unable to create inventory");
        return 1;
    }

    if (&connector)
    {
        if (inventory_create_connector(inventory, "kvm", "", &connector))
        {
            printf("unable to initialize connector\n");
            inventory_free(inventory);
            return 2;
        }
        //cheack ->>>>>    printf("connector initialized: %p\n", connector.container.instance.instance);
    }

    if (inventory_create_os(inventory, "win32", "", &connector, &os))
    {
        printf("Failed to create os instance\n");
        inventory_free(inventory);
        return 3;
    }
    // ConnectorInstance<> cloned = connector.clone();

    return 0;
}

