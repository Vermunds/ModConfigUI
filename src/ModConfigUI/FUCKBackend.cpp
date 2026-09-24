#include "Internal.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

// FUCK_API.h still logs through SKSE::log, which CommonLib no longer provides
namespace SKSE::log
{
	using spdlog::error;
	using spdlog::info;
}

#include <FUCK/src/FUCK_API.h>

namespace
{
	constexpr ImVec4 FOOTER_COLOR{ ModConfigUI::Internal::TOOLTIP_FOOTER_COLOR[0], ModConfigUI::Internal::TOOLTIP_FOOTER_COLOR[1], ModConfigUI::Internal::TOOLTIP_FOOTER_COLOR[2], ModConfigUI::Internal::TOOLTIP_FOOTER_COLOR[3] };

	// The vertical frame padding F.U.C.K's AlignTextToFramePadding and buttons use, before its UI scale.
	constexpr float TEXT_PADDING_Y = 7.0f;

	ImVec4 GetMutedTextColor()
	{
		ImVec4 text = FUCK::GetStyleColorVec4(ImGuiCol_Text);
		ImVec4 disabled = FUCK::GetStyleColorVec4(ImGuiCol_TextDisabled);
		constexpr float BLEND = ModConfigUI::Internal::MUTED_TEXT_BLEND;

		return ImVec4{
			text.x + (disabled.x - text.x) * BLEND,
			text.y + (disabled.y - text.y) * BLEND,
			text.z + (disabled.z - text.z) * BLEND,
			text.w + (disabled.w - text.w) * BLEND
		};
	}

	// Tooltip for the item just drawn, with a_footer as a last line in the footer colour. Either may be null.
	void SetOptionalTooltip(const char* a_text, const char* a_footer)
	{
		if (!a_footer)
		{
			if (a_text && FUCK::IsItemHovered())
			{
				FUCK::SetTooltip(a_text);
			}
			return;
		}

		if (!FUCK::IsItemHovered() || !FUCK::BeginTooltip())
		{
			return;
		}

		if (a_text)
		{
			FUCK::TextUnformatted(a_text);
		}
		FUCK::TextColored(FOOTER_COLOR, "%s", a_footer);

		FUCK::EndTooltip();
	}

	// F.U.C.K numbers the gamepad buttons one gamepad offset higher than SKSE::InputMap does: it adds
	// kMacro_GamepadOffset to key codes that already start there. Keyboard and mouse codes agree.
	std::uint32_t ToFUCKKey(ModConfigUI::ButtonBinding a_binding)
	{
		if (a_binding.GetDevice() == RE::INPUT_DEVICE::kGamepad)
		{
			return a_binding.key + SKSE::InputMap::kMacro_GamepadOffset;
		}
		return a_binding.key;
	}

	// a_binding where the cursor is, as its image or, lacking one, as text. F.U.C.K would draw its
	// "unknown key" image for a key the set has no picture of, the code says more than that does.
	void DrawButtonImage(ModConfigUI::ButtonBinding a_binding)
	{
		ImVec2 size{};
		ImTextureID texture = ModConfigUI::Internal::GetButtonIconName(a_binding) ? FUCK::GetIconForKey(ToFUCKKey(a_binding), &size) : ImTextureID{};
		if (!texture || size.y <= 0.0f)
		{
			FUCK::AlignTextToFramePadding();
			FUCK::TextUnformatted(ModConfigUI::Internal::GetButtonText(a_binding).c_str());
			return;
		}

		// One row tall, whatever icon scale the user picked, so it lines up with the text around it.
		float height = FUCK::GetFrameHeight();
		FUCK::DrawImage(texture, ImVec2{ size.x * height / size.y, height });
	}

	// As SetOptionalTooltip, with the footer naming a_default by the image of its key.
	void SetOptionalTooltip(const char* a_text, ModConfigUI::ButtonBinding a_default)
	{
		if (!FUCK::IsItemHovered() || !FUCK::BeginTooltip())
		{
			return;
		}

		if (a_text)
		{
			FUCK::TextUnformatted(a_text);
		}

		std::string before;
		std::string after;
		ModConfigUI::Internal::GetDefaultLineParts(before, after);


		// The text either side is lowered to the middle of the image, which is a whole row tall. F.U.C.K's
		// AlignTextToFramePadding uses a fixed padding rather than the one GetFrameHeight is made of, so the
		// offset is worked out here instead.
		float lineY = FUCK::GetCursorPos().y;
		float textY = lineY + (FUCK::GetFrameHeight() - FUCK::GetTextLineHeight()) * 0.5f;

		FUCK::SetCursorPosY(textY);
		FUCK::TextColored(FOOTER_COLOR, "%s", before.c_str());
		FUCK::SameLine(0.0f, 0.0f);
		FUCK::SetCursorPosY(lineY);
		DrawButtonImage(a_default);
		if (!after.empty())
		{
			FUCK::SameLine(0.0f, 0.0f);
			FUCK::SetCursorPosY(textY);
			FUCK::TextColored(FOOTER_COLOR, "%s", after.c_str());
		}

		FUCK::EndTooltip();
	}

	class FUCKRenderer : public ModConfigUI::Renderer
	{
	private:
		// The empty hint field the user clicked into, and whether it still has to be given the focus.
		std::string m_hintEditLabel;
		bool m_hintFocusPending = false;

		// Holds the hint of the field being drawn, ImGui needs a writable buffer even for a read only one.
		std::vector<char> m_hintBuffer;

		// Set between BeginColumns and EndColumns, where checkboxes are pushed to the right edge of their column.
		bool m_inColumns = false;

		// Whether NextColumn was already called since BeginColumns, so the next call has a margin to skip.
		bool m_columnStarted = false;

	public:
		void SeparatorText(const char* a_label) override { FUCK::SeparatorText(a_label); }
		void Separator() override { FUCK::Separator(); }
		void TextWrapped(const char* a_text) override { FUCK::TextWrapped("%s", a_text); }
		void TextMuted(const char* a_text) override { FUCK::TextColored(GetMutedTextColor(), "%s", a_text); }
		void TextWrappedMuted(const char* a_text) override { FUCK::TextColoredWrapped(GetMutedTextColor(), "%s", a_text); }
		void TextDisabled(const char* a_text) override { FUCK::TextDisabled("%s", a_text); }

		void LabeledText(const char* a_label, const char* a_value) override
		{
			FUCK::LeftLabel(a_label);
			FUCK::TextColored(GetMutedTextColor(), "%s", a_value);
		}
		void Text(const char* a_text) override { FUCK::TextUnformatted(a_text); }
		// Lowered to the middle of the line AlignTextToFramePadding assumes, which F.U.C.K builds from a fixed
		// padding of its own and is what its buttons are as tall as, so the image lines up with them and with text.
		void ButtonImage(ModConfigUI::ButtonBinding a_binding) override
		{
			float lineHeight = FUCK::GetTextLineHeight() + FUCK::UIScale(TEXT_PADDING_Y) * 2.0f;
			float offset = (lineHeight - FUCK::GetFrameHeight()) * 0.5f;
			if (offset > 0.0f)
			{
				FUCK::SetCursorPosY(FUCK::GetCursorPos().y + offset);
			}
			DrawButtonImage(a_binding);
		}

		void Spacing() override { FUCK::Spacing(); }
		void SameLine() override { FUCK::SameLine(); }

		void BeginDisabled(bool a_disabled) override { FUCK::BeginDisabled(a_disabled); }
		void EndDisabled() override { FUCK::EndDisabled(); }

		bool Button(const char* a_label) override { return FUCK::Button(a_label); }
		// F.U.C.K centres the checkbox far from its label, so it is placed here instead: at the start of the widget
		// half in a regular row, against the far edge of the cell inside columns. The group lets a tooltip cover both.
		bool Checkbox(const char* a_label, bool* a_value) override
		{
			std::string id = "##"s + a_label;

			if (!m_inColumns)
			{
				FUCK::BeginGroup();
				FUCK::LeftLabel(a_label);  // Leaves the cursor where the widget half starts
				bool changed = FUCK::Checkbox(id.c_str(), a_value, false, false);
				FUCK::EndGroup();
				return changed;
			}

			float startX = FUCK::GetCursorPos().x;
			float availableWidth = FUCK::GetContentRegionAvail().x;

			FUCK::BeginGroup();
			FUCK::AlignTextToFramePadding();
			FUCK::TextUnformatted(a_label);
			FUCK::SameLine(0.0f, 0.0f);
			FUCK::SetCursorPosX(startX + availableWidth - FUCK::GetFrameHeight());

			bool changed = FUCK::Checkbox(id.c_str(), a_value, false, false);
			FUCK::EndGroup();
			return changed;
		}

		bool CheckboxWidget(const char* a_id, bool* a_value) override
		{
			std::string id = "##"s + a_id;
			return FUCK::Checkbox(id.c_str(), a_value, false, false);
		}

		bool InputText(const char* a_label, char* a_buffer, std::size_t a_bufferSize) override { return FUCK::InputText(a_label, a_buffer, a_bufferSize); }
		bool Combo(const char* a_label, std::int32_t* a_currentItem, const char* const* a_items, std::int32_t a_itemCount) override { return FUCK::Combo(a_label, a_currentItem, a_items, a_itemCount); }

		bool SliderFloat(const char* a_label, float* a_value, float a_min, float a_max, const char* a_format) override
		{
			FUCK::SliderFloat(a_label, a_value, a_min, a_max, a_format);
			return FUCK::IsItemDeactivatedAfterEdit();
		}

		bool SliderInt(const char* a_label, std::int32_t* a_value, std::int32_t a_min, std::int32_t a_max, const char* a_format) override
		{
			FUCK::SliderInt(a_label, a_value, a_min, a_max, a_format);
			return FUCK::IsItemDeactivatedAfterEdit();
		}

		// F.U.C.K has no input with a hint and doesn't expose enough to draw one into the field. An empty field
		// is drawn as a read only one holding the hint instead, and clicking it focuses the real, empty field.
		bool InputTextWithHint(const char* a_label, const char* a_hint, char* a_buffer, std::size_t a_bufferSize) override
		{
			bool editing = m_hintEditLabel == a_label;

			if (a_buffer[0] == '\0' && !editing)
			{
				// Same label, so ImGui sees the same widget across the swap.
				m_hintBuffer.assign(a_hint, a_hint + std::strlen(a_hint) + 1);

				FUCK::InputText(a_label, m_hintBuffer.data(), m_hintBuffer.size(), ImGuiInputTextFlags_ReadOnly);

				if (FUCK::IsItemClicked())
				{
					m_hintEditLabel = a_label;
					m_hintFocusPending = true;
				}

				return false;
			}

			if (editing && m_hintFocusPending)
			{
				FUCK::SetKeyboardFocusHere();
				m_hintFocusPending = false;
			}

			bool changed = FUCK::InputText(a_label, a_buffer, a_bufferSize);

			// Leaving an empty field puts the hint back.
			if (editing && FUCK::IsItemDeactivated())
			{
				m_hintEditLabel.clear();
			}

			return changed;
		}

		bool BeginTable(const char* a_id, const char* const* a_columnHeaders, std::int32_t a_columnCount, const float* a_columnWidths) override
		{
			if (!FUCK::BeginTable(a_id, a_columnCount, FUCK::TableFlags::kSizingStretchProp | FUCK::TableFlags::kRowBg))
			{
				return false;
			}

			for (std::int32_t i = 0; i < a_columnCount; ++i)
			{
				float width = a_columnWidths ? a_columnWidths[i] : 1.0f;
				FUCK::TableSetupColumn(a_columnHeaders[i], FUCK::TableColumnFlags::kWidthStretch, width);
			}

			FUCK::TableHeadersRow();
			return true;
		}

		void EndTable() override { FUCK::EndTable(); }

		void TableNextRow() override { FUCK::TableNextRow(); }
		void TableNextColumn() override
		{
			FUCK::TableNextColumn();
			FUCK::AlignTextToFramePadding();
		}

		// Every column is followed by a narrow empty one, which is the margin on its right.
		bool BeginColumns(const char* a_id, std::int32_t a_columnCount) override
		{
			if (!FUCK::BeginTable(a_id, a_columnCount * 2, FUCK::TableFlags::kSizingStretchSame))
			{
				return false;
			}

			float margin = FUCK::GetFrameHeight();
			for (std::int32_t i = 0; i < a_columnCount; ++i)
			{
				FUCK::TableSetupColumn(nullptr, FUCK::TableColumnFlags::kWidthStretch, 1.0f);
				FUCK::TableSetupColumn(nullptr, FUCK::TableColumnFlags::kWidthFixed, margin);
			}

			m_inColumns = true;
			m_columnStarted = false;
			return true;
		}

		void NextColumn() override
		{
			if (m_columnStarted)
			{
				FUCK::TableNextColumn();
			}
			FUCK::TableNextColumn();
			m_columnStarted = true;
		}

		void EndColumns() override
		{
			FUCK::EndTable();
			m_inColumns = false;
		}

		void PushID(std::int32_t a_id) override { FUCK::PushID(a_id); }
		void PopID() override { FUCK::PopID(); }

		void ItemTooltip(const char* a_text) override { SetOptionalTooltip(a_text, nullptr); }
		void ItemTooltip(const char* a_text, const char* a_footer) override { SetOptionalTooltip(a_text, a_footer); }
		void ItemTooltip(const char* a_text, ModConfigUI::ButtonBinding a_default) override { SetOptionalTooltip(a_text, a_default); }

		void OpenPopup(const char* a_id) override { FUCK::OpenPopup(a_id); }
		bool BeginPopupModal(const char* a_id) override { return FUCK::BeginPopupModal(a_id); }
		void EndPopup() override { FUCK::EndPopup(); }
		void CloseCurrentPopup() override { FUCK::CloseCurrentPopup(); }

		std::string LabelColumn(const char* a_label) override
		{
			FUCK::LeftLabel(a_label);
			return "##"s + a_label;
		}

		// F.U.C.K's hotkey widget, used only to draw the binding and report the click. Its managed hotkey
		// API would take over the capture too, but can't honour the device filter.
		bool BindWidget(const char* a_label, ModConfigUI::ButtonBinding a_binding, bool a_listening) override
		{
			// As with Checkbox, F.U.C.K would push the widget to the far edge, so the label is drawn here and
			// the widget follows it at the start of its half. No modifiers, a binding here is a single button.
			std::string id = LabelColumn(a_label);
			return FUCK::Hotkey(id.c_str(), ToFUCKKey(a_binding), -1, -1, false, false, a_listening);
		}
	};

	FUCKRenderer renderer;

	class PageTool : public FUCK::ITool
	{
	public:
		explicit PageTool(std::size_t a_index) :
			m_index(a_index) {}

		const char* Name() const override { return ModConfigUI::Internal::GetPageName(m_index); }
		const char* Group() const override { return ModConfigUI::Internal::GetDisplayName(); }
		void Draw() override { ModConfigUI::Internal::DrawPage(m_index, renderer); }

		// F.U.C.K hands its tools the head of the event chain, which is the shape the capture wants.
		// Every page gets the call, only the one holding an armed binder answers it.
		bool OnAsyncInput(const void* a_event) override
		{
			return ModConfigUI::Internal::OnInputEvent(static_cast<const RE::InputEvent* const*>(a_event));
		}

	private:
		std::size_t m_index;
	};

	// The framework keeps the pointers, so the tools have to outlive registration.
	std::vector<std::unique_ptr<PageTool>> tools;
}

namespace ModConfigUI::Internal
{
	void InstallFUCK()
	{
		if (!FUCK::Connect(GetModInfo().pluginName))
		{
			logger::info("F.U.C.K not found, skipping menu registration.");
			return;
		}

		for (std::size_t i = 0; i < GetPageCount(); ++i)
		{
			tools.push_back(std::make_unique<PageTool>(i));
			FUCK::RegisterTool(tools.back().get());
		}

		logger::info("Registered menu with F.U.C.K.");
	}
}
