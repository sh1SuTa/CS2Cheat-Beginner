#include "memory.h"
#include <ntddk.h>
#include <wdm.h>

PVOID get_system_module_base(LPCSTR module_name)
{
	ULONG bytes = 0;
	//第一次调用 获取所需的缓冲区大小，NULL表不直接获取模块信息
	NTSTATUS status = ZwQuerySystemInformation(SystemModuleInfomation, NULL, bytes, &bytes);
	if (!bytes)
	{
		return NULL;
	}
	
	// 旧的内核内存分配函数 ExAllocatePoolWithTag 已经弃用
	//PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, bytes, 0x4E554C4C);
	// 表示分配的内存不会被分页确保内存一直驻留在物理内存中，分配内存所需的字节数为之前获取模块信息的字节大小，分配的内存标签为NULL
	PRTL_PROCESS_MODULES modules =(PRTL_PROCESS_MODULES) ExAllocatePool2(POOL_FLAG_NON_PAGED, bytes, 0x4E554C4C);

	status = ZwQuerySystemInformation(SystemModuleInfomation, modules, bytes, &bytes);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[-] ZwQuerySystemInformation返回的status不为Success \n");
		ExFreePoolWithTag(modules, 0x4E554C4C);
		return NULL;
	}
	PRTL_PROCESS_MODULE_INFORMATION module = modules->Modules;
	PVOID module_base = 0, module_size = 0;




	for (ULONG i = 0; i < modules->NumberOfModules; i++)
	{
		//strstr检查第一个字符串是否包含第二个字符串的子串
		//strcmp 返回 0表示 相等
		if (strcmp((char*)module[i].FullPathName, module_name) == 0) {
			DbgPrint("[+] 模块名：%s\n", module[i].FullPathName);
			module_base = module[i].ImageBase;
			module_size = (PVOID)module[i].ImageSize;
			break;
		}
	}
	if (modules)
	{

		ExFreePoolWithTag(modules, NULL);
	}if (module_base <= NULL)
	{
		DbgPrint("[-] 模块module_base<=NULL \n");
		return NULL;
	}
	return module_base;
}
//获取导出函数地址
PVOID get_system_module_export(const char* module_name, LPCSTR routine_name)
{

	PVOID lpModule = get_system_module_base(module_name);
	if (!lpModule)
	{
		return NULL;
	}
	return RtlFindExportedRoutineByName(lpModule, routine_name);
}
PVOID get_system_routine_address(LPCWSTR routine_name) {
	UNICODE_STRING name;
	RtlInitUnicodeString(&name, routine_name);
	return MmGetSystemRoutineAddress(&name);
}
PVOID get_system_module_export(LPCWSTR module_name, LPCSTR routine_name) {
	PLIST_ENTRY modules_list = reinterpret_cast<PLIST_ENTRY>(get_system_routine_address(L"PsLoadedModuleList"));

	if (!modules_list) {
		DbgPrint("[-] PsLoadedModuleList获取失败\n");
		return NULL;
	}
	for (PLIST_ENTRY link = modules_list; link != modules_list->Blink; link = link->Flink)
	{
		LDR_DATA_TABLE_ENTRY* entry = CONTAINING_RECORD(link, LDR_DATA_TABLE_ENTRY, InLoadOrderModuleList);
		UNICODE_STRING name;
		RtlUnicodeStringInit(&name, module_name);
		if (RtlEqualUnicodeString(&name, &entry->BaseDllName, TRUE))
		{
			return (entry->DllBase) ? RtlFindExportedRoutineByName(entry->DllBase, routine_name) : NULL;
		}
	}
	return NULL;

}
bool write_memory(void* address, void* buffer, size_t size)
{
	if (RtlCopyMemory(address, buffer, size))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool write_to_read_only_memory(void* 目标位置, void* 写入的数据, ULONG 写入大小)
{
	//分配内存描述符（MDL）
	PMDL Mdl = IoAllocateMdl(目标位置, 写入大小, FALSE, FALSE, NULL);
	if (!Mdl)
	{
		return false;
	}// 锁定内存页面
	MmProbeAndLockPages(Mdl, KernelMode, IoReadAccess);
	//将锁定的内存映射到系统空间
	PVOID Mapping = MmMapLockedPagesSpecifyCache(Mdl, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority);
	// 设置内存权限为可读写
	MmProtectMdlSystemAddress(Mdl, PAGE_READWRITE);
	write_memory(Mapping, 写入的数据, 写入大小);
	//释放
	MmUnmapLockedPages(Mapping, Mdl);
	MmUnlockPages(Mdl);
	IoFreeMdl(Mdl);
	return true;
}

ULONG64 get_module_base_x64(PEPROCESS proc, UNICODE_STRING module_name)
{
	PPEB pPeb = PsGetProcessPeb(proc);
	if (!pPeb)
	{
		return NULL;
	}

	KAPC_STATE state;
	// 将当前线程的上下文切换到目标进程上下文
	KeStackAttachProcess(proc, &state);

	PPEB_LDR_DATA pLdr = (PPEB_LDR_DATA)pPeb->Ldr;
	if (!pLdr)
	{ // 如果无法获取到加载数据结构，解除进程的堆栈附加
		KeUnstackDetachProcess(&state);
		return NULL;
	}

	// 遍历加载模块列表
	for (PLIST_ENTRY list = (PLIST_ENTRY)pLdr->ModuleListLoadOrder.Flink;
		list != &pLdr->ModuleListLoadOrder;
		list = (PLIST_ENTRY)list->Flink)
	{
		PLDR_DATA_TABLE_ENTRY pEntry = CONTAINING_RECORD(list, LDR_DATA_TABLE_ENTRY, InLoadOrderModuleList);

		// 比较模块名称是否匹配
		if (RtlCompareUnicodeString(&pEntry->BaseDllName, &module_name, TRUE) == 0)
		{
			ULONG64 baseAddress = (ULONG64)pEntry->DllBase;

			// 获取物理地址
			//PHYSICAL_ADDRESS physicalAddress = MmGetPhysicalAddress((PVOID)baseAddress);
			//ULONG64 physicalBaseAddress = (ULONG64)physicalAddress.QuadPart;

			// 恢复当前线程上下文
			KeUnstackDetachProcess(&state);
			return baseAddress;  // 返回物理地址
		}
	}

	KeUnstackDetachProcess(&state);
	return NULL;
}

bool read_kernel_memory(HANDLE pid, uintptr_t address, void* buffer, SIZE_T size)
{
	SIZE_T bytes = 0;
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS process;
	PsLookupProcessByProcessId((HANDLE)pid, &process);
	status = MmCopyVirtualMemory(process, (void*)address, (PEPROCESS)PsGetCurrentProcess(), buffer, size, KernelMode, &bytes);
	if (!NT_SUCCESS(status))
	{
		return false;
	}
	else
	{
		return true;
	}

}

bool write_kernel_memory(HANDLE pid, uintptr_t address, void* buffer, SIZE_T size)
{
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS process;
	PsLookupProcessByProcessId((HANDLE)pid, &process);
	KAPC_STATE state;
	KeStackAttachProcess(process, &state);
	MEMORY_BASIC_INFORMATION info;
	status = ZwQueryVirtualMemory(ZwCurrentProcess(), (PVOID)address, MemoryBasicInformation, &info, sizeof(info), NULL);
	if (!NT_SUCCESS(status))
	{
		KeUnstackDetachProcess(&state);
		return false;
	}
	if (((uintptr_t)info.BaseAddress + info.RegionSize) < (address + size))
	{
		KeUnstackDetachProcess(&state); return false;
	}
	if (!(info.State & MEM_COMMIT) || (info.Protect & (PAGE_GUARD | PAGE_NOACCESS)))
	{
		KeUnstackDetachProcess(&state); return false;
	}if ((info.Protect & PAGE_EXECUTE_READWRITE) || (info.Protect & PAGE_EXECUTE_WRITECOPY)
		|| (info.Protect & PAGE_READWRITE) || (info.Protect & PAGE_WRITECOPY))
	{
		RtlCopyMemory((void*)address, buffer, size);
	}
	KeUnstackDetachProcess(&state); return true;
}
