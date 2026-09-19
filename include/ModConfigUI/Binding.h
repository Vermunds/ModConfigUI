#pragma once

namespace ModConfigUI
{
	// A single button, held in the unified key space SKSE::InputMap defines: keyboard scan codes take
	// 0-255, mouse buttons start at 256, the mouse wheel at 264 and gamepad buttons at 266. Both menu
	// frameworks already speak it, and it is what mods have always written to their INI files, so a
	// binding can be stored and read back without a representation of its own.
	struct ButtonBinding
	{
		static constexpr std::uint32_t NONE = static_cast<std::uint32_t>(-1);

		std::uint32_t key = NONE;

		bool IsSet() const { return key != NONE; }

		// The device and id an input event carries. The unified space is one number and numbers the
		// gamepad buttons in order, an event splits the same button across two fields and identifies a
		// gamepad button by its XInput mask instead.
		RE::INPUT_DEVICE GetDevice() const;
		std::uint32_t GetDeviceId() const;

		static ButtonBinding FromEvent(RE::INPUT_DEVICE a_device, std::uint32_t a_idCode);

		friend bool operator==(const ButtonBinding& a_lhs, const ButtonBinding& a_rhs) { return a_lhs.key == a_rhs.key; }
	};

	// The devices a binder accepts. A press from any other device is left for the game to deal with.
	enum class BindFilter : std::uint32_t
	{
		kNone = 0,
		kKeyboard = 1 << 0,
		kMouse = 1 << 1,
		kGamepad = 1 << 2,
		kAny = kKeyboard | kMouse | kGamepad
	};

	constexpr BindFilter operator|(BindFilter a_lhs, BindFilter a_rhs) { return static_cast<BindFilter>(static_cast<std::uint32_t>(a_lhs) | static_cast<std::uint32_t>(a_rhs)); }
	constexpr bool operator&(BindFilter a_lhs, BindFilter a_rhs) { return (static_cast<std::uint32_t>(a_lhs) & static_cast<std::uint32_t>(a_rhs)) != 0; }

	// The name of the device the binding belongs to, for a menu that lists the two apart.
	const char* GetDeviceName(RE::INPUT_DEVICE a_device);
}
