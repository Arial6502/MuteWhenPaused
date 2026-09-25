#include "Util/Logger/Logger.hpp"
#include "Hooks/Hooks.hpp"
#include "Version.hpp"


SKSEPluginLoad(const SKSE::LoadInterface * a_SKSE) {

	//Fix Clib null base img address due to the static initialization order fiasco issue.
	REL::Module::reset(); 

	SKSE::Init(a_SKSE);
	logger::Initialize();
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
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary,
);