#include <uengine.h>
#include <uengine/core/entry.h>

#include "reflection.h"
#include "library_loader.h"
#include "ueditor/core/output_window.h"

#include <Windows.h>

using namespace uengine;

namespace ueditor {
	class EditorApplication : public Application {
	public:
		EditorApplication() {
			EditorWindow::add<OutputWindow>();
			EditorWindow::get<OutputWindow>()->open();
		}
	protected:
		void on_start() override {
			Window::instance()->vsync(true);
			Log::info("Hello, UEngine!");
		}

		void on_update() override {
			Graphics::clear_color(0.1f, 0.1f, 0.1f, 1.0f);
			Graphics::clear(ClearFlags::Color);
		}

		void on_imgui() override {
			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

			ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

			ImGui::SetCurrentContext(IMGUI::context());

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

					if (ImGui::MenuItem("Open Project", "Ctrl+P Ctrl+O")) {
						open_project(FileDialog::open_folder());
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Window")) {
					for (auto& pair : EditorWindow::windows()) {
						if (ImGui::MenuItem(pair.value->name().data())) {
							pair.value->open();
						}
					}
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

			EditorWindow::update();
		}
	private:
		String _project_path;
		SharedPtr<World> _world;

		void open_project(const String& path) {
			_project_path = path;
			compile_project();
		}

		void compile_project() {
			if (_project_path.is_empty()) {
				Log::trace("Nothing to compile.");
				return;
			}

			auto data_path = _project_path + "/.uengine";
			/*if (!directory::exists(data_path)) {
				auto result = directory::create(data_path);
				UENGINE_ASSERT(result, "Failed to create directory.");
			}
			FileStream fs(data_path + "/CMakeLists.txt", StreamMode::out);
			UENGINE_ASSERT(fs, "Failed to create CMakeLists.txt.");
			auto install_prefix = (Path::current() + "/..").as_string().replace('\\', '/');
			fs << "cmake_minimum_required(VERSION 3.10)\n";
			fs << "set(CMAKE_CXX_STANDARD 23)\n";
			fs << "set(CMAKE_CXX_STANDARD_REQUIRED TRUE)\n";
			fs << "project(Example)\n";
			fs << "file(GLOB_RECURSE EXAMPLE_SOURCE_FILES \"../source/*.h\" \"../source/*.cpp\" \"source/*.h\" \"source/*.cpp\")\n";
			fs << "add_library(${PROJECT_NAME} SHARED ${EXAMPLE_SOURCE_FILES})\n";
			fs << "target_include_directories(${PROJECT_NAME} PUBLIC \"../source\" \"source\" \"" << install_prefix + "/include" << "\")\n";
			fs << "target_link_directories(${PROJECT_NAME} PRIVATE \"" << install_prefix + "/lib" << "\" \"" << install_prefix + "/bin" << "\")\n";
			fs << "target_link_libraries(${PROJECT_NAME} PRIVATE uengine)\n";
			fs << "target_compile_definitions(${PROJECT_NAME} PRIVATE \"UENGINE_SCRIPT_API=__declspec(dllexport)\" INTERFACE \"UENGINE_SCRIPT_API=__declspec(dllimport)\")\n";
			fs << "set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME \"example\")\n";
			fs << "install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})";
			fs.close();
			auto src_path = data_path + "/source";
			if (!directory::exists(src_path)) {
				auto result = directory::create(src_path);
				UENGINE_ASSERT(result, "Failed to create directory.");
			}

			auto build_path = data_path + "/build";
			if (!directory::exists(build_path)) {
				auto result = directory::create(build_path);
				UENGINE_ASSERT(result, "Failed to create directory.");
			}*/

			auto install_path = data_path + "/install";
			/*if (!directory::exists(install_path)) {
				auto result = directory::create(install_path);
				UENGINE_ASSERT(result, "Failed to create directory.");
			}

			auto assembly = reflection::reflect(_project_path + "/source");

			FileStream glue_h_stream(src_path + "/glue.h", StreamMode::out);
			glue_h_stream << "#pragma once\n";
			glue_h_stream << "#include <uengine.h>\n";
			glue_h_stream << "extern \"C\" UENGINE_SCRIPT_API int get_system_count();\n";
			glue_h_stream << "extern \"C\" UENGINE_SCRIPT_API void get_systems(char** names, int* ids);\n";
			for (auto& type : assembly.get_types()) {
				glue_h_stream << "extern \"C\" UENGINE_SCRIPT_API uengine::system* allocate_" << type.get_name() << "();\n";
			}
			glue_h_stream.close();

			FileStream glue_cpp_stream(src_path + "/glue.cpp", StreamMode::out);
			glue_cpp_stream << "#include \"glue.h\"\n";
			for (auto& type : assembly.get_types()) {
				glue_cpp_stream << "#include \"" << Path::relative(type.get_path(), _project_path + "/source").as_string().get_data() << "\"\n";
				glue_cpp_stream << "uengine::system* allocate_" << type.get_name() << "() {\n";
				glue_cpp_stream << "	return new " << type.get_name() << "();\n";
				glue_cpp_stream << "}\n";
			}
			glue_cpp_stream << "int get_system_count() { return " << assembly.get_types().get_count() << "; }\n";
			glue_cpp_stream << "void get_systems(char** names, int* ids) {\n";
			for (int i = 0; i < assembly.get_types().get_count(); i++) {
				glue_cpp_stream << "	strcpy(names[" << i << "], \"" << assembly.get_types()[i].get_name() << "\");\n";
				glue_cpp_stream << "	ids[" << i << "] = uengine::type_info(typeid(" << assembly.get_types()[i].get_name() << ")).hash();\n";
			}
			glue_cpp_stream << "}";
			glue_cpp_stream.close();

			auto command = "cd " + data_path + "/build" + " && cmake .. && cmake --build . && cmake --install . --prefix \"" + data_path + "/install\" --config Debug";
			std::system(command.get_data());*/

			library_loader ll;
			ll.open(install_path + "/bin/example.dll");
			auto c = ll.get<int()>("get_system_count");
			auto s = ll.get<void(char**, int*)>("get_systems");
			int count = c();
			char** names = new char*[count];
			for (int i = 0; i < count; i++) {
				names[i] = new char[256];
			}
			int* ids = new int[count];
			s(names, ids);

			for (int i = 0; i < count; i++) {
				UENGINE_LOG_INFO(ids[i]);
				UENGINE_LOG_INFO(names[i]);
				auto a = ll.get<System*()>("allocate_" + String(names[i]));
				World::register_system(ids[i], a);
			}

			_world = make_shared<World>();
			_world->update();
			_world->update();

			for (int i = 0; i < count; i++) {
				delete[] names[i];
			}
			delete[] names;
			delete[] ids;
		}
	};
}

namespace uengine {
	Application* create_application() {
		return new ueditor::EditorApplication();
	}
}