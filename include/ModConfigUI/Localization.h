#pragma once

namespace ModConfigUI::Localization
{
	// Called by ModConfigUI::Install.
	void Load(const char* a_pluginName);

	// Returns a_key itself when there is no entry for it.
	const char* Get(const char* a_key);

	// A translation that doesn't match the arguments is returned unformatted.
	template <class... Args>
	std::string Format(const char* a_key, Args&... a_args)
	{
		const char* translation = Get(a_key);
		try
		{
			return std::vformat(translation, std::make_format_args(a_args...));
		}
		catch (const std::format_error&)
		{
			return translation;
		}
	}
}
