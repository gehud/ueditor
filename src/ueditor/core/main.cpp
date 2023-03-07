#include <uengine.h>
#include <uengine/core/entry.h>

#include "ueditor/core/assets.h"
#include "ueditor/core/library.h"
#include "ueditor/core/reflection.h"
#include "ueditor/core/explorer_window.h"
#include "ueditor/core/outline_window.h"
#include "ueditor/core/output_window.h"
#include "ueditor/core/properties_window.h"
#include "ueditor/core/viewport_window.h"
#include "ueditor/core/scene_serializer.h"
#include "ueditor/core/camera_controller.h"

#include <uengine/core/scene.h>
#include <uengine/core/rendering/framebuffer.h>
#include <uengine/core/rendering/model.h>

using namespace uengine;

namespace ueditor {
	class EditorApplication : public Application {
	public:
		EditorApplication() {
			EditorWindow::add<ExplorerWindow>();
			EditorWindow::get<ExplorerWindow>()->open();

			EditorWindow::add<OutputWindow>();
			EditorWindow::get<OutputWindow>()->open();

			EditorWindow::add<OutlineWindow>();
			EditorWindow::get<OutlineWindow>()->open();

			EditorWindow::add<PropertiesWindow>();
			EditorWindow::get<PropertiesWindow>()->open();

			EditorWindow::add<ViewportWindow>();
			EditorWindow::get<ViewportWindow>()->open();
			_framebuffer = make_shared<Framebuffer>();
			EditorWindow::get<ViewportWindow>()->framebuffer(_framebuffer);
		}
	protected:
		void on_start() override {
			Window::instance()->vsync(true);
			Log::info("Hello, UEngine!");
			auto scene = make_shared<Scene>();

			// Editor camera
			auto editor_camera_entity = scene->world().create_entity();
			auto& editor_camera_transform = scene->world().add_component<Transform>(editor_camera_entity);
			editor_camera_transform.position = {0.0f, 0.0f, -3.0f};
			scene->world().add_component<CameraController>(editor_camera_entity);
			auto& editor_camera = scene->world().add_component<Camera>(editor_camera_entity);
			editor_camera.clear_color = {0.1f, 0.1f, 0.1f, 1.0f};
			scene->world().create_system<CameraControllerSystem>();
			auto camera_entity = scene->world().create_entity();
			auto& camera = scene->world().add_component<Camera>(camera_entity);
			camera.clear_color = {0.1f, 0.2f, 0.1f, 1.0f};
			auto& transform = scene->world().add_component<Transform>(camera_entity);
			transform.position = Float3::back() * 4.0f + Float3::up() * 1.0f;
			auto cube = scene->world().create_entity();
			auto& render_mesh = scene->world().add_component<RenderMesh>(cube);
			auto cube_model = make_shared<Model>("../assets/models/cube.fbx");
			render_mesh.mesh = cube_model->meshes()[0];
			auto shader = make_shared<Shader>("../assets/shaders/texture.glsl");
			auto material = make_shared<Material>(shader);
			auto texture = make_shared<Texture2D>("../assets/textures/checkerboard.png");
			texture->filter_mode(Texture::FilterMode::Nearest);
			material->set("u_Texture", texture);

			render_mesh.materials = {material};

			Scene::load(scene);

			Input::on_key_pressed += UENGINE_BIND_METHOD_PTR(Input::KeyPressedEvent, this, &EditorApplication::on_key_pressed);
		}

		void on_update() override {
			// Drawing loaded scenes.
			_framebuffer->bind();
			Scene::active()->world().update();
			_framebuffer->unbind();

			// Background.
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
						save_file();
					}

					if (ImGui::MenuItem("Open", "Ctrl+O")) {
						open_file();
					}

					if (ImGui::MenuItem("Project", "Ctrl+P")) {
						open_project();
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

			// Toolbar.
			ImGui::SetCursorPos({ImGui::GetWindowSize().x / 2 - ImGui::CalcTextSize("Compile").x / 2,
								 ImGui::GetCursorPos().y + ImGui::GetStyle().WindowPadding.y / 2});
			if (ImGui::Button("Compile"))
				compile_project();
			ImGui::Separator();

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

					ImGuiID dock_id_center = 0;
					auto dock_id_outline = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id);
					auto dock_id_properties = ImGui::DockBuilderSplitNode(dock_id_outline, ImGuiDir_Down, 0.5f, nullptr, &dock_id_outline);
					auto dock_id_output = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.3f, nullptr, &dock_id_center);
					auto dock_id_explorer = ImGui::DockBuilderSplitNode(dock_id_output, ImGuiDir_Right, 0.5f, nullptr, &dock_id_output);

					ImGui::DockBuilderDockWindow("Explorer", dock_id_explorer);
					ImGui::DockBuilderDockWindow("Outline", dock_id_outline);
					ImGui::DockBuilderDockWindow("Output", dock_id_output);
					ImGui::DockBuilderDockWindow("Properties", dock_id_properties);
					ImGui::DockBuilderDockWindow("Viewport", dock_id_center);

					ImGui::DockBuilderFinish(dockspace_id);
				}
			}

			ImGui::End();

			EditorWindow::update();
		}
	private:
		String _project_path;
		SharedPtr<Framebuffer> _framebuffer;
		SharedPtr<Library> _scripts;

		void on_key_pressed(int key, int repreat_count) {
			bool control = Input::is_key(UENGINE_KEY_LEFT_CONTROL);
			switch (key) {
			case UENGINE_KEY_S:
				if (control) {
					save_file();
				}
				break;
			case UENGINE_KEY_O:
				if (control) {
					open_file();
				}
				break;
			case UENGINE_KEY_P:
				if (control) {
					open_project();
				}
				break;
			}
		}

		void save_file() {
			auto destination = FileDialog::save_file("UEngine Scene (*.uengine)\0*.uengine\0");
			if (destination.is_empty()) {
				Log::error("Wrong path.");
				return;
			}
			SceneSerializer ss(Scene::active());
			ss.serialize(destination);
		}

		void open_file() {
			auto source = FileDialog::open_file("UEngine Scene (*.uengine)\0*.uengine\0");
			if (source.is_empty()) {
				Log::error("Wrong path.");
				return;
			}
			auto scene = make_shared<Scene>();
			SceneSerializer ss(scene); 
			ss.deserialize(source);
			Scene::load(scene);
		}

		void open_project() {
			_project_path = FileDialog::open_folder();
			if (_project_path.is_empty()) {
				Log::error("Wrong path.");
				return;
			}

			EditorWindow::get<ExplorerWindow>()->project_path(_project_path);
			
			auto assets_path = _project_path + "/assets";
			if (Directory::exists(assets_path)) {
				Assets::path(assets_path);
				import_assets(assets_path);
			}
		}

		void import_assets(const Path& path) {
			Directory::for_each(path, [&](DirectoryEntry entry) {
				if (entry.is_directory()) {
					import_assets(entry.path());
				} else if (entry.path().extension() == ".fbx") {
					Assets::import(Path::relative(entry.path(), Assets::path()));
				}
			});
		}

		void compile_project() {
			if (_project_path.is_empty() || !Directory::exists(_project_path + "/assets")) {
				Log::error("Wrong path.");
				return;
			}

			auto data_path = _project_path + "/.uengine";
			if (!Path::exists(data_path)) {
				auto result = Directory::create(data_path);
				UENGINE_ASSERT(result, "Failed to create directory.");
			}
			FileStream fs(data_path + "/CMakeLists.txt", OpenMode::Out);
			UENGINE_ASSERT(fs, "Failed to create CMakeLists.txt.");
			auto install_prefix = (Path::current() + "/..").string().replace('\\', '/');
			fs << "cmake_minimum_required(VERSION 3.10)\n";
			fs << "set(CMAKE_CXX_STANDARD 23)\n";
			fs << "set(CMAKE_CXX_STANDARD_REQUIRED TRUE)\n";
			fs << "project(Example)\n";
			fs << "file(GLOB_RECURSE EXAMPLE_SOURCE_FILES \"../assets/*.h\" \"../assets/*.cpp\" \"src/*.h\" \"src/*.cpp\")\n";
			fs << "add_library(${PROJECT_NAME} SHARED ${EXAMPLE_SOURCE_FILES})\n";
			fs << "target_include_directories(${PROJECT_NAME} PUBLIC \"../assets\" \"../assets/scripts\" \"src\" \"" << install_prefix + "/include" << "\")\n";
			fs << "target_link_directories(${PROJECT_NAME} PRIVATE \"" << install_prefix + "/lib" << "\" \"" << install_prefix + "/bin" << "\")\n";
			fs << "target_link_libraries(${PROJECT_NAME} PRIVATE uengine)\n";
			fs << "target_compile_definitions(${PROJECT_NAME} PRIVATE \"UEDITOR\" \"UENGINE_SCRIPT_API=__declspec(dllexport)\" INTERFACE \"UENGINE_SCRIPT_API=__declspec(dllimport)\")\n";
			fs << "set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME \"example\")\n";
			fs << "install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})";
			fs.close();
			auto src_path = data_path + "/src";
			if (!Path::exists(src_path)) {
				auto result = Directory::create(src_path);
				UENGINE_ASSERT(result, "Failed to create directory.");
			}

			auto build_path = data_path + "/build";
			if (!Path::exists(build_path)) {
				auto result = Directory::create(build_path);
				UENGINE_ASSERT(result, "Failed to create directory.");
			}

			auto install_path = data_path + "/install";
			if (!Path::exists(install_path)) {
				auto result = Directory::create(install_path);
				UENGINE_ASSERT(result, "Failed to create directory.");
			}

			auto assembly = Reflection::reflect(_project_path + "/assets");

			FileStream glue_h_stream(src_path + "/glue.h", OpenMode::Out);
			glue_h_stream << "#pragma once\n";
			glue_h_stream << "#include <uengine/core/system.h>\n";
			glue_h_stream << "extern \"C\" UENGINE_SCRIPT_API int get_system_count();\n";
			glue_h_stream << "extern \"C\" UENGINE_SCRIPT_API void get_systems(char** names, int* ids);\n";
			for (auto& type : assembly.types()) {
				glue_h_stream << "extern \"C\" UENGINE_SCRIPT_API uengine::System* allocate_" << type.name() << "();\n";
				glue_h_stream << "extern \"C\" UENGINE_SCRIPT_API void delete_" << type.name() << "(uengine::System*);\n";
			}
			glue_h_stream.close();

			FileStream glue_cpp_stream(src_path + "/glue.cpp", OpenMode::Out);
			glue_cpp_stream << "#include \"glue.h\"\n";
			for (auto& type : assembly.types()) {
				glue_cpp_stream << "#include \"" << Path::relative(type.path(), _project_path + "/source").string().data() << "\"\n";
				glue_cpp_stream << "uengine::System* allocate_" << type.name() << "() {\n";
				glue_cpp_stream << "	return new " << type.name() << "();\n";
				glue_cpp_stream << "}\n";
				glue_cpp_stream << "void delete_" << type.name() << "(uengine::System* ptr) {\n";
				glue_cpp_stream << "	delete static_cast<" << type.name() << "*>(ptr);\n";
				glue_cpp_stream << "}\n";
			}
			glue_cpp_stream << "int get_system_count() { return " << assembly.types().count() << "; }\n";
			glue_cpp_stream << "void get_systems(char** names, int* ids) {\n";
			for (int i = 0; i < assembly.types().count(); i++) {
				glue_cpp_stream << "	strcpy(names[" << i << "], \"" << assembly.types()[i].name() << "\");\n";
				glue_cpp_stream << "	ids[" << i << "] = typeid(" << assembly.types()[i].name() << ").hash_code();\n";
			}
			glue_cpp_stream << "}";
			glue_cpp_stream.close();

			auto command = "cd " + data_path + "/build" + " && cmake .. && cmake --build . && cmake --install . --prefix \"" + data_path + "/install\" --config Debug";
			std::system(command.data());

			_scripts = make_shared<Library>(install_path + "/bin/example.dll");
			auto c = _scripts->get<int()>("get_system_count");
			auto s = _scripts->get<void(char**, int*)>("get_systems");
			int count = c();
			char** names = new char*[count];
			for (int i = 0; i < count; i++) {
				names[i] = new char[256];
			}
			int* ids = new int[count];
			s(names, ids);

			for (int i = 0; i < count; i++) {
				auto a = _scripts->get<System*()>("allocate_" + String(names[i]));
				auto d = _scripts->get<void(System*)>("delete_" + String(names[i]));
				World::register_system(ids[i], a, d);
			}

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