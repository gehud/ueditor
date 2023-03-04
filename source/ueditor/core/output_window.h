#pragma once
#ifndef UEDITOR_CORE_OUTPUT_WINDOW
#define UEDITOR_CORE_OUTPUT_WINDOW

#include "ueditor/core/editor_window.h"

#include <uengine/core/log.h>
#include <uengine/core/string.h>
#include <uengine/core/collections/list.h>

using namespace uengine;

namespace ueditor {
	class OutputWindow : public EditorWindow {
	public:
		OutputWindow();

		void clear();
	protected:
		void on_imgui() override;
	private:
		struct Message {
			Log::Level level;
			String text;
		};

		List<Message> _messages;

		void add_message(Log::Level level, const String& text);
	};
}

#endif