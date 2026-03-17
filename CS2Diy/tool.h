#pragma once
#include"cheat.h"
#include"tools/Mem.h"
#include"output/offsets.hpp"
#include"output/client_dll.hpp"
#include"tools/XorStr.h"
#include"address/gameAddress.h"
#include"output/buttons.hpp"
#include <Windows.h>
#include <stdexcept>
#include <string>
#include <TlHelp32.h>
#include <tchar.h>
#include <psapi.h>
using namespace cs2_dumper;
namespace 工具 {

	inline char* 指向摄像机服务的指针;
	inline char* fov;
	inline float 初始值 = 0.f;
	inline char* local_player;



}

namespace 工具 {
	using addr32 = unsigned int;
	using addr64 = unsigned long long;
}

namespace 工具 {
	class Process {
		private:
			unsigned short PID;
			HANDLE process_handle;
		public:
			Process(unsigned short PID) : PID(PID), process_handle(OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID)) {
				if (!process_handle)
				throw std::runtime_error("Failed to open handle");
			}

			Process(const TCHAR* proc_name) {
				PID = Process::get_pid(proc_name);
				if (!PID)
					throw std::runtime_error("Unable to find process ID through process name");
				process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
				if (!process_handle) {
					throw std::runtime_error("Failed to open process");
				}
			}

			DWORD static get_pid(const TCHAR* proc_name) {
				PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
				auto handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
				if (!handle)
				{	throw std::runtime_error("CreateToolhelp32Snapshot func error");	}
				bool ret = Process32First(handle, &pe);
				while (ret) {
					auto id = pe.th32ProcessID;
					HANDLE ph = OpenProcess(PROCESS_ALL_ACCESS, FALSE, id);
					if (ph) {
						if (_tcscmp(pe.szExeFile, proc_name) == 0) { CloseHandle(ph); return id; }
						CloseHandle(ph);
					}
					ret = Process32Next(handle, &pe);
				}
				return 0;
			}

			/*Obtain module handle through module name*/
			LONG_PTR get_module_handle(const TCHAR* module_name) const {
				HMODULE handles[1024]{};
				DWORD size;

				EnumProcessModules(process_handle, handles, sizeof(handles), &size);

				for (size_t i = 0; i < size / sizeof(handles[0]); i++) {
					TCHAR name[1024]{};
					GetModuleBaseName(process_handle, handles[i], name, sizeof(name));

					if (_tcscmp(name, module_name) == 0) { return (LONG_PTR)handles[i]; }
				}
				throw std::runtime_error("Unable to find module");
			}
			
			template< typename T, typename Ptr>
			T read(const Ptr& ptr) {
				T buffer{};
				SIZE_T t;
			ReadProcessMemory(process_handle, (void*)ptr, &buffer, sizeof(T), &t);
			if (t != sizeof(T))
				throw std::runtime_error("Memory read error");
			return buffer;
			}

			
			template< typename T, typename Ptr>
			void write(const Ptr& ptr, const T& value) {
				T v = value;
				SIZE_T write_b;
				WriteProcessMemory(process_handle, (void*)ptr, &v, sizeof(v), &write_b);
				if (write_b != sizeof(v))
					throw std::runtime_error("Memory write error");
			}

			~Process() { 
				CloseHandle(process_handle); 
			}
	};
}


void 改fov角度();
void 旋转大陀螺();
void 作弊线程1();
void 一直跳();