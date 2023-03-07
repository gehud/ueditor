#pragma once
#ifndef UEDITOR_CORE_SELECTION
#define UEDITOR_CORE_SELECTION

namespace uengine {
    class Selection {
    public:
        static void* context();
    private:
        static void* _context;
    };
}

#endif