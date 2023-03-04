#pragma once
#ifndef UEDITOR_CORE_WINDOW
#define UEDITOR_CORE_WINDOW

#include <uengine/core/string.h>
#include <uengine/core/traits.h>
#include <uengine/core/collections/dictionary.h>
#include <uengine/core/math/vector2.h>
#include <uengine/core/ui/imgui.h>

using namespace uengine;

namespace ueditor {
	class EditorWindow : public ImGuiWindowClass {
	public:
		EditorWindow(const String& name) : _name(name) {
			DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoWindowMenuButton;
		}

		const String& name() const {
			return _name;
		}

		void open() {
			_is_opened = true;
		}

		void close() {
			_is_opened = false;
		}

		Float2 size() const {
			return _size;
		}
	protected:
		virtual void on_imgui() {}
	private:
		static Dictionary<Size, SharedPtr<EditorWindow>> _windows;
		String _name;
		bool _is_opened = false;
		Float2 _size;
		ImGuiWindowFlags _window_flags = 0;

		static Dictionary<Size, SharedPtr<EditorWindow>>& windows();

		template<DerivedFrom<EditorWindow> T>
		static void add() {
			Size id = typeid(T).hash_code();
			if (windows().has(id))
				return;
			windows().add(id, uengine::make_shared<T>());
		}

		template<DerivedFrom<EditorWindow> T>
		static SharedPtr<T> get() {
			Size id = typeid(T).hash_code();
			if (!windows().has(id))
				return nullptr;

			return uengine::static_pointer_cast<T>(windows()[id]);
		}

		static void update();

		friend class EditorApplication;
	};
}

#endif