#pragma once
#ifndef UEDITOR_CORE_COMPONENT_DRAWER
#define UEDITOR_CORE_COMPONENT_DRAWER

#include <uengine/core/string.h>

using namespace uengine;

namespace ueditor {
	class IComponentDrawer {
	public:
		virtual const String& name() const = 0;
	protected:
		virtual void on_imgui() = 0;

		virtual void target(void* ptr) = 0;

		virtual TypeIndex type_index() = 0;

		friend class PropertiesWindow;
	};

	template<typename T>
	class ComponentDrawer : public IComponentDrawer {
	public:
		ComponentDrawer(const String& name) : _name(name) {}

		const String& name() const override {
			return _name;
		}
	protected:
		void on_imgui() override {}

		T& target() {
			return *static_cast<T*>(_component);
		}
	private:
		void target(void* ptr) override {
			_component = ptr;
		}

		TypeIndex type_index() override {
			return typeid(T);
		}

		String _name;
		void* _component;

		friend class PropertiesWindow;
	};
}

#endif