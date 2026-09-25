#include "Settings/Settings.hpp"
#include "Util/Logger/Logger.hpp"

namespace Settings {

	void Load() {
		Util::Config::Load(Util::Config::DefaultPath(), logging);
		logger::SetLevel(logging.sLevel.c_str());
	}

	void Save() {
		Util::Config::Save(Util::Config::DefaultPath(), logging);
	}

	void Panel::Draw() {
		static constexpr const char* levels[] = { "Trace", "Debug", "Info", "Warn", "Error", "Critical", "Off" };

		const auto it = std::ranges::find_if(levels, [](const char* a_level) {
			return Util::Text::EqualsInvariantStr(a_level, logging.sLevel);
		});
		int current = it != std::end(levels) ? static_cast<int>(it - std::begin(levels)) : 2;

		if (ImGui::Combo("Log Level", &current, levels, IM_ARRAYSIZE(levels))) {
			logging.sLevel = levels[current];
			logger::SetLevel(logging.sLevel.c_str());
		}

		if (ImGui::Button("Save")) {
			Save();
		}
	}
}
