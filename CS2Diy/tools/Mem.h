#pragma once
#include <Windows.h>
#include <iostream>
#include <tlhelp32.h>
#include"../address/gameAddress.h"

WINUSERAPI SHORT WINAPI GetAsyncKeyState(int vKey);

namespace mem
{
    inline char* GetModule(DWORD PID, const wchar_t* 模块名称)
    {
        HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, PID);
        if (handle == INVALID_HANDLE_VALUE)
        {
            return nullptr;
        }

        MODULEENTRY32 module_info;
        ZeroMemory(&module_info, sizeof(module_info));
        module_info.dwSize = sizeof(module_info);

        bool ret = Module32First(handle, &module_info);
        if (!ret)
        {
            return nullptr;
        }

        do
        {
            //判断模块名是否相等
            if (wcscmp(模块名称, module_info.szModule) == 0)
            {
                //如果找到了就打印模块的地址 并返回该模块的地址
                //printf("%ws %llx\n", module_info.szModule, module_info.modBaseAddr);
                CloseHandle(handle);
                return (char*)(module_info.modBaseAddr);
            }
        } while (Module32Next(handle, &module_info));

        CloseHandle(handle);
        return nullptr;
    }
    inline bool Read(char* 内存地址, void* 存放指针, int 欲读取字节)//无需进程句柄
    {
        SIZE_T 缓冲区;
        if (ReadProcessMemory(gameAddress::g_handle, 内存地址, 存放指针, 欲读取字节, &缓冲区))
            return true;
        return false;
    }
    inline bool Read(HANDLE 进程句柄, char* 内存地址, void* 存放指针, int 欲读取字节)
    {
        SIZE_T 缓冲区;
        if (ReadProcessMemory(进程句柄, 内存地址, 存放指针, 欲读取字节, &缓冲区))
            return true;
        return false;
    }
   
    inline bool Write( char* 内存地址, void* 写入数据, int 欲写入字节)
    {
        SIZE_T 缓冲区;
        if (WriteProcessMemory(gameAddress::g_handle, 内存地址, 写入数据, 欲写入字节, &缓冲区))
            return true;
        return false;
    }
}
