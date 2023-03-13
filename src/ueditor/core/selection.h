#pragma once
#ifndef UEDITOR_CORE_SELECTION
#define UEDITOR_CORE_SELECTION

#include "ueditor/core/editor_window.h"

#include <uengine/core/object.h>

namespace ueditor {
    /**
     * @brief Provides access to the selection.
     */
    class Selection {
    public:
        static Object* context();

        static Object* object();

        static void context(Object* value);

        static void object(Object* value);

        static void object_with_context(Object* object, Object* context);
    private:
        static Object* _context;
        static Object* _object;

        static void flush();

        friend class EditorApplication;
    };
}

#endif