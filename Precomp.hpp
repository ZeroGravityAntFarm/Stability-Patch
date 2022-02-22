#pragma once
#pragma warning (disable : 4996)
#pragma warning (disable : 4018)
#pragma warning (disable : 4267)
#pragma warning (disable : 26495)
#pragma warning (disable : 26444)
#pragma warning (disable : 26451)
#include <windows.h>
#include <IPTypes.h>
#include <string>
#include <cstdint>
#include <algorithm>
#include <vector>
#include <deque>
#include <Windows.h>
#include <functional>
#include <sstream>
#include <fstream>
#include <iostream>
#include <limits>
#include <functional>
#include <cctype>
#include <codecvt>
#include <iomanip>
#include <shellapi.h>
#include <map>
#include "Hooking.hpp"
#include "Pointer.hpp"
#include "Utils/Singleton.hpp"
#include "DatumHandle.hpp"
#include "Blam/BlamData.hpp"
#include "Blam/BlamNetwork.hpp"
#include "Blam/BlamTypes.hpp"
#include "Blam/BlamObjects.hpp"
#include "Simulation.hpp"
#include "Utils/Utils.hpp"
#include "DatumHandle.hpp"
#include "Definitions/EnumDefinition.hpp"
#include "Definitions/FieldDefinition.hpp"
#include "Definitions/StructDefinition.hpp"
#include "Blam/Math/RealColorRGB.hpp"

class ElDorito : public Utils::Singleton < ElDorito >
{
public:
	bool GameHasMenuShown = false;

	static void SetMainThreadID(size_t ThreadID)
	{
		MainThreadID = ThreadID;
	}
	static size_t GetMainThreadID()
	{
		return MainThreadID;
	}

	static Pointer GetMainTls(size_t Offset = 0);

	ElDorito();
	~ElDorito() = default;

	std::string GetDirectory();

	void Initialize();
	void Tick();
	void OnMainMenuShown();
	std::string GetMapsFolder() const { return mapsFolder; }
	bool IsWebDebuggingEnabled() const { return webDebugging; }
	bool IsDedicated() const { return isDedicated; }
	std::string GetInstanceName() const { return instanceName; }

private:
	static size_t MainThreadID; // Thread
	bool executeCommandQueue = false;
	bool isDedicated = false;
	std::string mapsFolder;
	bool webDebugging = false;
	bool connectToServer = false;
	std::string serverAddress = "";
	std::string serverPassword = "";
	std::string instanceName = "";
	bool skipTitleSplash = false;
	static bool(__cdecl* Video_InitD3D)(bool, bool);

	void setWatermarkText(const std::string& Message);
	void killProcessByName(const char* filename, int ourProcessID);
	static bool __cdecl hooked_Video_InitD3D(bool windowless, bool nullRefDevice);
};