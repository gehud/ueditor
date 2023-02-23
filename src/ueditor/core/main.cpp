#include <uengine.h>
#include <uengine/core/entry.h>

using namespace uengine;

class editor_application : public application {

};

namespace uengine {
	application* create_application() {
		return new editor_application();
	}
}