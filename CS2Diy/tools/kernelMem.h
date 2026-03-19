#pragma once
#include<iostream>
#include<Windows.h>

#include<memory>
#include<string_view>
#include<cstdint>
#include<vector>
#include <TlHelp32.h>
#include"XorStr.h"
#include"gameAddress.h"
namespace mem
{
	typedef struct _NULL_MEMORY {
		void* buffer_address;
		UINT_PTR address;
		ULONGLONG size;
		ULONG pid;
		BOOLEAN read;
		BOOLEAN write;
		BOOLEAN req_base;
		BOOLEAN draw_box;
		int r, g, b, x, y, w, h, t;
		void* output;
		const char* module_name;
		ULONG64 base_address;
	}NULL_MEMORY;

	template<typename ...Arg>
	void call_hook(const Arg ... args) {
		HMODULE hModule = LoadLibrary(L"win32u.dll");


		if (hModule == NULL)
		{
			printf("加载 win32u.dll 失败\n");
			return ;
		}
		//获取该库中名为 NtQueryCompositionSurfaceStatistics 的函数的地址
		void* hooked_func = GetProcAddress(hModule, "NtDxgkGetTrackedWorkloadStatistics");
		if (hooked_func == NULL) {
			printf("获取win32u.dll的NtDxgkGetTrackedWorkloadStatistics失败\n");
		}
		//将 hooked_func 转换为具有特定签名的函数指针,规定了函数的返回类型、参数类型和调用约定。
		auto func = static_cast<uint64_t(_stdcall*)(Arg...)>(hooked_func);
		if (func == NULL)
		{
			printf(XorStr("函数指针获取失败\n"));
		}
		//std::cout << "函数指针:" << func << std::endl;
		func(args ...);
	}


	inline bool Write(UINT_PTR write_address, UINT_PTR source_address, SIZE_T write_size) {
		NULL_MEMORY instructions;
		instructions.address = write_address;
		instructions.pid = gameAddress::g_pid;
		instructions.write = true;
		instructions.read = false;
		instructions.draw_box = false;
		instructions.req_base = false;
		instructions.buffer_address = (void*)source_address;
		instructions.size = write_size;
		call_hook(&instructions);
		return true;
	}

	template<typename S>
	bool Write(UINT_PTR write_address, const S& value) {
		return Write(write_address, (UINT_PTR)value, sizeof(S));
	}


	



	struct HandleDisposer {
		using pointer = HANDLE;
		void operator()(HANDLE handle)const {
			if (handle != NULL && handle != INVALID_HANDLE_VALUE) {
				CloseHandle(handle);
			}
		}
	};
	using unique_handle = std::unique_ptr<HANDLE, HandleDisposer>;

	//获取进程id非内核
	inline std::uint32_t get_process_id(std::wstring process_name) {
		PROCESSENTRY32 processentry;
		const unique_handle snapshot_handle(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL));

		if (snapshot_handle.get() == INVALID_HANDLE_VALUE)
		{
			return NULL;
		}
		processentry.dwSize = sizeof(MODULEENTRY32);
		while (Process32Next(snapshot_handle.get(), &processentry) == TRUE) {

			if (process_name.compare(processentry.szExeFile) == NULL)
			{
				return processentry.th32ProcessID;
			}
		}
		return NULL;
	}

	static ULONG64 get_module_base_address(const char* module_name, ULONG process_id) {
		NULL_MEMORY instructions = { 0 };
		instructions.pid = process_id;

		instructions.req_base = true;
		instructions.read = false;
		instructions.write = false;
		instructions.module_name = module_name;
		call_hook(&instructions);
		ULONG64 base = NULL;
		base = instructions.base_address;
		return base;
	}

	template<class T>
	T Read(UINT_PTR read_address) {
		T repones{};
		NULL_MEMORY instructions;
		instructions.pid = gameAddress::g_pid;
		instructions.size = sizeof(T);
		instructions.address = read_address;
		instructions.read = true;
		instructions.write = false;
		instructions.draw_box = false;
		instructions.req_base = false;
		instructions.output = &repones;
		call_hook(&instructions);
		return repones;

	}

	template <typename F>
	void Read(UINT_PTR read_address, F(&response)[4][4]) {
		// 初始化读取指令
		NULL_MEMORY instructions;
		instructions.pid = gameAddress::g_pid;  // 进程ID
		instructions.size = sizeof(F) * 4 * 4;
		instructions.address = read_address;  // 要读取的内存地址
		instructions.read = true;  // 设置为读取操作
		instructions.write = false;  // 不进行写入
		instructions.draw_box = false;  // 这个参数可以根据需要调整
		instructions.req_base = false;  // 这个参数可以根据需要调整
		instructions.output = response;  // 设置读取结果的输出地址

		// 调用钩子函数读取内存
		call_hook(&instructions);
	}

}

//bool draw_box(int x, int y, int w, int h, int t, int r, int g, int b) {
//	NULL_MEMORY instructions;
//	instructions.pid = process::process_id;
//	instructions.write = false;
//	instructions.read = false;
//	instructions.draw_box = true;
//	instructions.req_base = false;
//	instructions.x = x;
//	instructions.y = y;
//	instructions.w = w;
//	instructions.h = h;
//	instructions.t = t;
//
//	instructions.r = r;
//	instructions.g = g;
//	instructions.b = b;
//
//	call_hook(&instructions);
//	return true;
//}