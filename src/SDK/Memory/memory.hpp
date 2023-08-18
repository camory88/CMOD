#pragma once

#include "memflow.hpp"
#include <mutex>
#include <cstddef>

class Memory
{
private:
    ProcessInstance<> proc;
    const ProcessInfo *proc_info;
    uint64_t base;

public:
    uint64_t GetModuleBase(const char *module);
    uint64_t GetModuleSize(const char *module);
    ModuleInfo GetPrimaryModule();

    uint64_t get_proc_baseaddr()
    {
        return this->base;
    }

    // needs testing i think i did this right
    bool heartbeat()
    {
        ProcessState state = this->proc.state();
        if (state.tag == ProcessState::Tag::ProcessState_Alive)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    Memory(const char *name, OsInstance<> *os);

    template <typename T>
    bool Read(uint64_t address, T &buffer)
    {
        return base && this->proc.read_raw_into(address, CSliceMut<uint8_t>(reinterpret_cast<char *>(&buffer), sizeof(T)));
    }

    template <typename T>
    bool Read(uint64_t address, T &buffer, size_t size)
    {
        return base && this->proc.read_raw_into(address, CSliceMut<uint8_t>(reinterpret_cast<char *>(&buffer), size));
    }

    template <typename T>
    T Read(uint64_t address)
    {
        T buffer;
        this->proc.read_raw_into(address, CSliceMut<uint8_t>(reinterpret_cast<char *>(&buffer), sizeof(T)));
        return buffer;
    }

    template <typename T>
    bool ReadArray(uint64_t address, T buffer[], size_t len)
    {
        return base && this->proc.read_raw_into(address, CSliceMut<uint8_t>(reinterpret_cast<char *>(buffer), sizeof(T) * len));
    }

    template <typename T>
    bool Write(uint64_t address, const T data)
    {
        return base && this->proc.write_raw(address, CSliceRef<uint8_t>(reinterpret_cast<const char *>(&data), sizeof(T)));
    }

    template <typename T>
    bool WriteArray(uint64_t address, const T data[], size_t len)
    {
        return base && this->proc.write_raw(address, CSliceRef<uint8_t>(reinterpret_cast<const char *>(data), sizeof(T) * len));
    }

    std::string ReadString(uint64_t address)
    {
        char buffer[1024];
        ReadArray<char>(address, buffer, 1024);

        std::string result;

        size_t len;
        for (len = 0; len < 1024; ++len)
            if (buffer[len] == '\0')
                break;
        result.assign(buffer, len);

        return result;
    }

    template <typename T>
    T ReadPtr(uint64_t addr, int levels)
    {
        for (int i = 1; i <= levels; i++)
        {
            addr = Read<uint64_t>(addr);
        }
        return Read<T>(addr);
    }
};

Memory GetMemory(const char *name, OsInstance<> *os);

int initMemflow(Inventory *inventory, ConnectorInstance<> &connector, OsInstance<> &os);

