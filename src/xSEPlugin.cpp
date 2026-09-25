#include "Util/Logger/Logger.hpp"
#include "Hooks/Hooks.hpp"
#include "Settings/Settings.hpp"
#include "Version.hpp"

namespace {

	void OnMessage(SKSE::MessagingInterface::Message* a_message) {
		if (a_message->type == SKSE::MessagingInterface::kDataLoaded) {
			Util::UI::UIItemRegistry::Install();
		}
	}
}

SKSEPluginLoad(const SKSE::LoadInterface * a_SKSE) {

	//Fix Clib null base img address due to the static initialization order fiasco issue.
	REL::Module::reset(); 

	SKSE::Init(a_SKSE);
	logger::Initialize();
	Settings::Load();

	if (!SKSE::GetMessagingInterface()->RegisterListener(OnMessage)) {
		Util::Win32::ReportAndExit("Unable to register message listener.");
	}

	Settings::Panel::RegisterUI();

	SKSE::GetTrampoline().create(Const::Plugin::TRAMPOLINE_ALLOC_BYTES);
	Hooks::Install();

	logger::info("SKSEPluginLoad OK");

	return true;
}

SKSEPluginInfo(
	.Version = Plugin::ModVersion,
	.Name = Plugin::ModName,
	.Author = "Arial6502",
	.StructCompatibility = SKSE::StructCompatibility::Independent,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary
);