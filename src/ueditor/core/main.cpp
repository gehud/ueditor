#include <uengine.h>
#include <uengine/core/entry.h>

using namespace uengine;

class editor_application : public application {
protected:
	void on_update() {
		graphics::clear_color(0.1f, 0.1f, 0.1f, 1.0f);
		graphics::clear(clear_flags::color);
	}

	void on_imgui() override {
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		ImGui::SetCurrentContext(imgui::get_context());

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Main", nullptr, window_flags);
		ImGui::PopStyleVar();
		ImGui::PopStyleVar(2);

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Save", "Ctrl+S")) {
					// Save
				}

				if (ImGui::MenuItem("Settings")) {

				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window")) {
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("DockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

			static auto first_time = true;
			if (first_time) {
				first_time = false;

				ImGui::DockBuilderRemoveNode(dockspace_id);
				ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
				ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

				auto dock_id_scene = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.25f, nullptr, &dockspace_id);
				auto dock_id_properties = ImGui::DockBuilderSplitNode(dock_id_scene, ImGuiDir_Down, 0.5f, nullptr, &dock_id_scene);
				auto dock_id_output = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.35f, nullptr, &dockspace_id);
				auto dock_id_explorer = ImGui::DockBuilderSplitNode(dock_id_output, ImGuiDir_Right, 0.5f, nullptr, &dock_id_output);

				ImGui::DockBuilderDockWindow("Scene", dock_id_scene);
				ImGui::DockBuilderDockWindow("Properties", dock_id_properties);
				ImGui::DockBuilderDockWindow("Explorer", dock_id_explorer);
				ImGui::DockBuilderDockWindow("Output", dock_id_output);

				ImGui::DockBuilderFinish(dockspace_id);
			}
		}

		ImGui::End();
	}
};

namespace uengine {
	application* create_application() {
		return new editor_application();
	}
}