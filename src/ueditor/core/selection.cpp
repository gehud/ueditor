#include "ueditor/core/selection.h"

namespace ueditor {
    Object* Selection::_context = nullptr;
    Object* Selection::_object = nullptr;

    Object* Selection::context() {
        return _context;
    }

     Object* Selection::object() {
        return _object;
    }

    void Selection::context(Object* value) {
        _context = value;
    }

    void Selection::object(Object* value) {
        _object = value;
    }

    void Selection::object_with_context(Object* object, Object* context) {
        _object = object;
        _context = _context;
    }

    void Selection::flush() {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
            _context = nullptr;
            _object = nullptr;
        }
    }
}