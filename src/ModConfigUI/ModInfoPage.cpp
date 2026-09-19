#include "Internal.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <shellapi.h>

#pragma comment(lib, "Shell32.lib")

namespace
{
	void OpenInShell(const char* a_target)
	{
		HINSTANCE result = ShellExecuteA(nullptr, "open", a_target, nullptr, nullptr, SW_SHOWNORMAL);
		if (reinterpret_cast<std::intptr_t>(result) <= 32)
		{
			SKSE::log::warn("Failed to open '{}' (error {}).", a_target, reinterpret_cast<std::intptr_t>(result));
		}
	}

	std::string GetLogFilePath(const char* a_pluginName)
	{
		std::optional<std::filesystem::path> directory = SKSE::log::log_directory();
		if (!directory.has_value())
		{
			return std::string();
		}
		return (directory.value() / (std::string(a_pluginName) + ".log")).string();
	}
}

namespace ModConfigUI::Internal
{
	void DrawModInfoPage(Renderer& a_renderer)
	{
		const ModInfo& modInfo = GetModInfo();

		a_renderer.SeparatorText(Localization::Get("$ModConfigUI_ModInfo"));

		a_renderer.LabeledText(Localization::Get("$ModConfigUI_Name"), GetDisplayName());
		if (modInfo.version)
		{
			a_renderer.LabeledText(Localization::Get("$ModConfigUI_Version"), modInfo.version);
		}
		if (modInfo.author)
		{
			a_renderer.LabeledText(Localization::Get("$ModConfigUI_Author"), modInfo.author);
		}

		if (modInfo.description)
		{
			a_renderer.SeparatorText(Localization::Get("$ModConfigUI_Description"));
			a_renderer.TextWrappedMuted(Localization::Get(modInfo.description));
		}

		a_renderer.SeparatorText(Localization::Get("$ModConfigUI_Resources"));

		// A button is left out when the mod didn't provide a target for it.
		bool hasButton = false;
		if (modInfo.nexusUrl && modInfo.nexusUrl[0] != '\0')
		{
			if (a_renderer.Button(Localization::Get("$ModConfigUI_NexusLink")))
			{
				OpenInShell(modInfo.nexusUrl);
			}
			a_renderer.ItemTooltip(modInfo.nexusUrl);
			hasButton = true;
		}

		if (modInfo.sourceUrl && modInfo.sourceUrl[0] != '\0')
		{
			if (hasButton)
			{
				a_renderer.SameLine();
			}
			if (a_renderer.Button(Localization::Get("$ModConfigUI_ViewSource")))
			{
				OpenInShell(modInfo.sourceUrl);
			}
			a_renderer.ItemTooltip(modInfo.sourceUrl);
			hasButton = true;
		}

		std::string logFilePath = GetLogFilePath(modInfo.pluginName);
		if (!logFilePath.empty())
		{
			if (hasButton)
			{
				a_renderer.SameLine();
			}
			if (a_renderer.Button(Localization::Get("$ModConfigUI_ViewLogFile")))
			{
				OpenInShell(logFilePath.c_str());
			}
			a_renderer.ItemTooltip(logFilePath.c_str());
		}

		RestoreDefaultsFunction restoreDefaults = GetRestoreDefaults();
		if (!restoreDefaults)
		{
			return;
		}

		a_renderer.SeparatorText(Localization::Get("$ModConfigUI_Settings"));

		const char* restoreDefaultsPopup = Localization::Get("$ModConfigUI_RestoreDefaultsPopup");

		if (a_renderer.Button(Localization::Get("$ModConfigUI_RestoreDefaults")))
		{
			a_renderer.OpenPopup(restoreDefaultsPopup);
		}
		a_renderer.ItemTooltip(Localization::Get("$ModConfigUI_RestoreDefaults_Tooltip"));

		if (a_renderer.BeginPopupModal(restoreDefaultsPopup))
		{
			a_renderer.TextWrapped(Localization::Get("$ModConfigUI_RestoreDefaults_Confirm"));
			a_renderer.Spacing();

			if (a_renderer.Button(Localization::Get("$ModConfigUI_Restore")))
			{
				restoreDefaults();
				a_renderer.CloseCurrentPopup();
			}

			a_renderer.SameLine();

			if (a_renderer.Button(Localization::Get("$ModConfigUI_Cancel")))
			{
				a_renderer.CloseCurrentPopup();
			}

			a_renderer.EndPopup();
		}
	}
}
