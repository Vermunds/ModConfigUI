#pragma once

#include "ModConfigUI/ModConfigUI.h"
#include "ModConfigUI/Localization.h"

namespace ModConfigUI::Internal
{
	const ModInfo& GetModInfo();

	// The mod's name with its translation resolved, the display name may be a translation key.
	const char* GetDisplayName();

	// Page 0 is always the built in "Mod Info" page, the mod's own pages follow.
	std::size_t GetPageCount();
	const char* GetPageName(std::size_t a_index);
	void DrawPage(std::size_t a_index, Renderer& a_renderer);

	RestoreDefaultsFunction GetRestoreDefaults();

	void DrawModInfoPage(Renderer& a_renderer);

	void InstallSKSEMenuFramework();
	void InstallFUCK();

	// How far the muted label colour sits between the theme's regular (0.0) and disabled (1.0) text colour.
	inline constexpr float MUTED_TEXT_BLEND = 0.6f;

	// RGBA of the last tooltip line that names a setting's default. The gold a binder flashes while it
	// listens, so the menu keeps to a single accent colour.
	inline constexpr float TOOLTIP_FOOTER_COLOR[4] = { 1.0f, 0.8f, 0.2f, 1.0f };

	// The file name, without the folder or the extension, of the image for a_binding.
	// Null when the binding has no image of its own, which is the caller's cue to draw GetButtonText.
	const char* GetButtonIconName(ButtonBinding a_binding);

	// What is drawn for a binding that has no image: "Not bound", or the key code of a button the
	// icon set has no picture of, such as the mouse wheel.
	std::string GetButtonText(ButtonBinding a_binding);

	// The two halves of the "Default: ..." line around its value, for a default that is drawn as an
	// image rather than written out. Either may be empty.
	void GetDefaultLineParts(std::string& a_before, std::string& a_after);

	// Fed by every backend from its own input hook. Returns true when the event was taken, so the game
	// doesn't also act on the button.
	bool OnInputEvent(const RE::InputEvent* const* a_event);

	// As above for one event, its next link is ignored.
	bool OnSingleInputEvent(const RE::InputEvent* a_event);

	// Listening, or still waiting for the release of the button that cancelled. Cheap, checked before any event.
	bool IsCapturing();

	// Listening right now, the release wait not included.
	bool IsListening();

	// Listens for one button press on behalf of the widget identified by a_owner. Only driven by Renderer::BindButton.
	namespace Capture
	{
		enum class Result
		{
			kNone,
			kCaptured,
			kCancelled
		};

		void Begin(std::uint32_t a_owner, BindFilter a_filter);
		bool IsArmed(std::uint32_t a_owner);

		// Called every frame while a_owner is armed. A capture that stops being polled is given up on.
		Result Poll(std::uint32_t a_owner, ButtonBinding& a_captured);

		void Cancel(std::uint32_t a_owner);
	}
}
