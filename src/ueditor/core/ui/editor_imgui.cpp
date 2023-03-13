#include "ueditor/core/ui/editor_imgui.h"

namespace ueditor {
    SharedPtr<Object> EditorIMGUI::_drag_drop_object = nullptr;

    SharedPtr<Object>& EditorIMGUI::drag_drop_object() {
        return _drag_drop_object;
    }

    void EditorIMGUI::flush() {
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && _drag_drop_object)
            _drag_drop_object = nullptr;
    }
}