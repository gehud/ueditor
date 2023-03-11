#pragma once
#ifndef UEDITOR_CORE_EXPLORER
#define UEDITOR_CORE_EXPLORER

#include "ueditor/core/editor_window.h"

#include <uengine/core/io/path.h>
#include <uengine/core/collections/dictionary.h>
#include <uengine/core/rendering/texture.h>

namespace ueditor {
	class ExplorerWindow : public EditorWindow {
	public:
		ExplorerWindow();

		void project_path(const Path& path) {
			_project_path = path;
		}
	protected:
		void on_imgui() override;
	private:
		Path _project_path;
		Path _directory_path;
		Path _selected_path;
		Path _rename_path;
		SharedPtr<Texture2D> _folder_icon;
		SharedPtr<Texture2D> _folder_open_icon;
		SharedPtr<Texture2D> _file_icon;
		SharedPtr<Texture2D> _cube_icon;
		SharedPtr<Texture2D> _mesh_icon;
		Dictionary<Path, List<ULong>> _uuids;
		Dictionary<Path, Dictionary<ULong, String>> _names;
		Dictionary<Path, Dictionary<ULong, String>> _types;

		Path get_directory_path_from_selected();

		void create_folder();

		void create_file();

		void handle_popup();

		bool directory_tree(const char* label, ImGuiTreeNodeFlags flags, ImTextureID icon, float* icon_width = nullptr, bool* is_pressed = nullptr);

		void make_tree(const Path& path, bool is_asset = false);

		void delete_selected();

		void rename_selected();

		void handle_delete();

		void handle_rename();

		void open_externaly();
	};
}

#endif