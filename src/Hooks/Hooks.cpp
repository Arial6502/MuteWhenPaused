#include "Hooks/Hooks.hpp"
#include "Hooks/Util/HookUtil.hpp"

namespace Hooks {

	//-----------------
	// UPDATE
	//-----------------
	namespace {

		struct MainUpdateNullSub {

			static void thunk(RE::Main* a_this, float a_deltaTime) {
				func(a_this, a_deltaTime);
			}

			FUNCTYPE_CALL func;

		};

	}

	void Install() {
		stl::write_call<MainUpdateNullSub>(RelocationEx(35565, 36564), OffsetEx(0x748, 0xC26));
	}

}