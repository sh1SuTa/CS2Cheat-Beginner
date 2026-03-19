
#pragma once
#include"definitions.h"
//通信用的结构体
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
PVOID get_system_module_base(LPCSTR module_name);
PVOID get_system_module_export(const char* module_name, LPCSTR routine_name);
PVOID get_system_module_export(LPCWSTR module_name, LPCSTR routine_name);
bool write_memory(void* address, void* buffer, size_t size);
bool write_to_read_only_memory(void* 目标位置, void* 写入的数据, size_t 写入大小);

ULONG64 get_module_base_x64(PEPROCESS proc, UNICODE_STRING module_name);

bool read_kernel_memory(HANDLE pid, uintptr_t address, void* buffer, SIZE_T size);
bool write_kernel_memory(HANDLE pid, uintptr_t address, void* buffer, SIZE_T size);