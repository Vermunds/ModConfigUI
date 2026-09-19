#include "Internal.h"

#include <SKSEMenuFramework.h>

namespace
{
	constexpr ImGuiMCP::ImVec4 FOOTER_COLOR{ ModConfigUI::Internal::TOOLTIP_FOOTER_COLOR[0], ModConfigUI::Internal::TOOLTIP_FOOTER_COLOR[1], ModConfigUI::Internal::TOOLTIP_FOOTER_COLOR[2], ModConfigUI::Internal::TOOLTIP_FOOTER_COLOR[3] };

	ImGuiMCP::ImVec4 GetMutedTextColor()
	{
		const ImGuiMCP::ImVec4* text = ImGuiMCP::GetStyleColorVec4(ImGuiMCP::ImGuiCol_Text);
		const ImGuiMCP::ImVec4* disabled = ImGuiMCP::GetStyleColorVec4(ImGuiMCP::ImGuiCol_TextDisabled);
		constexpr float BLEND = ModConfigUI::Internal::MUTED_TEXT_BLEND;

		return ImGuiMCP::ImVec4{
			text->x + (disabled->x - text->x) * BLEND,
			text->y + (disabled->y - text->y) * BLEND,
			text->z + (disabled->z - text->z) * BLEND,
			text->w + (disabled->w - text->w) * BLEND
		};
	}

	// Draws the label on the left and moves the cursor to the start of the control column, the layout
	// F.U.C.K has natively. Returns the hidden id to pass to the widget in place of its label.
	std::string DrawRowLabel(const char* a_label)
	{
		float startX = ImGuiMCP::GetCursorPosX();
		float labelWidth = ImGuiMCP::GetContentRegionAvail().x * ModConfigUI::LABEL_COLUMN_RATIO;

		ImGuiMCP::AlignTextToFramePadding();
		ImGuiMCP::TextUnformatted(a_label);
		ImGuiMCP::SameLine(0.0f, 0.0f);
		ImGuiMCP::SetCursorPosX(startX + labelWidth);

		return "##"s + a_label;
	}

	// One key image. A null texture is a file that isn't there, remembered so a missing image is
	// looked for once rather than every frame.
	struct KeyIcon
	{
		ImGuiMCP::ImTextureID texture = nullptr;
		float aspect = 1.0f;
	};

	// The framework's LoadTexture never reports the size of a PNG, so it is read from the IHDR chunk at byte 16.
	bool GetPngAspect(const std::string& a_path, float& a_aspect)
	{
		std::ifstream file(a_path, std::ios::binary);
		if (!file)
		{
			return false;
		}

		std::uint8_t header[24]{};
		if (!file.read(reinterpret_cast<char*>(header), sizeof(header)))
		{
			return false;
		}

		static constexpr std::uint8_t SIGNATURE[8] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
		if (std::memcmp(header, SIGNATURE, sizeof(SIGNATURE)) != 0)
		{
			return false;
		}

		std::uint32_t width = (header[16] << 24) | (header[17] << 16) | (header[18] << 8) | header[19];
		std::uint32_t height = (header[20] << 24) | (header[21] << 16) | (header[22] << 8) | header[23];

		if (width == 0 || height == 0)
		{
			return false;
		}

		a_aspect = static_cast<float>(width) / static_cast<float>(height);
		return true;
	}

	const KeyIcon& GetKeyIcon(const char* a_iconName)
	{
		static std::map<std::string, KeyIcon> icons;

		std::map<std::string, KeyIcon>::iterator it = icons.find(a_iconName);
		if (it != icons.end())
		{
			return it->second;
		}

		// The images belong to the "ImGui Icons" asset mod, which F.U.C.K already requires.
		std::string path = std::format("Data/Interface/ImGuiIcons/Icons/{}.png", a_iconName);

		KeyIcon icon;
		if (GetPngAspect(path, icon.aspect))
		{
			icon.texture = SKSEMenuFramework::LoadTexture(path);
		}

		if (!icon.texture)
		{
			SKSE::log::info("No key image at {}, its code will be drawn instead.", path);
		}

		return icons.emplace(a_iconName, icon).first->second;
	}

	// The loaded image of a_binding, or null when it has none to draw.
	const KeyIcon* FindKeyIcon(ModConfigUI::ButtonBinding a_binding)
	{
		const char* iconName = ModConfigUI::Internal::GetButtonIconName(a_binding);
		if (!iconName)
		{
			return nullptr;
		}

		const KeyIcon& icon = GetKeyIcon(iconName);
		return icon.texture ? &icon : nullptr;
	}

	// One row tall, and as wide as the image needs to keep its shape at that height.
	ImGuiMCP::ImVec2 GetKeyIconSize(const KeyIcon& a_icon)
	{
		float height = ImGuiMCP::GetFrameHeight();
		return ImGuiMCP::ImVec2{ height * a_icon.aspect, height };
	}

	// a_binding where the cursor is, as its image or, lacking one, as text.
	void DrawButtonImage(ModConfigUI::ButtonBinding a_binding)
	{
		const KeyIcon* icon = FindKeyIcon(a_binding);
		if (!icon)
		{
			ImGuiMCP::AlignTextToFramePadding();
			ImGuiMCP::TextUnformatted(ModConfigUI::Internal::GetButtonText(a_binding).c_str());
			return;
		}

		ImGuiMCP::Image(icon->texture, GetKeyIconSize(*icon));
	}

	// Tooltip for the item drawn just before, shown as soon as it is hovered. SetItemTooltip would wait for
	// the hover delay of the style first, F.U.C.K shows its tooltips right away and both menus should behave the same.
	bool IsItemHoveredForTooltip()
	{
		return ImGuiMCP::IsItemHovered(ImGuiMCP::ImGuiHoveredFlags_AllowWhenDisabled | ImGuiMCP::ImGuiHoveredFlags_DelayNone);
	}

	void SetInstantTooltip(const char* a_text)
	{
		if (IsItemHoveredForTooltip())
		{
			ImGuiMCP::SetTooltip("%s", a_text);
		}
	}

	// As SetInstantTooltip, with a_footer as a last line in the footer colour. Either may be null.
	void SetInstantTooltip(const char* a_text, const char* a_footer)
	{
		if (!a_footer)
		{
			if (a_text)
			{
				SetInstantTooltip(a_text);
			}
			return;
		}

		if (!IsItemHoveredForTooltip() || !ImGuiMCP::BeginTooltip())
		{
			return;
		}

		if (a_text)
		{
			ImGuiMCP::TextUnformatted(a_text);
		}
		ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, FOOTER_COLOR);
		ImGuiMCP::TextUnformatted(a_footer);
		ImGuiMCP::PopStyleColor();

		ImGuiMCP::EndTooltip();
	}

	// As SetInstantTooltip, with the footer naming a_default by the image of its key.
	void SetInstantTooltip(const char* a_text, ModConfigUI::ButtonBinding a_default)
	{
		if (!IsItemHoveredForTooltip() || !ImGuiMCP::BeginTooltip())
		{
			return;
		}

		if (a_text)
		{
			ImGuiMCP::TextUnformatted(a_text);
		}

		std::string before;
		std::string after;
		ModConfigUI::Internal::GetDefaultLineParts(before, after);

		ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, FOOTER_COLOR);

		// The text either side is lowered to the middle of the image, which is a whole row tall.
		ImGuiMCP::AlignTextToFramePadding();
		ImGuiMCP::TextUnformatted(before.c_str());
		ImGuiMCP::SameLine(0.0f, 0.0f);
		DrawButtonImage(a_default);
		if (!after.empty())
		{
			ImGuiMCP::SameLine(0.0f, 0.0f);
			ImGuiMCP::AlignTextToFramePadding();
			ImGuiMCP::TextUnformatted(after.c_str());
		}

		ImGuiMCP::PopStyleColor();

		ImGuiMCP::EndTooltip();
	}

	class SKSEMenuFrameworkRenderer : public ModConfigUI::Renderer
	{
	private:
		// Set between BeginColumns and EndColumns, where checkboxes are pushed to the right edge of their column.
		bool m_inColumns = false;

		// Whether NextColumn was already called since BeginColumns, so the next call has a margin to skip.
		bool m_columnStarted = false;

	public:
		void SeparatorText(const char* a_label) override { ImGuiMCP::SeparatorText(a_label); }
		void Separator() override { ImGuiMCP::Separator(); }
		void TextWrapped(const char* a_text) override { ImGuiMCP::TextWrapped("%s", a_text); }

		void TextMuted(const char* a_text) override
		{
			ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, GetMutedTextColor());
			ImGuiMCP::Text("%s", a_text);
			ImGuiMCP::PopStyleColor();
		}

		void TextWrappedMuted(const char* a_text) override
		{
			ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, GetMutedTextColor());
			ImGuiMCP::TextWrapped("%s", a_text);
			ImGuiMCP::PopStyleColor();
		}

		void TextDisabled(const char* a_text) override { ImGuiMCP::TextDisabled("%s", a_text); }

		void LabeledText(const char* a_label, const char* a_value) override
		{
			ImGuiMCP::BeginGroup();
			DrawRowLabel(a_label);
			ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, GetMutedTextColor());
			ImGuiMCP::TextUnformatted(a_value);
			ImGuiMCP::PopStyleColor();
			ImGuiMCP::EndGroup();
		}
		void Text(const char* a_text) override { ImGuiMCP::TextUnformatted(a_text); }
		void ButtonImage(ModConfigUI::ButtonBinding a_binding) override { DrawButtonImage(a_binding); }

		void Spacing() override { ImGuiMCP::Spacing(); }
		void SameLine() override { ImGuiMCP::SameLine(); }

		void BeginDisabled(bool a_disabled) override { ImGuiMCP::BeginDisabled(a_disabled); }
		void EndDisabled() override { ImGuiMCP::EndDisabled(); }

		bool Button(const char* a_label) override { return ImGuiMCP::Button(a_label); }

		// The label and the control are grouped so that a tooltip covers the whole row. Inside columns the box
		// sits against the far edge of the cell, the label column of a regular row would leave it floating mid cell.
		bool Checkbox(const char* a_label, bool* a_value) override
		{
			std::string id = "##"s + a_label;

			ImGuiMCP::BeginGroup();
			if (m_inColumns)
			{
				float startX = ImGuiMCP::GetCursorPosX();
				float availableWidth = ImGuiMCP::GetContentRegionAvail().x;

				ImGuiMCP::AlignTextToFramePadding();
				ImGuiMCP::TextUnformatted(a_label);
				ImGuiMCP::SameLine(0.0f, 0.0f);
				ImGuiMCP::SetCursorPosX(startX + availableWidth - ImGuiMCP::GetFrameHeight());
			}
			else
			{
				DrawRowLabel(a_label);
			}
			bool changed = ImGuiMCP::Checkbox(id.c_str(), a_value);
			ImGuiMCP::EndGroup();
			return changed;
		}

		bool CheckboxWidget(const char* a_id, bool* a_value) override
		{
			std::string id = "##"s + a_id;
			return ImGuiMCP::Checkbox(id.c_str(), a_value);
		}

		bool InputText(const char* a_label, char* a_buffer, std::size_t a_bufferSize) override
		{
			ImGuiMCP::BeginGroup();
			std::string id = DrawRowLabel(a_label);
			ImGuiMCP::SetNextItemWidth(-1.0f);
			bool changed = ImGuiMCP::InputText(id.c_str(), a_buffer, a_bufferSize);
			ImGuiMCP::EndGroup();
			return changed;
		}

		bool InputTextWithHint(const char* a_label, const char* a_hint, char* a_buffer, std::size_t a_bufferSize) override
		{
			ImGuiMCP::BeginGroup();
			std::string id = DrawRowLabel(a_label);
			ImGuiMCP::SetNextItemWidth(-1.0f);

			// ImGui draws the hint in the disabled text colour, which some themes leave barely readable.
			ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_TextDisabled, GetMutedTextColor());
			bool changed = ImGuiMCP::InputTextWithHint(id.c_str(), a_hint, a_buffer, a_bufferSize);
			ImGuiMCP::PopStyleColor();

			ImGuiMCP::EndGroup();
			return changed;
		}

		bool Combo(const char* a_label, std::int32_t* a_currentItem, const char* const* a_items, std::int32_t a_itemCount) override
		{
			ImGuiMCP::BeginGroup();
			std::string id = DrawRowLabel(a_label);
			ImGuiMCP::SetNextItemWidth(-1.0f);
			bool changed = ImGuiMCP::Combo(id.c_str(), a_currentItem, a_items, a_itemCount);
			ImGuiMCP::EndGroup();
			return changed;
		}

		bool SliderFloat(const char* a_label, float* a_value, float a_min, float a_max, const char* a_format) override
		{
			ImGuiMCP::BeginGroup();
			std::string id = DrawRowLabel(a_label);
			ImGuiMCP::SetNextItemWidth(-1.0f);
			ImGuiMCP::SliderFloat(id.c_str(), a_value, a_min, a_max, a_format);

			// Has to be asked before the group closes, afterwards it refers to the group itself.
			bool finished = ImGuiMCP::IsItemDeactivatedAfterEdit();

			ImGuiMCP::EndGroup();
			return finished;
		}

		bool SliderInt(const char* a_label, std::int32_t* a_value, std::int32_t a_min, std::int32_t a_max, const char* a_format) override
		{
			ImGuiMCP::BeginGroup();
			std::string id = DrawRowLabel(a_label);
			ImGuiMCP::SetNextItemWidth(-1.0f);
			ImGuiMCP::SliderInt(id.c_str(), a_value, a_min, a_max, a_format);

			// Has to be asked before the group closes, afterwards it refers to the group itself.
			bool finished = ImGuiMCP::IsItemDeactivatedAfterEdit();

			ImGuiMCP::EndGroup();
			return finished;
		}

		bool BeginTable(const char* a_id, const char* const* a_columnHeaders, std::int32_t a_columnCount, const float* a_columnWidths) override
		{
			constexpr ImGuiMCP::ImGuiTableFlags FLAGS = ImGuiMCP::ImGuiTableFlags_SizingStretchProp | ImGuiMCP::ImGuiTableFlags_RowBg;

			if (!ImGuiMCP::BeginTable(a_id, a_columnCount, FLAGS))
			{
				return false;
			}

			for (std::int32_t i = 0; i < a_columnCount; ++i)
			{
				float width = a_columnWidths ? a_columnWidths[i] : 1.0f;
				ImGuiMCP::TableSetupColumn(a_columnHeaders[i], ImGuiMCP::ImGuiTableColumnFlags_WidthStretch, width);
			}

			ImGuiMCP::TableHeadersRow();
			return true;
		}

		void EndTable() override { ImGuiMCP::EndTable(); }

		void TableNextRow() override { ImGuiMCP::TableNextRow(); }
		void TableNextColumn() override
		{
			ImGuiMCP::TableNextColumn();
			ImGuiMCP::AlignTextToFramePadding();
		}

		// Every column is followed by a narrow empty one, which is the margin on its right.
		bool BeginColumns(const char* a_id, std::int32_t a_columnCount) override
		{
			if (!ImGuiMCP::BeginTable(a_id, a_columnCount * 2, ImGuiMCP::ImGuiTableFlags_SizingStretchSame))
			{
				return false;
			}

			float margin = ImGuiMCP::GetFrameHeight();
			for (std::int32_t i = 0; i < a_columnCount; ++i)
			{
				ImGuiMCP::TableSetupColumn(nullptr, ImGuiMCP::ImGuiTableColumnFlags_WidthStretch, 1.0f);
				ImGuiMCP::TableSetupColumn(nullptr, ImGuiMCP::ImGuiTableColumnFlags_WidthFixed, margin);
			}

			m_inColumns = true;
			m_columnStarted = false;
			return true;
		}

		void NextColumn() override
		{
			if (m_columnStarted)
			{
				ImGuiMCP::TableNextColumn();
			}
			ImGuiMCP::TableNextColumn();
			m_columnStarted = true;
		}

		void EndColumns() override
		{
			ImGuiMCP::EndTable();
			m_inColumns = false;
		}

		void PushID(std::int32_t a_id) override { ImGuiMCP::PushID(a_id); }
		void PopID() override { ImGuiMCP::PopID(); }

		void ItemTooltip(const char* a_text) override { SetInstantTooltip(a_text); }
		void ItemTooltip(const char* a_text, const char* a_footer) override { SetInstantTooltip(a_text, a_footer); }
		void ItemTooltip(const char* a_text, ModConfigUI::ButtonBinding a_default) override { SetInstantTooltip(a_text, a_default); }

		void OpenPopup(const char* a_id) override { ImGuiMCP::OpenPopup(a_id); }
		// Left to size itself, a modal fits its content, but wrapped text wraps at the width the window
		// already has: a new window starts out tiny, every word gets a line of its own and the height is
		// then clamped to the viewport. So the width is fixed and only the height follows the content.
		bool BeginPopupModal(const char* a_id) override
		{
			// 22 font sizes wide, but never more than 90% of the screen.
			ImGuiMCP::ImGuiViewport* viewport = ImGuiMCP::GetMainViewport();
			float width = std::fmin(ImGuiMCP::GetFontSize() * 22.0f, viewport->Size.x * 0.9f);

			// A zero height fits that axis to the content again every frame.
			ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{ width, 0.0f }, ImGuiMCP::ImGuiCond_Always);
			ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImGuiViewportManager::GetCenter(viewport), ImGuiMCP::ImGuiCond_Appearing, ImGuiMCP::ImVec2{ 0.5f, 0.5f });

			return ImGuiMCP::BeginPopupModal(a_id);
		}
		void EndPopup() override { ImGuiMCP::EndPopup(); }
		void CloseCurrentPopup() override { ImGuiMCP::CloseCurrentPopup(); }

		std::string LabelColumn(const char* a_label) override { return DrawRowLabel(a_label); }

		// Plain ImGui has no hotkey widget, so the row is built here out of the same key images F.U.C.K
		// draws, from the asset mod the two of them share. A key with no image of its own falls back to
		// the text button the base implementation draws.
		bool BindWidget(const char* a_label, ModConfigUI::ButtonBinding a_binding, bool a_listening) override
		{
			const KeyIcon* icon = FindKeyIcon(a_binding);
			if (!icon)
			{
				return ModConfigUI::Renderer::BindWidget(a_label, a_binding, a_listening);
			}

			ImGuiMCP::BeginGroup();
			std::string id = DrawRowLabel(a_label);

			ImGuiMCP::ImVec2 size = GetKeyIconSize(*icon);

			// ImageButton would frame the image and pad it, and neither can be styled all the way off:
			// the border survives a transparent fill. F.U.C.K draws no widget at all, just a hit box
			// and the image, so that is what happens here too. The image is the button.
			bool clicked = ImGuiMCP::InvisibleButton(id.c_str(), size);

			// How F.U.C.K tints a key image, so a binder looks the same in both menus: its colour carries the state.
			// Idle is white dimmed to 65%, hovered is full white, and a binder waiting for a press pulses gold,
			// a full sweep about every 1.3 seconds.
			ImGuiMCP::ImVec4 tint{ 1.0f, 1.0f, 1.0f, 0.65f };
			if (a_listening)
			{
				tint = ImGuiMCP::ImVec4{ 1.0f, 0.8f, 0.2f, 0.4f + 0.6f * std::fabs(std::sin(static_cast<float>(ImGuiMCP::GetTime()) * 5.0f)) };
			}
			else if (ImGuiMCP::IsItemHovered())
			{
				tint.w = 1.0f;
			}

			ImGuiMCP::ImVec2 min = ImGuiMCP::GetItemRectMin();
			ImGuiMCP::ImVec2 max = ImGuiMCP::GetItemRectMax();

			ImGuiMCP::ImDrawList* drawList = ImGuiMCP::GetWindowDrawList();
			ImGuiMCP::ImDrawListManager::AddImage(drawList, icon->texture, min, max, ImGuiMCP::ImVec2{ 0.0f, 0.0f }, ImGuiMCP::ImVec2{ 1.0f, 1.0f }, ImGuiMCP::ColorConvertFloat4ToU32(tint));

			ImGuiMCP::EndGroup();

			return clicked;
		}
	};

	// The framework acts on Escape, its configurable toggle keys and B by itself, so no keyboard or gamepad
	// button reaches it while a binder listens. The mouse still does, so the binder can be clicked to give up.
	bool IsKeyboardOrGamepadButton(const RE::InputEvent* a_event)
	{
		if (a_event->eventType != RE::INPUT_EVENT_TYPE::kButton)
		{
			return false;
		}

		RE::INPUT_DEVICE device = a_event->GetDevice();
		return device == RE::INPUT_DEVICE::kKeyboard || device == RE::INPUT_DEVICE::kGamepad;
	}

	// The framework's input callbacks miss its own keys and skip everything while an ImGui item is active, so
	// the input queue is hooked instead, on top of the framework's hook, to see the events before it does.
	struct SendEventsHook
	{
		static void Thunk(RE::BSTEventSource<RE::InputEvent*>* a_source, RE::InputEvent* const* a_events)
		{
			if (a_events && ModConfigUI::Internal::IsCapturing())
			{
				RE::InputEvent** head = const_cast<RE::InputEvent**>(a_events);
				RE::InputEvent* previous = nullptr;

				for (RE::InputEvent* event = *head; event;)
				{
					RE::InputEvent* next = event->next;

					// Only while armed: once cancelled, the capture takes nothing but the Escape release it is
					// waiting for, so a release that never comes can't leave the keyboard dead.
					bool listening = ModConfigUI::Internal::IsListening();
					bool consumed = ModConfigUI::Internal::OnSingleInputEvent(event);

					if (consumed || (listening && IsKeyboardOrGamepadButton(event)))
					{
						if (previous)
						{
							previous->next = next;
						}
						else
						{
							*head = next;
						}
					}
					else
					{
						previous = event;
					}

					event = next;
				}
			}

			m_original(a_source, a_events);
		}

		static void Install()
		{
			// BSInputDeviceManager::PollInputDevices, the call to BSInputDeviceManager::SendEvents
			REL::Relocation<std::uintptr_t> target{ REL::ID{ 68617 }, 0x7B };

			// Its own, so a mod using this library doesn't have to budget for it in the shared trampoline.
			static SKSE::Trampoline trampoline{ "ModConfigUI" };
			trampoline.create(14);
			m_original = trampoline.write_call<5>(target.address(), &Thunk);
		}

		static inline REL::Relocation<decltype(&Thunk)> m_original;
	};

	SKSEMenuFrameworkRenderer renderer;

	// The framework takes a plain function pointer per section item, so one static callback has to
	// exist for every page a mod could register. They only differ in the page index they forward.
	template <std::size_t Index>
	void __stdcall PageCallback()
	{
		ModConfigUI::Internal::DrawPage(Index, renderer);
	}

	template <std::size_t... Indices>
	constexpr std::array<SKSEMenuFramework::Model::RenderFunction, sizeof...(Indices)> MakePageCallbacks(std::index_sequence<Indices...>)
	{
		return { &PageCallback<Indices>... };
	}

	constexpr auto PAGE_CALLBACKS = MakePageCallbacks(std::make_index_sequence<ModConfigUI::MAX_PAGES + 1>{});
}

namespace ModConfigUI::Internal
{
	void InstallSKSEMenuFramework()
	{
		if (!SKSEMenuFramework::IsInstalled())
		{
			SKSE::log::info("SKSE Menu Framework not found, skipping menu registration.");
			return;
		}

		SKSEMenuFramework::SetSection(GetDisplayName());

		// Runs after every plugin has loaded, so the framework's own hook on the same call is already in place.
		SendEventsHook::Install();

		for (std::size_t i = 0; i < GetPageCount(); ++i)
		{
			SKSEMenuFramework::AddSectionItem(GetPageName(i), PAGE_CALLBACKS[i]);
		}

		SKSE::log::info("Registered menu with SKSE Menu Framework.");
	}
}
