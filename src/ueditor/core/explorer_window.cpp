#include "ueditor/core/explorer_window.h"

#include <uengine/core/io/directory.h>
#include <uengine/core/io/file.h>
#include <uengine/core/ui/imgui.h>

namespace ueditor {
	ExplorerWindow::ExplorerWindow() : EditorWindow("Explorer") {
		_folder_icon = make_shared<Texture2D>("../assets/textures/folder.png");
		_folder_open_icon = make_shared<Texture2D>("../assets/textures/folder_open.png");
		_file_icon = make_shared<Texture2D>("../assets/textures/file.png");
		_cube_icon = make_shared<Texture2D>("../assets/textures/cube.png");
		_mesh_icon = make_shared<Texture2D>("../assets/textures/mesh.png");
	}

	void ExplorerWindow::on_imgui() {
        if (_project_path.is_empty())
            return;

        make_tree(_project_path);
        if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)
            || ((ImGui::IsMouseClicked(ImGuiMouseButton_Left)
                || ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                && !ImGui::IsAnyItemHovered())) {
            _selected_path.clear();
            _directory_path.clear();
            _rename_path.clear();
        }
        handle_delete();
        handle_rename();
	}

    Path ExplorerWindow::get_directory_path_from_selected() {
        if (!_selected_path.is_directory())
            return _selected_path.parent();
        return _selected_path;
    }

    void ExplorerWindow::create_folder() {
        Path path = _directory_path + "/folder";
        bool creation_result = Directory::create(path);
        UENGINE_ASSERT(creation_result, "Failed to create folder.");
        _selected_path = path;
        rename_selected();
    }

    void ExplorerWindow::create_file() {
        Path path = _directory_path + "/file";
        FileStream new_file(path);
        UENGINE_ASSERT(new_file, "Failed to create file.");
        _selected_path = path;
        rename_selected();
    }

    void ExplorerWindow::handle_popup() {
        if (ImGui::BeginPopupContextItem()) {
            ImGui::TextDisabled("Create");

            if (ImGui::MenuItem("Folder"))
                create_folder();

            if (ImGui::MenuItem("File"))
                create_file();

            ImGui::TextDisabled("Actions");

            if (ImGui::MenuItem("Rename"))
                rename_selected();

            if (ImGui::MenuItem("Delete"))
                delete_selected();

            ImGui::EndPopup();
        }
    }

    bool ExplorerWindow::directory_tree(const char* label, ImGuiTreeNodeFlags flags, ImTextureID icon, float* icon_width, bool* is_pressed) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiID id = window->GetID(label);

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const bool display_frame = (flags & ImGuiTreeNodeFlags_Framed) != 0;
        const ImVec2 padding = (display_frame || (flags & ImGuiTreeNodeFlags_FramePadding)) ? style.FramePadding : ImVec2(style.FramePadding.x, ImMin(window->DC.CurrLineTextBaseOffset, style.FramePadding.y));

        const char* label_end = ImGui::FindRenderedTextEnd(label);
        const ImVec2 label_size = ImGui::CalcTextSize(label, label_end, false);

        // We vertically grow up to current line height up the typical widget height.
        const float frame_height = ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y * 2), label_size.y + padding.y * 2);
        ImRect frame_bb;
        frame_bb.Min.x = (flags & ImGuiTreeNodeFlags_SpanFullWidth) ? window->WorkRect.Min.x : window->DC.CursorPos.x;
        frame_bb.Min.y = window->DC.CursorPos.y;
        frame_bb.Max.x = window->WorkRect.Max.x;
        frame_bb.Max.y = window->DC.CursorPos.y + frame_height;
        if (display_frame) {
            // Framed header expand a little outside the default padding, to the edge of InnerClipRect
            // (FIXME: May remove this at some point and make InnerClipRect align with WindowPadding.x instead of WindowPadding.x*0.5f)
            frame_bb.Min.x -= IM_FLOOR(window->WindowPadding.x * 0.5f - 1.0f);
            frame_bb.Max.x += IM_FLOOR(window->WindowPadding.x * 0.5f);
        }

        const float text_offset_x = g.FontSize + (display_frame ? padding.x * 3 : padding.x * 2);           // Collapser arrow width + Spacing
        const float text_offset_y = ImMax(padding.y, window->DC.CurrLineTextBaseOffset);                    // Latch before ItemSize changes it
        const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);  // Include collapser
        ImVec2 text_pos(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y);
        ImGui::ItemSize(ImVec2(text_width, frame_height), padding.y);

        // For regular tree nodes, we arbitrary allow to click past 2 worth of ItemSpacing
        ImRect interact_bb = frame_bb;
        if (!display_frame && (flags & (ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth)) == 0)
            interact_bb.Max.x = frame_bb.Min.x + text_width + style.ItemSpacing.x * 2.0f;

        // Store a flag for the current depth to tell if we will allow closing this node when navigating one of its child.
        // For this purpose we essentially compare if g.NavIdIsAlive went from 0 to 1 between TreeNode() and TreePop().
        // This is currently only support 32 level deep and we are fine with (1 << Depth) overflowing into a zero.
        const bool is_leaf = (flags & ImGuiTreeNodeFlags_Leaf) != 0;
        bool is_open = ImGui::TreeNodeUpdateNextOpen(id, flags);
        if (is_open && !g.NavIdIsAlive && (flags & ImGuiTreeNodeFlags_NavLeftJumpsBackHere) && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
            window->DC.TreeJumpToParentOnPopMask |= (1 << window->DC.TreeDepth);

        bool item_add = ImGui::ItemAdd(interact_bb, id);
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HasDisplayRect;
        g.LastItemData.DisplayRect = frame_bb;

        if (!item_add) {
            if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
                ImGui::TreePushOverrideID(id);
            IMGUI_TEST_ENGINE_ITEM_INFO(g.LastItemData.ID, label, g.LastItemData.StatusFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
            return is_open;
        }

        ImGuiButtonFlags button_flags = ImGuiTreeNodeFlags_None;
        if (flags & ImGuiTreeNodeFlags_AllowItemOverlap)
            button_flags |= ImGuiButtonFlags_AllowItemOverlap;
        if (!is_leaf)
            button_flags |= ImGuiButtonFlags_PressedOnDragDropHold;

        // We allow clicking on the arrow section with keyboard modifiers held, in order to easily
        // allow browsing a tree while preserving selection with code implementing multi-selection patterns.
        // When clicking on the rest of the tree node we always disallow keyboard modifiers.
        const float arrow_hit_x1 = (text_pos.x - text_offset_x) - style.TouchExtraPadding.x;
        const float arrow_hit_x2 = (text_pos.x - text_offset_x) + (g.FontSize + padding.x * 2.0f) + style.TouchExtraPadding.x;
        const bool is_mouse_x_over_arrow = (g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2);
        if (window != g.HoveredWindow || !is_mouse_x_over_arrow)
            button_flags |= ImGuiButtonFlags_NoKeyModifiers;

        // Open behaviors can be altered with the _OpenOnArrow and _OnOnDoubleClick flags.
        // Some alteration have subtle effects (e.g. toggle on MouseUp vs MouseDown events) due to requirements for multi-selection and drag and drop support.
        // - Single-click on label = Toggle on MouseUp (default, when _OpenOnArrow=0)
        // - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=0)
        // - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=1)
        // - Double-click on label = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1)
        // - Double-click on arrow = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1 and _OpenOnArrow=0)
        // It is rather standard that arrow click react on Down rather than Up.
        // We set ImGuiButtonFlags_PressedOnClickRelease on OpenOnDoubleClick because we want the item to be active on the initial MouseDown in order for drag and drop to work.
        if (is_mouse_x_over_arrow)
            button_flags |= ImGuiButtonFlags_PressedOnClick;
        else if (flags & ImGuiTreeNodeFlags_OpenOnDoubleClick)
            button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
        else
            button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight;

        bool selected = (flags & ImGuiTreeNodeFlags_Selected) != 0;
        const bool was_selected = selected;

        bool hovered, held;
        interact_bb.Max.x += text_offset_x + padding.x;
        bool pressed = ImGui::ButtonBehavior(interact_bb, id, &hovered, &held, button_flags);

        // Is pressed.
        if (is_pressed)
            *is_pressed = pressed;

        bool toggled = false;
        if (!is_leaf) {
            if (pressed && g.DragDropHoldJustPressedId != id) {
                if ((flags & (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)) == 0 || (g.NavActivateId == id))
                    toggled = true;
                if (flags & ImGuiTreeNodeFlags_OpenOnArrow)
                    toggled |= is_mouse_x_over_arrow && !g.NavDisableMouseHover; // Lightweight equivalent of IsMouseHoveringRect() since ButtonBehavior() already did the job
                if ((flags & ImGuiTreeNodeFlags_OpenOnDoubleClick) && g.IO.MouseClickedCount[0] == 2)
                    toggled = true;
            } else if (pressed && g.DragDropHoldJustPressedId == id) {
                IM_ASSERT(button_flags & ImGuiButtonFlags_PressedOnDragDropHold);
                if (!is_open) // When using Drag and Drop "hold to open" we keep the node highlighted after opening, but never close it again.
                    toggled = true;
            }

            if (g.NavId == id && g.NavMoveDir == ImGuiDir_Left && is_open) {
                toggled = true;
                ImGui::NavMoveRequestCancel();
            }
            if (g.NavId == id && g.NavMoveDir == ImGuiDir_Right && !is_open) // If there's something upcoming on the line we may want to give it the priority?
            {
                toggled = true;
                ImGui::NavMoveRequestCancel();
            }

            if (toggled) {
                is_open = !is_open;
                window->DC.StateStorage->SetInt(id, is_open);
                g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledOpen;
            }
        }
        if (flags & ImGuiTreeNodeFlags_AllowItemOverlap)
            ImGui::SetItemAllowOverlap();

        // In this branch, TreeNodeBehavior() cannot toggle the selection so this will never trigger.
        if (selected != was_selected) //-V547
            g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

        // Render
        const ImU32 text_col = ImGui::GetColorU32(ImGuiCol_Text);
        ImGuiNavHighlightFlags nav_highlight_flags = ImGuiNavHighlightFlags_TypeThin;
        if (display_frame) {
            // Framed type
            const ImU32 bg_col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
            ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, true, style.FrameRounding);
            ImGui::RenderNavHighlight(frame_bb, id, nav_highlight_flags);
            if (flags & ImGuiTreeNodeFlags_Bullet)
                ImGui::RenderBullet(window->DrawList, ImVec2(text_pos.x - text_offset_x * 0.60f, text_pos.y + g.FontSize * 0.5f), text_col);
            else if (!is_leaf)
                ImGui::RenderArrow(window->DrawList, ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y), text_col, is_open ? ImGuiDir_Down : ImGuiDir_Right, 1.0f);
            else // Leaf without bullet, left-adjusted text
                text_pos.x -= text_offset_x;
            if (flags & ImGuiTreeNodeFlags_ClipLabelForTrailingButton)
                frame_bb.Max.x -= g.FontSize + style.FramePadding.x;

            if (g.LogEnabled)
                ImGui::LogSetNextTextDecoration("###", "###");
            ImGui::RenderTextClipped(text_pos, frame_bb.Max, label, label_end, &label_size);
        } else {
            // Unframed typed for tree nodes
            if (hovered || selected) {
                const ImU32 bg_col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
                ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, false);
            }
            ImGui::RenderNavHighlight(frame_bb, id, nav_highlight_flags);
            if (flags & ImGuiTreeNodeFlags_Bullet)
                ImGui::RenderBullet(window->DrawList, ImVec2(text_pos.x - text_offset_x * 0.5f, text_pos.y + g.FontSize * 0.5f), text_col);
            else if (!is_leaf)
                ImGui::RenderArrow(window->DrawList, ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y + g.FontSize * 0.15f), text_col, is_open ? ImGuiDir_Down : ImGuiDir_Right, 0.70f);
            if (g.LogEnabled)
                ImGui::LogSetNextTextDecoration(">", NULL);

            // Draw Icon
            if (icon == nullptr) {
                icon = (ImTextureID)(is_open ? _folder_open_icon->id() : _folder_icon->id());
            }
            window->DrawList->AddImage(icon, ImVec2(text_pos.x, text_pos.y - text_offset_y),
                ImVec2(text_pos.x + text_offset_x, text_pos.y - text_offset_y + frame_height), ImVec2(0, 1), ImVec2(1, 0));

            if (icon_width)
                *icon_width = text_offset_x;

            text_pos.x += text_offset_x + padding.x;
            ImGui::RenderText(text_pos, label, label_end, false);
        }

        if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
            ImGui::TreePushOverrideID(id);
        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
        return is_open;
    }

    static inline bool is_keyboard_focus() {
        return ImGui::IsKeyDown(ImGuiKey_LeftArrow)
            || ImGui::IsKeyDown(ImGuiKey_RightArrow)
            || ImGui::IsKeyDown(ImGuiKey_UpArrow)
            || ImGui::IsKeyDown(ImGuiKey_DownArrow);
    }

    void ExplorerWindow::make_tree(const Path& path) {
        String filename = path.filename().string();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding;
        flags |= (!path.is_directory() || path.is_empty()) ? ImGuiTreeNodeFlags_Bullet : 0;
        flags |= _selected_path == path ? ImGuiTreeNodeFlags_Selected : 0;
        const char* label = _rename_path == path ? "##Rename" : filename.data();
        ImTextureID icon = nullptr;
        if (path.extension() == ".fbx") {
            icon = reinterpret_cast<ImTextureID>(_cube_icon->id());
        } else if (path.extension() == ".mesh") {
            icon = reinterpret_cast<ImTextureID>(_mesh_icon->id());
        } else if (!path.is_directory()) {
            icon = reinterpret_cast<ImTextureID>(_file_icon->id());
        }
        float icon_width = 0.0f;
        bool is_pressed = false;
        bool opened = directory_tree(label, flags, icon, &icon_width, &is_pressed);
        if (is_pressed || (is_keyboard_focus() && ImGui::IsItemFocused())) {
            _selected_path = path;
            _directory_path = get_directory_path_from_selected();
        }

        if (ImGui::BeginDragDropSource()) {
            auto path_string = path.string();
            auto extension = path.extension();
            ImGui::SetDragDropPayload("EXPLORER_ITEM", path_string.data(), (path_string.length() + 1), ImGuiCond_Once);
            ImGui::EndDragDropSource();
        }

        handle_popup();

        if (_rename_path == path) {
            char buffer[256];
            strcpy(buffer, filename.data());

            ImGui::PushStyleColor(ImGuiCol_FrameBg, {0.21f, 0.58f, 0.87f, 1.0f});

            ImVec2 text_size = ImGui::CalcTextSize(buffer);
            ImVec2 rect_size = ImGui::GetItemRectSize();
            ImGui::SetKeyboardFocusHere();
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + icon_width);
            ImGui::InputTextEx("##Name", nullptr, buffer, sizeof(buffer),
                {text_size.x + ImGui::GetStyle().FramePadding.x, rect_size.y}, 0);

            ImGui::PopStyleColor();

            if (ImGui::IsKeyDown(ImGuiKey_Enter)) {
                int result = std::rename(path.string().data(), (path + "/" + buffer).string().data());
                UENGINE_ASSERT(!result, "Failed to rename.");
                _rename_path.clear();
            }
        }

        if (opened && path.is_directory()) {
            Directory::for_each(path, [&](DirectoryEntry entry) {
                if (entry.path().filename() != ".uengine" 
                && entry.path().extension() != ".meta") {
                    make_tree(entry.path());
                }
            });
        }

        if (opened) {
            ImGui::TreePop();
        }
    }

#ifdef UENGINE_WINDOWS
#include <Windows.h>
    static bool move_to_recicle_bin(const Path& path) {
        auto string = path.string() + '\0';
        SHFILEOPSTRUCT operation{};
        operation.wFunc = FO_DELETE;
        operation.fFlags = FOF_ALLOWUNDO | FOF_NOERRORUI | FOF_NOCONFIRMATION | FOF_SILENT;
        operation.pFrom = string.data();
        int result = SHFileOperation(&operation);
        return result == 0;
    }
#endif

    void ExplorerWindow::delete_selected() {
        if (_selected_path.is_empty() || _selected_path == _project_path)
            return;
        bool remove_result;
#ifdef UENGINE_WINDOWS
        remove_result = move_to_recicle_bin(_selected_path);
        UENGINE_ASSERT(remove_result, "Failed to delete.");
#else
        Directory::remove(_selected_path);
#endif
        if (_selected_path == _directory_path)
            _directory_path.clear();
        _selected_path.clear();
    }

    void ExplorerWindow::rename_selected() {
        if (_selected_path.is_empty() || _selected_path == _project_path)
            return;

        _rename_path = _selected_path;
    }

    void ExplorerWindow::handle_delete() {
        if (ImGui::IsKeyDown(ImGuiKey_Delete))
            delete_selected();
    }

    void ExplorerWindow::handle_rename() {
        if (ImGui::IsKeyDown(ImGuiKey_Escape) || ((ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && !ImGui::IsAnyItemHovered())) {
            _rename_path.clear();
            return;
        }

        if (ImGui::IsKeyDown(ImGuiKey_F2) && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow))
            rename_selected();
    }

}