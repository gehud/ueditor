#pragma once
#ifndef UEDITOR_CORE_VIEWPORT_WINDOW
#define UEDITOR_CORE_VIEWPORT_WINDOW

#include "ueditor/core/editor_window.h"

#include <uengine/core/memory.h>
#include <uengine/core/rendering/framebuffer.h>
#include <uengine/core/rendering/graphics.h>

namespace ueditor {
	class ViewportWindow final : public EditorWindow {
	public:
		ViewportWindow();

		void framebuffer(const SharedPtr<Framebuffer>& framebuffer) {
			_framebuffer = framebuffer;
			_framebuffer->update(_last_size.x, _last_size.y);
			Graphics::viewport(0, 0, _last_size.x, _last_size.y);
			if (_last_size.x != 0 && _last_size.y != 0) {
				Graphics::aspect(_last_size.x / (float)_last_size.y);
			}
		}
	protected:
		void on_imgui() override;
	private:
		SharedPtr<Framebuffer> _framebuffer;
		Float2 _last_size;
	};
}

#endif