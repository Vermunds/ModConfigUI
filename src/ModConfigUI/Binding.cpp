#include "Internal.h"

namespace
{
	using ModConfigUI::ButtonBinding;

	namespace Unified = SKSE::InputMap;

	constexpr std::uint32_t MOUSE_OFFSET = Unified::kMacro_MouseButtonOffset;
	constexpr std::uint32_t GAMEPAD_OFFSET = Unified::kMacro_GamepadOffset;
	constexpr std::uint32_t MAX_KEY = Unified::kMaxMacros;

	const char* GetKeyboardIconName(std::uint32_t a_scanCode)
	{
		switch (a_scanCode)
		{
		case 0x01:
			return "Esc";
		case 0x02:
			return "1";
		case 0x03:
			return "2";
		case 0x04:
			return "3";
		case 0x05:
			return "4";
		case 0x06:
			return "5";
		case 0x07:
			return "6";
		case 0x08:
			return "7";
		case 0x09:
			return "8";
		case 0x0A:
			return "9";
		case 0x0B:
			return "0";
		case 0x0C:
			return "Hyphen";
		case 0x0D:
			return "Equal";
		case 0x0E:
			return "Backspace";
		case 0x0F:
			return "Tab";
		case 0x10:
			return "Q";
		case 0x11:
			return "W";
		case 0x12:
			return "E";
		case 0x13:
			return "R";
		case 0x14:
			return "T";
		case 0x15:
			return "Y";
		case 0x16:
			return "U";
		case 0x17:
			return "I";
		case 0x18:
			return "O";
		case 0x19:
			return "P";
		case 0x1A:
			return "Bracketleft";
		case 0x1B:
			return "Bracketright";
		case 0x1C:
			return "Enter";
		case 0x1D:
			return "L-Ctrl";
		case 0x1E:
			return "A";
		case 0x1F:
			return "S";
		case 0x20:
			return "D";
		case 0x21:
			return "F";
		case 0x22:
			return "G";
		case 0x23:
			return "H";
		case 0x24:
			return "J";
		case 0x25:
			return "K";
		case 0x26:
			return "L";
		case 0x27:
			return "Semicolon";
		case 0x28:
			return "Quotesingle";
		case 0x29:
			return "Tilde";
		case 0x2A:
			return "L-Shift";
		case 0x2B:
			return "Backslash";
		case 0x2C:
			return "Z";
		case 0x2D:
			return "X";
		case 0x2E:
			return "C";
		case 0x2F:
			return "V";
		case 0x30:
			return "B";
		case 0x31:
			return "N";
		case 0x32:
			return "M";
		case 0x33:
			return "Comma";
		case 0x34:
			return "Period";
		case 0x35:
			return "Slash";
		case 0x36:
			return "R-Shift";
		case 0x37:
			return "NumPadMult";
		case 0x38:
			return "L-Alt";
		case 0x39:
			return "Space";
		case 0x3A:
			return "CapsLock";
		case 0x3B:
			return "F1";
		case 0x3C:
			return "F2";
		case 0x3D:
			return "F3";
		case 0x3E:
			return "F4";
		case 0x3F:
			return "F5";
		case 0x40:
			return "F6";
		case 0x41:
			return "F7";
		case 0x42:
			return "F8";
		case 0x43:
			return "F9";
		case 0x44:
			return "F10";
		case 0x45:
			return "NumLock";
		case 0x46:
			return "ScrollLock";
		case 0x47:
			return "Keypad7";
		case 0x48:
			return "Keypad8";
		case 0x49:
			return "NumPad9";
		case 0x4A:
			return "NumPadMinus";
		case 0x4B:
			return "Keypad4";
		case 0x4C:
			return "Keypad5";
		case 0x4D:
			return "Keypad6";
		case 0x4E:
			return "NumPadPlus";
		case 0x4F:
			return "Keypad1";
		case 0x50:
			return "Keypad2";
		case 0x51:
			return "Keypad3";
		case 0x52:
			return "NumPad0";
		case 0x53:
			return "NumPadDec";
		case 0x57:
			return "F11";
		case 0x58:
			return "F12";
		case 0x9C:
			return "KeypadEnter";
		case 0x9D:
			return "R-Ctrl";
		case 0xB5:
			return "NumPadDivide";
		case 0xB7:
			return "PrintScreen";
		case 0xB8:
			return "R-Alt";
		case 0xC5:
			return "Pause";
		case 0xC7:
			return "Home";
		case 0xC8:
			return "Up";
		case 0xC9:
			return "PgUp";
		case 0xCB:
			return "Left";
		case 0xCD:
			return "Right";
		case 0xCF:
			return "End";
		case 0xD0:
			return "Down";
		case 0xD1:
			return "PgDn";
		case 0xD2:
			return "Insert";
		case 0xD3:
			return "Delete";
		default:
			return nullptr;
		}
	}

	const char* GetGamepadIconName(std::uint32_t a_key)
	{
		switch (a_key)
		{
		case Unified::kGamepadButtonOffset_DPAD_UP:
			return "Up";
		case Unified::kGamepadButtonOffset_DPAD_DOWN:
			return "Down";
		case Unified::kGamepadButtonOffset_DPAD_LEFT:
			return "Left";
		case Unified::kGamepadButtonOffset_DPAD_RIGHT:
			return "Right";
		case Unified::kGamepadButtonOffset_START:
			return "360_Start";
		case Unified::kGamepadButtonOffset_BACK:
			return "360_Back";
		case Unified::kGamepadButtonOffset_LEFT_THUMB:
			return "360_LS";
		case Unified::kGamepadButtonOffset_RIGHT_THUMB:
			return "360_RS";
		case Unified::kGamepadButtonOffset_LEFT_SHOULDER:
			return "360_LB";
		case Unified::kGamepadButtonOffset_RIGHT_SHOULDER:
			return "360_RB";
		case Unified::kGamepadButtonOffset_A:
			return "360_A";
		case Unified::kGamepadButtonOffset_B:
			return "360_B";
		case Unified::kGamepadButtonOffset_X:
			return "360_X";
		case Unified::kGamepadButtonOffset_Y:
			return "360_Y";
		case Unified::kGamepadButtonOffset_LT:
			return "360_LT";
		case Unified::kGamepadButtonOffset_RT:
			return "360_RT";
		default:
			return nullptr;
		}
	}
}

namespace ModConfigUI
{
	RE::INPUT_DEVICE ButtonBinding::GetDevice() const
	{
		if (key < MOUSE_OFFSET)
		{
			return RE::INPUT_DEVICE::kKeyboard;
		}
		if (key < GAMEPAD_OFFSET)
		{
			return RE::INPUT_DEVICE::kMouse;
		}
		if (key < MAX_KEY)
		{
			return RE::INPUT_DEVICE::kGamepad;
		}
		return RE::INPUT_DEVICE::kNone;
	}

	std::uint32_t ButtonBinding::GetDeviceId() const
	{
		switch (GetDevice())
		{
		case RE::INPUT_DEVICE::kKeyboard:
			return key;
		case RE::INPUT_DEVICE::kMouse:
			return key - MOUSE_OFFSET;
		case RE::INPUT_DEVICE::kGamepad:
			return Unified::GamepadKeycodeToMask(key);
		default:
			return 0;
		}
	}

	ButtonBinding ButtonBinding::FromEvent(RE::INPUT_DEVICE a_device, std::uint32_t a_idCode)
	{
		switch (a_device)
		{
		case RE::INPUT_DEVICE::kKeyboard:
			return a_idCode < MOUSE_OFFSET ? ButtonBinding{ a_idCode } : ButtonBinding{};
		case RE::INPUT_DEVICE::kMouse:
			return a_idCode < GAMEPAD_OFFSET - MOUSE_OFFSET ? ButtonBinding{ MOUSE_OFFSET + a_idCode } : ButtonBinding{};
		case RE::INPUT_DEVICE::kGamepad:
			{
				std::uint32_t key = Unified::GamepadMaskToKeycode(a_idCode);
				return key < MAX_KEY ? ButtonBinding{ key } : ButtonBinding{};
			}
		default:
			return ButtonBinding{};
		}
	}

	const char* GetDeviceName(RE::INPUT_DEVICE a_device)
	{
		switch (a_device)
		{
		case RE::INPUT_DEVICE::kKeyboard:
			return Localization::Get("$ModConfigUI_Device_Keyboard");
		case RE::INPUT_DEVICE::kMouse:
			return Localization::Get("$ModConfigUI_Device_Mouse");
		case RE::INPUT_DEVICE::kGamepad:
			return Localization::Get("$ModConfigUI_Device_Gamepad");
		default:
			return Localization::Get("$ModConfigUI_Device_Unknown");
		}
	}
}

namespace ModConfigUI::Internal
{
	const char* GetButtonIconName(ButtonBinding a_binding)
	{
		if (!a_binding.IsSet())
		{
			return nullptr;
		}

		switch (a_binding.GetDevice())
		{
		case RE::INPUT_DEVICE::kKeyboard:
			return GetKeyboardIconName(a_binding.key);
		case RE::INPUT_DEVICE::kMouse:
			{
				// Mouse1 through Mouse8, the two wheel directions have no image of their own.
				std::uint32_t id = a_binding.GetDeviceId();
				if (id >= Unified::kMacro_NumMouseButtons)
				{
					return nullptr;
				}

				static constexpr const char* NAMES[Unified::kMacro_NumMouseButtons] = { "Mouse1", "Mouse2", "Mouse3", "Mouse4", "Mouse5", "Mouse6", "Mouse7", "Mouse8" };
				return NAMES[id];
			}
		case RE::INPUT_DEVICE::kGamepad:
			return GetGamepadIconName(a_binding.key);
		default:
			return nullptr;
		}
	}

	std::string GetButtonText(ButtonBinding a_binding)
	{
		if (!a_binding.IsSet())
		{
			return Localization::Get("$ModConfigUI_Button_Unbound");
		}

		return std::format("{} {}", Localization::Get("$ModConfigUI_Button_Unknown"), a_binding.key);
	}
}
