#include "ueditor/core/viewport_window.h"

#include "uengine/core/ui/imgui.h"

namespace ueditor {
	ViewportWindow::ViewportWindow() : EditorWindow("Viewport") {
		_last_size = size();
	}

	void ViewportWindow::on_imgui() {
		if (_last_size != size()) {
			_last_size = size();
			_framebuffer->update(_last_size.x, _last_size.y);
			Graphics::viewport(0, 0, _last_size.x, _last_size.y);
		}

		ImGui::Image((ImTextureID)_framebuffer->color_buffer(), ImGui::GetContentRegionAvail(), {0, 1}, {1, 0});
	}
}