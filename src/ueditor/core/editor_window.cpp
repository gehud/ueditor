#include "ueditor/core/editor_window.h"

namespace ueditor {
	Dictionary<Size, SharedPtr<EditorWindow>> EditorWindow::_windows;
	EditorWindow* EditorWindow::_focused = nullptr;

	EditorWindow* EditorWindow::focused() {
		return _focused;
	}

	Dictionary<Size, SharedPtr<EditorWindow>>& EditorWindow::windows() {
		return _windows;
	}

	void EditorWindow::update() {
		bool is_any_window_focused = false;

		for (auto& pair : _windows) {
			if (!pair.value()->_is_opened)
				continue;

			ImGui::SetNextWindowClass(pair.value().get());
			if (ImGui::Begin(pair.value()->name().data(), &pair.value()->_is_opened, pair.value()->_window_flags)) {
				auto& window = pair.value();
				auto size = Float2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
				if (size != window->_size) {
					window->_size = Float2(size.x, size.y);
				}

				pair.value()->on_imgui();
				if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
					_focused = pair.value().get();
					pair.value()->_is_focused = true;
					is_any_window_focused = true;
				} else {
					pair.value()->_is_focused = true;
				}

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsAnyItemHovered()) {
					pair.value()->_is_item_clicked = true;
				} else {
					pair.value()->_is_item_clicked = false;
				}
			}

			ImGui::End();
		}

		if (!is_any_window_focused)
			_focused = nullptr;
	}
}