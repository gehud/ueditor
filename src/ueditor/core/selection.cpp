#include "UEditor/core/selection.h"

namespace uengine {
    void* Selection::_context = nullptr;

    void* Selection::context() {
        return _context;
    }
}