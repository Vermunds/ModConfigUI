#include "ModConfigUI/Localization.h"

namespace
{
	std::unordered_map<std::string, std::string> translations;

	std::string GetGameLanguage()
	{
		RE::Setting* setting = RE::GetINISetting("sLanguage:General");
		if (!setting || setting->GetType() != RE::Setting::Type::kString)
		{
			return "ENGLISH"s;
		}

		std::string language = setting->GetString();
		for (char& character : language)
		{
			character = static_cast<char>(std::toupper(static_cast<std::uint8_t>(character)));
		}
		return language;
	}

	std::filesystem::path GetTranslationPath(const char* a_pluginName, const std::string& a_language)
	{
		return std::filesystem::path(std::format(R"(Data\Interface\Translations\{}_{}.txt)", a_pluginName, a_language));
	}

	std::string UnescapeNewlines(const std::string& a_value)
	{
		std::string result;
		result.reserve(a_value.size());

		for (std::size_t i = 0; i < a_value.size(); ++i)
		{
			if (a_value[i] == '\\' && i + 1 < a_value.size() && a_value[i + 1] == 'n')
			{
				result.push_back('\n');
				++i;
			}
			else
			{
				result.push_back(a_value[i]);
			}
		}

		return result;
	}

	bool ReadFileAsUTF8(const std::filesystem::path& a_path, std::string& a_contents)
	{
		std::ifstream file(a_path, std::ios::binary | std::ios::ate);
		if (!file.good())
		{
			return false;
		}

		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		if (size < 2)
		{
			logger::error("Translation file '{}' is too small to hold a byte order mark.", a_path.string());
			return false;
		}

		std::vector<char> buffer(static_cast<std::size_t>(size));
		if (!file.read(buffer.data(), size))
		{
			return false;
		}

		if (static_cast<std::uint8_t>(buffer[0]) != 0xFF || static_cast<std::uint8_t>(buffer[1]) != 0xFE)
		{
			logger::error("Translation file '{}' must be encoded in UTF-16 LE (UCS-2 LE).", a_path.string());
			return false;
		}

		std::wstring_view wide(reinterpret_cast<const wchar_t*>(buffer.data() + 2), static_cast<std::size_t>(size - 2) / sizeof(wchar_t));
		if (wide.empty())
		{
			a_contents.clear();
			return true;
		}

		if (!REX::UTF16_TO_UTF8(wide, a_contents))
		{
			logger::error("Failed to convert translation file '{}' to UTF-8.", a_path.string());
			return false;
		}

		return true;
	}

	bool ParseFile(const std::filesystem::path& a_path)
	{
		std::string contents;
		if (!ReadFileAsUTF8(a_path, contents))
		{
			return false;
		}

		logger::info("Reading translations from '{}'...", a_path.string());

		std::istringstream stream(contents);
		std::string line;
		std::size_t count = 0;

		while (std::getline(stream, line))
		{
			if (!line.empty() && line.back() == '\r')
			{
				line.pop_back();
			}

			if (line.empty())
			{
				continue;
			}

			std::istringstream lineStream(line);
			std::string key;
			std::string value;

			lineStream >> key;
			std::getline(lineStream >> std::ws, value);

			if (key.empty() || value.empty())
			{
				continue;
			}

			translations.insert_or_assign(key, UnescapeNewlines(value));
			++count;
		}

		logger::info("\tLoaded {} translation entries.", count);
		return true;
	}
}

namespace ModConfigUI::Localization
{
	void Load(const char* a_pluginName)
	{
		translations.clear();

		if (!a_pluginName)
		{
			return;
		}

		const std::string language = GetGameLanguage();
		if (ParseFile(GetTranslationPath(a_pluginName, language)))
		{
			return;
		}

		if (language != "ENGLISH")
		{
			logger::info("No {} translation file found for {}, falling back to english...", language, a_pluginName);
			if (ParseFile(GetTranslationPath(a_pluginName, "ENGLISH"s)))
			{
				return;
			}
		}

		logger::warn("No translation file found for {}.", a_pluginName);
	}

	const char* Get(const char* a_key)
	{
		if (!a_key)
		{
			return "";
		}

		std::unordered_map<std::string, std::string>::const_iterator it = translations.find(a_key);
		if (it != translations.end())
		{
			return it->second.c_str();
		}

		return a_key;
	}
}
