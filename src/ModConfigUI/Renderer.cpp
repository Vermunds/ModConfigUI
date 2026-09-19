#include "Internal.h"

namespace
{
	// A slider's default, written the way the slider writes its own value. The formats come from the
	// mods, never from a file, so they are as trusted as the ones handed to the slider itself.
	template <class T>
	std::string FormatNumber(const char* a_format, T a_value)
	{
		char buffer[64]{};
		std::snprintf(buffer, sizeof(buffer), a_format, a_value);
		return buffer;
	}
}

namespace ModConfigUI
{
	std::string GetDefaultLine(const char* a_value)
	{
		if (!a_value || a_value[0] == '\0')
		{
			a_value = Localization::Get("$ModConfigUI_Default_Empty");
		}

		return Localization::Format("$ModConfigUI_Default", a_value);
	}

	std::string GetDefaultLine(bool a_value)
	{
		return GetDefaultLine(a_value ? Localization::Get("$ModConfigUI_Default_Enabled") : Localization::Get("$ModConfigUI_Default_Disabled"));
	}

	namespace Internal
	{
		void GetDefaultLineParts(std::string& a_before, std::string& a_after)
		{
			// The line is written around a marker no translation would contain, then split where it landed.
			constexpr std::string_view MARKER = "\x1F";

			std::string line = GetDefaultLine(MARKER.data());
			std::size_t position = line.find(MARKER);
			if (position == std::string::npos)
			{
				a_before = line;
				a_after.clear();
				return;
			}

			a_before = line.substr(0, position);
			a_after = line.substr(position + MARKER.size());
		}
	}

	bool Renderer::Checkbox(const char* a_label, bool* a_value, bool a_default, const char* a_tooltip)
	{
		bool changed = Checkbox(a_label, a_value);
		ItemTooltip(a_tooltip, GetDefaultLine(a_default).c_str());
		return changed;
	}

	bool Renderer::InputText(const char* a_label, char* a_buffer, std::size_t a_bufferSize, const char* a_default, const char* a_tooltip)
	{
		bool changed = InputText(a_label, a_buffer, a_bufferSize);
		ItemTooltip(a_tooltip, GetDefaultLine(a_default).c_str());
		return changed;
	}

	bool Renderer::InputTextWithHint(const char* a_label, const char* a_hint, char* a_buffer, std::size_t a_bufferSize, const char* a_default, const char* a_tooltip)
	{
		bool changed = InputTextWithHint(a_label, a_hint, a_buffer, a_bufferSize);
		ItemTooltip(a_tooltip, GetDefaultLine(a_default).c_str());
		return changed;
	}

	bool Renderer::Combo(const char* a_label, std::int32_t* a_currentItem, const char* const* a_items, std::int32_t a_itemCount, std::int32_t a_default, const char* a_tooltip)
	{
		bool changed = Combo(a_label, a_currentItem, a_items, a_itemCount);
		const char* defaultItem = (a_default >= 0 && a_default < a_itemCount) ? a_items[a_default] : nullptr;
		ItemTooltip(a_tooltip, GetDefaultLine(defaultItem).c_str());
		return changed;
	}

	bool Renderer::SliderFloat(const char* a_label, float* a_value, float a_min, float a_max, const char* a_format, float a_default, const char* a_tooltip)
	{
		bool finished = SliderFloat(a_label, a_value, a_min, a_max, a_format);
		std::string value = FormatNumber(a_format ? a_format : "%.3f", static_cast<double>(a_default));
		ItemTooltip(a_tooltip, GetDefaultLine(value.c_str()).c_str());
		return finished;
	}

	bool Renderer::SliderInt(const char* a_label, std::int32_t* a_value, std::int32_t a_min, std::int32_t a_max, const char* a_format, std::int32_t a_default, const char* a_tooltip)
	{
		bool finished = SliderInt(a_label, a_value, a_min, a_max, a_format);
		std::string value = FormatNumber(a_format ? a_format : "%d", a_default);
		ItemTooltip(a_tooltip, GetDefaultLine(value.c_str()).c_str());
		return finished;
	}

	bool Renderer::BindButton(const char* a_label, ButtonBinding* a_value, BindFilter a_filter)
	{
		return DrawBindButton(a_label, a_value, a_filter, nullptr, nullptr);
	}

	bool Renderer::BindButton(const char* a_label, ButtonBinding* a_value, ButtonBinding a_default, const char* a_tooltip, BindFilter a_filter)
	{
		return DrawBindButton(a_label, a_value, a_filter, a_tooltip, &a_default);
	}

	bool Renderer::DrawBindButton(const char* a_label, ButtonBinding* a_value, BindFilter a_filter, const char* a_tooltip, const ButtonBinding* a_default)
	{
		if (!a_label || !a_value)
		{
			return false;
		}

		namespace Capture = Internal::Capture;

		// Widgets are told apart by their label, so the capture is owned by the same thing that identifies
		// the widget drawing it. Two binders under one label would be one widget either way.
		std::uint32_t owner = static_cast<std::uint32_t>(std::hash<std::string_view>{}(a_label));
		bool listening = Capture::IsArmed(owner);
		bool changed = false;

		ButtonBinding captured;
		switch (Capture::Poll(owner, captured))
		{
		case Capture::Result::kCaptured:
			*a_value = captured;
			changed = true;
			listening = false;
			break;
		case Capture::Result::kCancelled:
			listening = false;
			break;
		default:
			break;
		}

		// The widget carries the binding as its own text, so its label can't tell two of them apart.
		PushID(static_cast<std::int32_t>(owner));

		if (BindWidget(a_label, *a_value, listening))
		{
			if (listening)
			{
				Capture::Cancel(owner);
			}
			else
			{
				Capture::Begin(owner, a_filter);
			}
		}

		// While listening the only thing worth saying is how to finish, the setting's own hint returns after.
		const char* tooltip = listening ? Localization::Get("$ModConfigUI_Bind_ListeningTooltip") : (a_tooltip ? a_tooltip : Localization::Get("$ModConfigUI_Bind_Tooltip"));
		if (a_default)
		{
			ItemTooltip(tooltip, *a_default);
		}
		else
		{
			ItemTooltip(tooltip);
		}

		PopID();

		return changed;
	}

	bool Renderer::BindWidget(const char* a_label, ButtonBinding a_binding, bool a_listening)
	{
		LabelColumn(a_label);
		std::string text = a_listening ? Localization::Get("$ModConfigUI_Bind_Listening") : Internal::GetButtonText(a_binding);

		return Button(text.c_str());
	}
}
