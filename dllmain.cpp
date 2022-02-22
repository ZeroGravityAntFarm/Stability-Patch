// dllmain.cpp : Defines the entry point for the DLL application.
#include "Precomp.hpp"


/////////////////////////////////////////////////////////////////////////////////////////
/*=======================================================================================
									Config
=======================================================================================*/
/////////////////////////////////////////////////////////////////////////////////////////
bool debugFlag = false;


bool CanAccessFolder(LPCTSTR folderName, DWORD genericAccessRights)
{
	bool bRet = false;
	DWORD length = 0;
	if (!::GetFileSecurity(folderName, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION
		| DACL_SECURITY_INFORMATION, NULL, NULL, &length) &&
		ERROR_INSUFFICIENT_BUFFER == ::GetLastError()) {
		PSECURITY_DESCRIPTOR security = static_cast<PSECURITY_DESCRIPTOR>(::malloc(length));
		if (security && ::GetFileSecurity(folderName, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION
			| DACL_SECURITY_INFORMATION, security, length, &length)) {
			HANDLE hToken = NULL;
			if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_IMPERSONATE | TOKEN_QUERY |
				TOKEN_DUPLICATE | STANDARD_RIGHTS_READ, &hToken)) {
				HANDLE hImpersonatedToken = NULL;
				if (::DuplicateToken(hToken, SecurityImpersonation, &hImpersonatedToken)) {
					GENERIC_MAPPING mapping = { 0xFFFFFFFF };
					PRIVILEGE_SET privileges = { 0 };
					DWORD grantedAccess = 0, privilegesLength = sizeof(privileges);
					BOOL result = FALSE;

					mapping.GenericRead = FILE_GENERIC_READ;
					mapping.GenericWrite = FILE_GENERIC_WRITE;
					mapping.GenericExecute = FILE_GENERIC_EXECUTE;
					mapping.GenericAll = FILE_ALL_ACCESS;

					::MapGenericMask(&genericAccessRights, &mapping);
					if (::AccessCheck(security, hImpersonatedToken, genericAccessRights,
						&mapping, &privileges, &privilegesLength, &grantedAccess, &result)) {
						bRet = (result == TRUE);
					}
					::CloseHandle(hImpersonatedToken);
				}
				::CloseHandle(hToken);
			}
			::free(security);
		}
	}

	return bRet;
}

/////////////////////////////////////////////////////////////////////////////////////////
/*=======================================================================================
									MOPP Fix
=======================================================================================*/
/////////////////////////////////////////////////////////////////////////////////////////

int __stdcall fixMoppAddress(int currentAddress) {
	if (((currentAddress & 0x00010000) >> 16) == 1) {
		currentAddress = currentAddress & 0xFF00FFFF;
		currentAddress = currentAddress + 0x04000000;
	}
	return currentAddress;
}

void __declspec(naked) loc_D0E9E3_hook()
{
	__asm
	{
		mov     esi, [esi + 0x2C]
		add     esi, ecx
		push	esi
		call	fixMoppAddress
		mov     esi, eax
		//pop		eax
		mov     eax, [ebp + 0x10]		// a4 
		push	0xD0E9EB
		ret
	}
}

void __declspec(naked) loc_D0D919_hook()
{
	__asm
	{
		mov     ecx, [ebp + 0x8]
		mov     esi, [ecx + 0x18]
		mov     ecx, [ebx + 0x64]
		mov     edx, [ecx]
		mov     edx, [edx + 0x14]
		add     esi, eax

		push	ecx
		push    ebx
		push	edx

		push	esi
		call	fixMoppAddress
		mov     esi, eax
		//pop		eax

		pop		edx
		pop     ebx
		pop     ecx

		push	0xD0D929
		ret
	}
}

void __declspec(naked) loc_D0F505_hook() {

	__asm {
		mov     esi, [esi + 0x1C]
		mov     ebp, [ebp + 0x0]
		add     esi, eax
		push	esi
		call	fixMoppAddress
		mov     esi, eax
		//pop		eax
		push	0xD0F50D
		ret
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
/*=======================================================================================
									Contrails Fix
=======================================================================================*/
/////////////////////////////////////////////////////////////////////////////////////////

__declspec(naked) void ContrailFixHook()
{
	__asm
	{
		add ecx, [0x00A58061]
		cmp ecx, -1
		jg render
		push 0xA580BE
		retn
		render :
		push 0xA58067
			retn
	}
}


void UnprotectMemory()
{
	// Enable write to all executable memory
	size_t Offset, Total;
	Offset = Total = 0;
	MEMORY_BASIC_INFORMATION MemInfo;

	//printf("\nUnprotecting memory...");
	while (VirtualQuery((uint8_t*)GetBasePointer() + Offset, &MemInfo, sizeof(MEMORY_BASIC_INFORMATION)))
	{
		Offset += MemInfo.RegionSize;
		if (MemInfo.Protect == PAGE_EXECUTE_READ)
		{
			//printf("%0X\n", (size_t)((uint8_t*)GetBasePointer() + Offset));
			Total += MemInfo.RegionSize;
			VirtualProtect(MemInfo.BaseAddress, MemInfo.RegionSize, PAGE_EXECUTE_READWRITE, &MemInfo.Protect);
		}
	}
	//printf("\nDone! Unprotected %u bytes of memory\n", Total);
}
/////////////////////////////////////////////////////////////////////////////////////////
/*=======================================================================================
Place Main Function Call inside the Main() function:
=======================================================================================*/
/////////////////////////////////////////////////////////////////////////////////////////
void Main()
{

}

BOOL InitInstance(HINSTANCE hModule)
{
	//Disable Windows DPI scaling
	SetProcessDPIAware();

	//Make sure our working directory is in the ED root folder
	char appPath[MAX_PATH];
	GetModuleFileName(hModule, appPath, MAX_PATH);
	std::string FullPath = std::string(appPath);
	std::string Path = FullPath.substr(0, FullPath.find_last_of("\\"));
	//SetCurrentDirectory(Path.c_str());

	//Check for read/write priveledges in the current directory
	if (!CanAccessFolder(".", GENERIC_READ | GENERIC_WRITE))
	{
		MessageBoxA(NULL, "Invalid permissions for the current directory.", "Invalid Directory Permissions", MB_OK | MB_ICONERROR);
		return true;
	}

	DisableThreadLibraryCalls(hModule);
	UnprotectMemory();

	if (debugFlag)
	{
		AllocConsole();
		freopen("conin$", "r", stdin);
		freopen("conout$", "w", stdout);
		freopen("conout$", "w", stderr);
	}
	
	Hook(0x62152B, Main, HookFlags::IsCall).Apply();

	// mopp freeze hack (fixes the surface index so that if it's in a small bsp, then the index is <= 0xFFFF. HALO 3 ONLY. (ODST doesn<t require any of this)
	Hook(0xD0E9E3 - 0x400000, loc_D0E9E3_hook).Apply();
	Hook(0xD0D919 - 0x400000, loc_D0D919_hook).Apply();
	Hook(0xD0F505 - 0x400000, loc_D0F505_hook).Apply();

	// fixes the amd freeze
	Hook(0x658061, ContrailFixHook).Apply();

	return true;
}

BOOL ExitInstance()
{
	return true;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD Reason, LPVOID Misc)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH: return InitInstance(hModule);
	case DLL_PROCESS_DETACH: return ExitInstance();
	case DLL_THREAD_ATTACH:
		return true;
	case DLL_THREAD_DETACH:
		return true;
	}

	return false;
}

