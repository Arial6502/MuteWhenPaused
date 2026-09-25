#pragma once

#include "Util/UI/UIRegistry.hpp"

namespace Settings {

	struct Logging {
		INI_SECTION("Logging");
		std::string sLevel = "Info";
	};

	inline Logging logging;

	//Creates the INI with defaults if it does not exist yet.
	void Load();
	void Save();

	struct Panel : Util::UI::UIEntry<Panel> {
		static constexpr std::string_view UICategoryName = "Settings";
		static void Draw();
	};
}
