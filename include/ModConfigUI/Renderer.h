#pragma once

#include "ModConfigUI/Binding.h"

namespace ModConfigUI
{
	// Fraction of a row the label occupies. A table lining up with the widget rows uses it as its label column's weight.
	inline constexpr float LABEL_COLUMN_RATIO = 0.5f;

	// The "Default: ..." line a setting row puts at the bottom of its tooltip, for a page building one by hand.
	std::string GetDefaultLine(const char* a_value);

	// As above, for a checkbox.
	std::string GetDefaultLine(bool a_value);

	// Drawing surface shared by every supported menu framework, pages are written once against this.
	class Renderer
	{
	public:
		virtual ~Renderer() = default;

		virtual void SeparatorText(const char* a_label) = 0;
		virtual void Separator() = 0;
		virtual void TextWrapped(const char* a_text) = 0;

		// Between the regular and the disabled text colour, for static labels.
		virtual void TextMuted(const char* a_text) = 0;
		virtual void TextWrappedMuted(const char* a_text) = 0;

		// For text describing something that is unavailable.
		virtual void TextDisabled(const char* a_text) = 0;

		// Static text laid out like a widget row: label in the label column, value next to it.
		virtual void LabeledText(const char* a_label, const char* a_value) = 0;

		virtual void Text(const char* a_text) = 0;

		// The image of a_binding's key, one row tall. Written out as text when there is no image.
		virtual void ButtonImage(ButtonBinding a_binding) = 0;

		virtual void Spacing() = 0;
		virtual void SameLine() = 0;

		// Nests, and passing false draws normally, so a row can be wrapped unconditionally.
		virtual void BeginDisabled(bool a_disabled) = 0;
		virtual void EndDisabled() = 0;

		virtual bool Button(const char* a_label) = 0;
		virtual bool Checkbox(const char* a_label, bool* a_value) = 0;

		// The box on its own, for a cell of a table the page lays out itself. a_id is never drawn.
		virtual bool CheckboxWidget(const char* a_id, bool* a_value) = 0;

		virtual bool InputText(const char* a_label, char* a_buffer, std::size_t a_bufferSize) = 0;

		// a_hint is shown while the buffer is empty.
		virtual bool InputTextWithHint(const char* a_label, const char* a_hint, char* a_buffer, std::size_t a_bufferSize) = 0;

		virtual bool Combo(const char* a_label, std::int32_t* a_currentItem, const char* const* a_items, std::int32_t a_itemCount) = 0;

		// The value is written live while dragging, but true is only returned on release, so a drag saves once.
		virtual bool SliderFloat(const char* a_label, float* a_value, float a_min, float a_max, const char* a_format) = 0;
		virtual bool SliderInt(const char* a_label, std::int32_t* a_value, std::int32_t a_min, std::int32_t a_max, const char* a_format) = 0;

		// Setting rows: the widget, then a tooltip of a_tooltip (may be nullptr) with a last line naming a_default.
		bool Checkbox(const char* a_label, bool* a_value, bool a_default, const char* a_tooltip);
		bool InputText(const char* a_label, char* a_buffer, std::size_t a_bufferSize, const char* a_default, const char* a_tooltip);
		bool InputTextWithHint(const char* a_label, const char* a_hint, char* a_buffer, std::size_t a_bufferSize, const char* a_default, const char* a_tooltip);
		bool Combo(const char* a_label, std::int32_t* a_currentItem, const char* const* a_items, std::int32_t a_itemCount, std::int32_t a_default, const char* a_tooltip);
		bool SliderFloat(const char* a_label, float* a_value, float a_min, float a_max, const char* a_format, float a_default, const char* a_tooltip);
		bool SliderInt(const char* a_label, std::int32_t* a_value, std::int32_t a_min, std::int32_t a_max, const char* a_format, std::int32_t a_default, const char* a_tooltip);

		// a_columnWidths holds one stretch weight per column, nullptr stretches them evenly.
		// On false nothing may be drawn and EndTable must not be called.
		virtual bool BeginTable(const char* a_id, const char* const* a_columnHeaders, std::int32_t a_columnCount, const float* a_columnWidths) = 0;
		virtual void EndTable() = 0;

		virtual void TableNextRow() = 0;

		// Text put in the cell lines up with the framed widgets beside it.
		virtual void TableNextColumn() = 0;

		// Equally wide columns without headers, each with a margin on its right. On false EndColumns must not be called.
		virtual bool BeginColumns(const char* a_id, std::int32_t a_columnCount) = 0;
		virtual void NextColumn() = 0;
		virtual void EndColumns() = 0;

		// Tells apart widgets that share a label, such as the same button on several table rows.
		virtual void PushID(std::int32_t a_id) = 0;
		virtual void PopID() = 0;

		// Tooltip for the item drawn just before.
		virtual void ItemTooltip(const char* a_text) = 0;

		// With a_footer as a highlighted last line. Either may be nullptr.
		virtual void ItemTooltip(const char* a_text, const char* a_footer) = 0;

		// With the footer naming a_default by the image of its key.
		virtual void ItemTooltip(const char* a_text, ButtonBinding a_default) = 0;

		virtual void OpenPopup(const char* a_id) = 0;
		virtual bool BeginPopupModal(const char* a_id) = 0;
		virtual void EndPopup() = 0;
		virtual void CloseCurrentPopup() = 0;

		// Draws a_label in the label column and returns the id the widget next to it has to use.
		virtual std::string LabelColumn(const char* a_label) = 0;

		// A row that binds one button: clicking it listens for a press, clicking again gives up. A press from a
		// device outside a_filter, or Escape, cancels without a change. Returns true on the frame a new binding
		// was accepted, with a_value already written. Not virtual, the capture logic is shared by every backend.
		bool BindButton(const char* a_label, ButtonBinding* a_value, BindFilter a_filter = BindFilter::kAny);

		// As above, for a setting: a_tooltip replaces the generic hint when set, and a_default is named on the last line.
		bool BindButton(const char* a_label, ButtonBinding* a_value, ButtonBinding a_default, const char* a_tooltip, BindFilter a_filter = BindFilter::kAny);

		// Draws one binder row and returns whether it was clicked, presentation only. The default draws a text
		// button, a backend overrides it to draw the key's image.
		virtual bool BindWidget(const char* a_label, ButtonBinding a_binding, bool a_listening);

	private:
		bool DrawBindButton(const char* a_label, ButtonBinding* a_value, BindFilter a_filter, const char* a_tooltip, const ButtonBinding* a_default);
	};
}
