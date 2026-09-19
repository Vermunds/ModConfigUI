#include "Internal.h"

namespace
{
	ModConfigUI::ModInfo modInfo{};
	std::vector<ModConfigUI::Page> pages;
	ModConfigUI::RestoreDefaultsFunction restoreDefaults = nullptr;
	bool installed = false;
}

namespace ModConfigUI
{
	namespace Internal
	{
		const ModInfo& GetModInfo()
		{
			return modInfo;
		}

		const char* GetDisplayName()
		{
			return Localization::Get(modInfo.displayName);
		}

		std::size_t GetPageCount()
		{
			return pages.size() + 1;
		}

		const char* GetPageName(std::size_t a_index)
		{
			if (a_index == 0)
			{
				return Localization::Get("$ModConfigUI_ModInfo");
			}
			return Localization::Get(pages[a_index - 1].name);
		}

		void DrawPage(std::size_t a_index, Renderer& a_renderer)
		{
			if (a_index >= GetPageCount())
			{
				return;
			}

			if (a_index == 0)
			{
				DrawModInfoPage(a_renderer);
			}
			else
			{
				pages[a_index - 1].draw(a_renderer);
			}
		}

		RestoreDefaultsFunction GetRestoreDefaults()
		{
			return restoreDefaults;
		}
	}

	void Install(const ModInfo& a_modInfo, std::span<const Page> a_pages, RestoreDefaultsFunction a_restoreDefaults)
	{
		if (installed)
		{
			SKSE::log::warn("ModConfigUI::Install called more than once, ignoring.");
			return;
		}

		if (!a_modInfo.pluginName || !a_modInfo.displayName)
		{
			SKSE::log::error("ModConfigUI::Install requires at least a plugin name and a display name.");
			return;
		}

		if (a_pages.size() > MAX_PAGES)
		{
			SKSE::log::error("ModConfigUI::Install was given {} pages, only the first {} are registered.", a_pages.size(), MAX_PAGES);
			a_pages = a_pages.first(MAX_PAGES);
		}

		modInfo = a_modInfo;
		pages.assign(a_pages.begin(), a_pages.end());
		restoreDefaults = a_restoreDefaults;
		installed = true;

		Localization::Load(a_modInfo.pluginName);

		Internal::InstallSKSEMenuFramework();
		Internal::InstallFUCK();
	}
}
