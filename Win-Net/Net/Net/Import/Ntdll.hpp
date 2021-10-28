#pragma once
#ifndef BUILD_LINUX
#include "ImportResolver.h"
#include <winternl.h>

//
// Macro
//

#define NtCurrentThread ((HANDLE)-2)

#define STATUS_INFO_LENGTH_MISMATCH 0xC0000004

#define PDI_MODULES                       0x01
#define PDI_BACKTRACE                     0x02
#define PDI_HEAPS                         0x04
#define PDI_HEAP_TAGS                     0x08
#define PDI_HEAP_BLOCKS                   0x10
#define PDI_LOCKS                         0x20

//
// Types
//

typedef enum _MEMORY_INFORMATION_CLASS {
	MemoryBasicInformation,
	MemoryWorkingSetList,
} MEMORY_INFORMATION_CLASS;

typedef struct _RTL_HEAP_TAG
{
	ULONG NumberOfAllocations;
	ULONG NumberOfFrees;
	ULONG BytesAllocated;
	USHORT TagIndex;
	USHORT CreatorBackTraceIndex;
	WCHAR TagName[24];
} RTL_HEAP_TAG, * PRTL_HEAP_TAG;

typedef struct _RTL_HEAP_ENTRY
{
	ULONG Size;
	USHORT Flags;
	USHORT AllocatorBackTraceIndex;
	union
	{
		struct
		{
			ULONG Settable;
			ULONG Tag;
		} s1;
		struct
		{
			ULONG CommittedSize;
			PVOID FirstBlock;
		} s2;
	} u;
} RTL_HEAP_ENTRY, * PRTL_HEAP_ENTRY;

typedef struct _RTL_HEAP_INFORMATION
{
	PVOID BaseAddress;
	ULONG Flags;
	USHORT EntryOverhead;
	USHORT CreatorBackTraceIndex;
	ULONG BytesAllocated;
	ULONG BytesCommitted;
	ULONG NumberOfTags;
	ULONG NumberOfEntries;
	ULONG NumberOfPseudoTags;
	ULONG PseudoTagGranularity;
	ULONG Reserved[5];
	PRTL_HEAP_TAG Tags;
	PRTL_HEAP_ENTRY Entries;
} RTL_HEAP_INFORMATION, * PRTL_HEAP_INFORMATION;

typedef struct _RTL_PROCESS_HEAPS
{
	ULONG NumberOfHeaps;
	RTL_HEAP_INFORMATION Heaps[1];
} RTL_PROCESS_HEAPS, * PRTL_PROCESS_HEAPS;

typedef struct _DEBUG_BUFFER {
	HANDLE SectionHandle;
	PVOID SectionBase;
	PVOID RemoteSectionBase;
	WPARAM SectionBaseDelta;
	HANDLE EventPairHandle;
	HANDLE RemoteEventPairHandle;
	HANDLE RemoteProcessId;
	HANDLE RemoteThreadHandle;
	ULONG InfoClassMask;
	SIZE_T SizeOfInfo;
	SIZE_T AllocatedSize;
	SIZE_T SectionSize;
	PVOID ModuleInformation;
	PVOID BackTraceInformation;
	PVOID HeapInformation;
	PVOID LockInformation;
	PVOID SpecificHeap;
	HANDLE RemoteProcessHandle;
	PVOID VerifierOptions;
	PVOID ProcessHeap;
	HANDLE CriticalSectionHandle;
	HANDLE CriticalSectionOwnerThread;
	PVOID Reserved[4];
} DEBUG_BUFFER, * PDEBUG_BUFFER;

typedef struct _DEBUG_HEAP_INFORMATION {
	PVOID Base;
	DWORD Flags;
	USHORT Granularity;
	USHORT CreatorBackTraceIndex;
	SIZE_T Allocated;
	SIZE_T Committed;
	DWORD TagCount;
	DWORD BlockCount;
	DWORD PseudoTagCount;
	DWORD PseudoTagGranularity;
	DWORD Reserved[5];
	PVOID Tags;
	PVOID Blocks;
} DEBUG_HEAP_INFORMATION, * PDEBUG_HEAP_INFORMATION;

/* To prevent MSVC warning 4820 when compiling for x64 */
#ifdef _WIN64
#   define EXPLICIT_PADDING(n, size) BYTE padding_##n[size]
#else
#   define EXPLICIT_PADDING(n, size)
#endif

typedef struct _RTL_DEBUG_INFORMATION
{
	HANDLE      SectionHandleClient;
	PVOID       ViewBaseClient;
	PVOID       ViewBaseTarget;
	ULONG_PTR   ViewBaseDelta;
	HANDLE      EventPairClient;
	HANDLE      EventPairTarget;
	HANDLE      TargetProcessId;
	HANDLE      TargetThreadHandle;
	ULONG       Flags;
	EXPLICIT_PADDING(0, 4);
	SIZE_T      OffsetFree;
	SIZE_T      CommitSize;
	SIZE_T      ViewSize;
	union
	{
		PVOID   Modules;
		PVOID   ModulesEx;
	};
	PVOID       BackTraces;
	struct _RTL_PROCESS_HEAPS* Heaps;
	PVOID       Locks;
	PVOID       SpecificHeap;
	HANDLE      TargetProcessHandle;
	PVOID       VerifierOptions;
	PVOID       ProcessHeap;
	HANDLE      CriticalSectionHandle;
	HANDLE      CriticalSectionOwnerThread;
	PVOID       Reserved[4];
} RTL_DEBUG_INFORMATION, * PRTL_DEBUG_INFORMATION;

typedef struct _SYSTEM_KERNEL_DEBUGGER_INFORMATION {
	BOOLEAN DebuggerEnabled;
	BOOLEAN DebuggerNotPresent;
} SYSTEM_KERNEL_DEBUGGER_INFORMATION, * PSYSTEM_KERNEL_DEBUGGER_INFORMATION;

typedef struct _OBJECT_TYPE_INFORMATION
{
	UNICODE_STRING TypeName;
	ULONG TotalNumberOfHandles;
	ULONG TotalNumberOfObjects;
} OBJECT_TYPE_INFORMATION, * POBJECT_TYPE_INFORMATION;

typedef struct _OBJECT_ALL_INFORMATION
{
	ULONG NumberOfObjects;
	OBJECT_TYPE_INFORMATION ObjectTypeInformation[1];
} OBJECT_ALL_INFORMATION, * POBJECT_ALL_INFORMATION;

typedef union _PSAPI_WORKING_SET_BLOCK {
	ULONG Flags;
	struct {
		ULONG Protection : 5;
		ULONG ShareCount : 3;
		ULONG Shared : 1;
		ULONG Reserved : 3;
		ULONG VirtualPage : 20;
	};
} PSAPI_WORKING_SET_BLOCK, * PPSAPI_WORKING_SET_BLOCK;

typedef struct _MEMORY_WORKING_SET_LIST
{
	ULONG NumberOfPages;
	PSAPI_WORKING_SET_BLOCK WorkingSetList[1];
} MEMORY_WORKING_SET_LIST, * PMEMORY_WORKING_SET_LIST;

typedef struct _PROCESS_LDT_INFORMATION
{
	ULONG Start;
	ULONG Length;
	LDT_ENTRY LdtEntries[1];
} PROCESS_LDT_INFORMATION, * PPROCESS_LDT_INFORMATION;

typedef struct _RTL_UNKNOWN_FLS_DATA {
	PVOID unk1;
	PVOID unk2;
	PVOID unk3;
	PVOID Argument;
} RTL_UNKNOWN_FLS_DATA, * PRTL_UNKNOWN_FLS_DATA;

typedef enum _HARDERROR_RESPONSE_OPTION
{
	OptionAbortRetryIgnore,
	OptionOk,
	OptionOkCancel,
	OptionRetryCancel,
	OptionYesNo,
	OptionYesNoCancel,
	OptionShutdownSystem,
	OptionOkNoWait,
	OptionCancelTryContinue
}HARDERROR_RESPONSE_OPTION;

typedef enum _HARDERROR_RESPONSE
{
	ResponseReturnToCaller,
	ResponseNotHandled,
	ResponseAbort,
	ResponseCancel,
	ResponseIgnore,
	ResponseNo,
	ResponseOk,
	ResponseRetry,
	ResponseYes,
	ResponseTryAgain,
	ResponseContinue
} HARDERROR_RESPONSE, * PHARDERROR_RESPONSE;

// heap masks
#define PDI_MODULES 0x01
#define PDI_HEAPS 0x04
#define PDI_HEAP_TAGS 0x08
#define PDI_HEAP_BLOCKS 0x10
#define PDI_HEAP_ENTRIES_EX 0x200

#ifdef NET_DISABLE_IMPORT_NTDLL
#define Ntdll
#else
#define IMPORT_NAME Ntdll

RESOLVE_IMPORT_BEGIN;
DEFINE_IMPORT(NTSTATUS, NtCreateThreadEx, PHANDLE hThread, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, HANDLE ProcessHandle, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, ULONG Flags, ULONG_PTR StackZeroBits, SIZE_T SizeOfStackCommit, SIZE_T SizeOfStackReserve, LPVOID lpBytesBuffer);
MAKE_IMPORT(hThread, DesiredAccess, ObjectAttributes, ProcessHandle, lpStartAddress, lpParameter, Flags, StackZeroBits, SizeOfStackCommit, SizeOfStackReserve, lpBytesBuffer);

DEFINE_IMPORT(NTSTATUS, NtQueryInformationProcess, HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
MAKE_IMPORT(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);

DEFINE_IMPORT(PDEBUG_BUFFER, RtlCreateQueryDebugBuffer, ULONG  Size, BOOLEAN EventPair);
MAKE_IMPORT(Size, EventPair);

DEFINE_IMPORT(NTSTATUS, RtlQueryProcessDebugInformation, DWORD pid, DWORD mask, PDEBUG_BUFFER db);
MAKE_IMPORT(pid, mask, db);

DEFINE_IMPORT(NTSTATUS, RtlAdjustPrivilege, ULONG Privilege, BOOLEAN  Enable, BOOLEAN CurrentThread, PBOOLEAN Enabled);
MAKE_IMPORT(Privilege, Enable, CurrentThread, Enabled);

DEFINE_IMPORT(NTSTATUS, NtRaiseHardError, NTSTATUS ErrorStatus, ULONG NumberOfParameters, ULONG UnicodeStringParameterMask, PULONG_PTR Parameters, ULONG ResponseOption, PULONG Response);
MAKE_IMPORT(ErrorStatus, NumberOfParameters, UnicodeStringParameterMask, Parameters, ResponseOption, Response);

DEFINE_IMPORT(NTSTATUS, RtlQueryProcessHeapInformation, PRTL_DEBUG_INFORMATION buffer);
MAKE_IMPORT(buffer);

DEFINE_IMPORT(NTSTATUS, NtQuerySystemInformation, SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
MAKE_IMPORT(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);

DEFINE_IMPORT(NTSTATUS, NtQueryObject, HANDLE Handle, OBJECT_INFORMATION_CLASS ObjectInformationClass, PVOID ObjectInformation, ULONG ObjectInformationLength, PULONG ReturnLength);
MAKE_IMPORT(Handle, ObjectInformationClass, ObjectInformation, ObjectInformationLength, ReturnLength);

DEFINE_IMPORT(NTSTATUS, NtCreateDebugObject, PHANDLE DebugObjectHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, BOOLEAN KillProcessOnExit);
MAKE_IMPORT(DebugObjectHandle, DesiredAccess, ObjectAttributes, KillProcessOnExit);

DEFINE_IMPORT(NTSTATUS, NtClose, HANDLE Handle);
MAKE_IMPORT(Handle);

DEFINE_IMPORT(NTSTATUS, NtQueryInformationThread, HANDLE ThreadHandle, THREADINFOCLASS ThreadInformationClass, PVOID ThreadInformation, ULONG ThreadInformationLength, PULONG ReturnLength);
MAKE_IMPORT(ThreadHandle, ThreadInformationClass, ThreadInformation, ThreadInformationLength, ReturnLength);

DEFINE_IMPORT(NTSTATUS, NtSetInformationThread, HANDLE ThreadHandle, THREAD_INFORMATION_CLASS ThreadInformationClass, PVOID ThreadInformation, ULONG ThreadInformationLength);
MAKE_IMPORT(ThreadHandle, ThreadInformationClass, ThreadInformation, ThreadInformationLength);

DEFINE_IMPORT(NTSTATUS, RtlProcessFlsData, PRTL_UNKNOWN_FLS_DATA Buffer);
MAKE_IMPORT(Buffer);

DEFINE_IMPORT(NTSTATUS, RtlDestroyQueryDebugBuffer, PDEBUG_BUFFER Buffer);
MAKE_IMPORT(Buffer);

DEFINE_IMPORT(NTSTATUS, NtSetDebugFilterState, DWORD d1, DWORD d2, bool b);
MAKE_IMPORT(d1, d2, b);
RESOLVE_IMPORT_END;
#endif
#endif
