#include "ueditor/core/output_window.h"

#include "uengine/core/application.h"

namespace ueditor {
	OutputWindow::OutputWindow() : EditorWindow("Output") {
		Application::instance()->on_log += UENGINE_BIND_METHOD_PTR(Application::LogEvent, this, &OutputWindow::add_message);
	}

	void OutputWindow::clear() {
		_messages.clear();
	}

	void OutputWindow::on_imgui() {
		if (ImGui::Button("Clear"))
			clear();

		for (auto& message : _messages) {
			switch (message.level) {
			case Log::Level::Trace:
				ImGui::TextColored({0.9f, 0.9f, 0.9f, 1.0f}, message.text.data());
				break;
			case Log::Level::Info:
				ImGui::TextColored({0.57f, 0.89f, 0.57f, 1.0f}, message.text.data());
				break;
			case Log::Level::Debug:
				ImGui::TextColored({0.3f, 0.58f, 0.87f, 1.0f}, message.text.data());
				break;
			case Log::Level::Warning:
				ImGui::TextColored({0.86f, 0.86f, 0.59f, 1.0f}, message.text.data());
				break;
			case Log::Level::Error:
				ImGui::TextColored({0.91f, 0.28f, 0.33f, 1.0f}, message.text.data());
				break;
			case Log::Level::Critical:
				ImVec2 cursor_screen_position = ImGui::GetCursorScreenPos();
				ImGui::GetWindowDrawList()->AddRectFilled(cursor_screen_position,
					{cursor_screen_position.x + ImGui::GetContentRegionAvail().x, cursor_screen_position.y + ImGui::CalcTextSize("[]").y + ImGui::GetStyle().FramePadding.y},
					ImGui::GetColorU32({0.91f, 0.28f, 0.33f, 1.0f}));
				ImGui::TextColored({0.9f, 0.9f, 0.9f, 1.0f}, message.text.data());
				break;
			}
		}
	}

	void OutputWindow::add_message(Log::Level level, const String& text) {
		_messages.add({level, text});
	}
}