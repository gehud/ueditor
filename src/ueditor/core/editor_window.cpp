#include "ueditor/core/editor_window.h"

namespace ueditor {
	Dictionary<Size, SharedPtr<EditorWindow>> EditorWindow::_windows;

	Dictionary<Size, SharedPtr<EditorWindow>>& EditorWindow::windows() {
		return _windows;
	}

	void EditorWindow::update() {
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
			}
			ImGui::End();
		}
	}
}