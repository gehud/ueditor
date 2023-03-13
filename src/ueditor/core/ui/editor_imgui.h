#pragma once
#ifndef UEDITOR_CORE_UI_EDITOR_IMGUI
#define UEDITOR_CORE_UI_EDITOR_IMGUI

#include <uengine/core/definitions.h>
#include <uengine/core/object.h>
#include <uengine/core/string.h>
#include <uengine/core/memory.h>
#include <uengine/core/ui/imgui.h>

using namespace uengine;

namespace ueditor {
    class EditorIMGUI {
    public:
        template<DerivedFrom<Object> T>
        static T* object_field(const String& label, T* object) {
            ImGui::Text(label.data());
            ImGui::SameLine();
            auto cursor_screen_position = ImGui::GetCursorScreenPos();
            auto label_size = ImGui::CalcTextSize(label.data());
            auto rect = ImRect{
                cursor_screen_position, {
                    cursor_screen_position.x + ImGui::GetContentRegionAvail().x,
                    cursor_screen_position.y + label_size.y
                }
            };

            ImGui::GetWindowDrawList()->AddRectFilled(rect.Min, rect.Max, ImGui::GetColorU32({0.08f, 0.08f, 0.08f, 1.0f}));
            auto id = ImGui::GetID(label.data());
            if (ImGui::BeginDragDropTargetCustom(rect, id)) {
                if (auto payload = ImGui::AcceptDragDropPayload("DRAG_DROP_OBJECT")) {
                    Object* data = static_cast<Object**>(payload->Data)[0];
                    if (dynamic_cast<T*>(data))
                        return static_cast<T*>(data); 
                }

                ImGui::EndDragDropTarget();
            }

            ImGui::Dummy({
                -label_size.x - ImGui::GetStyle().FramePadding.x * 2,
                label_size.y
            });

            return object;
        }

        template<DerivedFrom<Object> T>
        static bool object_field(const String& label, SharedPtr<T>& object) {
            auto result = object_field(label, object.get());
            if (result != object.get()) {
                Log::info((int)result);
                if (dynamic_cast<T*>(drag_drop_object().get()) == result) {
                    object = uengine::static_pointer_cast<T>(drag_drop_object());
                    drag_drop_object() = nullptr;
                } else
                    object.reset(result);

                return true;
            }

            return false;
        }
    private:
        // Saved shared reference when drag and drop.
        static SharedPtr<Object> _drag_drop_object;

        static SharedPtr<Object>& drag_drop_object();

        template<DerivedFrom<Object> T>
        static void set_object_payload(T* object) {

            ImGui::SetDragDropPayload("DRAG_DROP_OBJECT", &object, sizeof(Object*));
        }

        template<DerivedFrom<Object> T>
        static void set_object_payload(const SharedPtr<T>& object) {
            drag_drop_object() = object;
            set_object_payload(drag_drop_object().get());
        }

        static void flush();

        friend class EditorApplication;
        friend class ExplorerWindow;
    };
}

#endif