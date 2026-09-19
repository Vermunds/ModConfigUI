#pragma once

#include "ModConfigUI/Renderer.h"

namespace ModConfigUI
{
	inline constexpr std::size_t MAX_PAGES = 15;  // The most pages a single mod can register, on top of the built in "Mod Info" page.

	struct ModInfo
	{
		const char* pluginName;   // SKSE plugin name, used for the log file and to register with F.U.C.K
		const char* displayName;  // Name of the menu section, may be a translation key
		const char* version;
		const char* author;
		const char* description;
		const char* nexusUrl;
		const char* sourceUrl;
	};

	using DrawFunction = void (*)(Renderer& a_renderer);
	using RestoreDefaultsFunction = void (*)();

	struct Page
	{
		const char* name;
		DrawFunction draw;
	};

	// Registers the menu with every supported framework that is present. Call once, in kPostLoad.
	// a_restoreDefaults is invoked by the confirmation popup on the "Mod Info" page when the user clicks "Restore Defaults". Passing nullptr leaves the button out.
	void Install(const ModInfo& a_modInfo, std::span<const Page> a_pages, RestoreDefaultsFunction a_restoreDefaults = nullptr);
}
