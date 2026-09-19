#include "Internal.h"

namespace
{
	using ModConfigUI::BindFilter;
	using ModConfigUI::ButtonBinding;
	using ModConfigUI::Internal::Capture::Result;

	// Shared between the render thread, which arms and polls, and the input thread, which answers. Atomics only, no locks.
	std::atomic<bool> armed{ false };
	std::atomic<std::uint32_t> ownerId{ 0 };
	std::atomic<std::uint32_t> acceptedDevices{ static_cast<std::uint32_t>(BindFilter::kNone) };
	std::atomic<Result> finishedResult{ Result::kNone };
	std::atomic<std::uint32_t> capturedKey{ ButtonBinding::NONE };

	// A binding is only taken on the release of a button pressed after arming, so the click that armed it isn't the answer.
	std::atomic<std::uint32_t> pendingKey{ ButtonBinding::NONE };

	// An armed capture nobody polls anymore would eat input forever, so the input side gives up on it after a timeout.
	std::atomic<std::chrono::steady_clock::rep> lastPollTime{ 0 };

	// The release of the press that cancelled, swallowed too so the menu doesn't close on half a press.
	std::atomic<std::uint32_t> swallowReleaseKey{ ButtonBinding::NONE };

	std::chrono::steady_clock::rep Now()
	{
		return std::chrono::steady_clock::now().time_since_epoch().count();
	}

	void Disarm(Result a_result)
	{
		finishedResult = a_result;
		pendingKey = ButtonBinding::NONE;
		armed = false;
	}

	// Returns true when the event was taken, so the game doesn't also act on the button being bound.
	bool ProcessButton(const RE::ButtonEvent* a_event)
	{
		ButtonBinding binding = ButtonBinding::FromEvent(a_event->device.get(), a_event->idCode);

		BindFilter filter = static_cast<BindFilter>(acceptedDevices.load());
		bool isAllowed = false;
		switch (binding.GetDevice())
		{
		case RE::INPUT_DEVICE::kKeyboard:
			isAllowed = filter & BindFilter::kKeyboard;
			break;
		case RE::INPUT_DEVICE::kMouse:
			isAllowed = filter & BindFilter::kMouse;
			break;
		case RE::INPUT_DEVICE::kGamepad:
			isAllowed = filter & BindFilter::kGamepad;
			break;
		default:
			break;
		}

		// Escape, or a button the binder can't take, gives up without a change.
		bool isEscape = a_event->device.get() == RE::INPUT_DEVICE::kKeyboard && a_event->idCode == 0x01;
		if (isEscape || !binding.IsSet() || !isAllowed)
		{
			// Only a fresh press counts, a release or repeat is of a button already down when arming.
			if (!a_event->IsDown())
			{
				return isEscape;
			}

			swallowReleaseKey = binding.key;
			Disarm(Result::kCancelled);
			return true;
		}

		if (a_event->IsDown())
		{
			pendingKey = binding.key;
			return true;
		}

		if (a_event->IsUp())
		{
			// Anything released that we didn't see pressed was already held when the capture was armed.
			if (pendingKey.load() == binding.key)
			{
				capturedKey = binding.key;
				Disarm(Result::kCaptured);
			}
			return true;
		}

		// A hold repeat of the button on its way to being bound.
		return true;
	}
}

namespace ModConfigUI::Internal::Capture
{
	void Begin(std::uint32_t a_owner, BindFilter a_filter)
	{
		ownerId = a_owner;
		acceptedDevices = static_cast<std::uint32_t>(a_filter);
		finishedResult = Result::kNone;
		capturedKey = ButtonBinding::NONE;
		pendingKey = ButtonBinding::NONE;
		swallowReleaseKey = ButtonBinding::NONE;
		lastPollTime = Now();
		armed = true;
	}

	bool IsArmed(std::uint32_t a_owner)
	{
		return armed.load() && ownerId.load() == a_owner;
	}

	Result Poll(std::uint32_t a_owner, ButtonBinding& a_captured)
	{
		if (ownerId.load() != a_owner)
		{
			return Result::kNone;
		}

		if (armed.load())
		{
			lastPollTime = Now();
			return Result::kNone;
		}

		// The result is handed out once, the next poll finds nothing left to report.
		Result result = finishedResult.exchange(Result::kNone);
		if (result == Result::kCaptured)
		{
			a_captured = ButtonBinding{ capturedKey.load() };
		}
		return result;
	}

	void Cancel(std::uint32_t a_owner)
	{
		if (ownerId.load() == a_owner)
		{
			Disarm(Result::kCancelled);
		}
	}
}

namespace ModConfigUI::Internal
{
	bool OnSingleInputEvent(const RE::InputEvent* a_event)
	{
		if (!a_event)
		{
			return false;
		}

		if (!armed.load())
		{
			std::uint32_t swallow = swallowReleaseKey.load();
			if (swallow == ButtonBinding::NONE || a_event->eventType != RE::INPUT_EVENT_TYPE::kButton || !a_event->HasIDCode())
			{
				return false;
			}

			const RE::ButtonEvent* button = static_cast<const RE::ButtonEvent*>(a_event);
			if (ButtonBinding::FromEvent(button->device.get(), button->idCode).key != swallow)
			{
				return false;
			}

			// Taken until the release, a hold repeat included.
			if (button->IsUp())
			{
				swallowReleaseKey = ButtonBinding::NONE;
			}
			return true;
		}

		// Whoever armed this has stopped drawing. Give the input back to the game.
		if (std::chrono::steady_clock::duration{ Now() - lastPollTime.load() } > std::chrono::seconds{ 5 })
		{
			Disarm(Result::kCancelled);
			return false;
		}

		if (a_event->eventType != RE::INPUT_EVENT_TYPE::kButton || !a_event->HasIDCode())
		{
			return false;
		}

		return ProcessButton(static_cast<const RE::ButtonEvent*>(a_event));
	}

	bool OnInputEvent(const RE::InputEvent* const* a_event)
	{
		if (!a_event)
		{
			return false;
		}

		bool consumed = false;

		for (const RE::InputEvent* event = *a_event; event; event = event->next)
		{
			bool wasArmed = armed.load();

			if (OnSingleInputEvent(event))
			{
				consumed = true;
			}

			// A capture that just finished must not also take the rest of the chain.
			if (wasArmed && !armed.load())
			{
				break;
			}
		}

		return consumed;
	}

	bool IsCapturing()
	{
		return armed.load(std::memory_order_relaxed) || swallowReleaseKey.load(std::memory_order_relaxed) != ButtonBinding::NONE;
	}

	bool IsListening()
	{
		return armed.load(std::memory_order_relaxed);
	}
}
