#include "Hooks/Hooks.hpp"
#include "Hooks/Util/HookUtil.hpp"

namespace Hooks {

	namespace {

		//BSGameSound::flags bit 10: nothing in the exe sets it, the volume calc function (67873) does force -10000 dB when it is set though.
		constexpr auto SoundMuted = static_cast<RE::BSGameSound::Flags>(1 << 10);
		constexpr std::uint32_t SoundTypeLoop = 0x3800;
		constexpr std::uint32_t SoundTypeMusic = 0x20;

		__forceinline void MuteLoopingSounds(RE::BSAudioManager* a_manager, std::uint32_t a_newestID) {
			for (const auto& [id, sound] : a_manager->activeSounds) {
				if (!sound || id > a_newestID) {
					continue;
				}

				const std::uint32_t type = sound->soundType.underlying();
				if ((type & SoundTypeLoop) == 0 || (type & SoundTypeMusic) != 0) {
					continue;
				}

				sound->flags.set(SoundMuted);
				sound->SetVolumeImpl();
			}
		}

		__forceinline void UnmuteSounds(RE::BSAudioManager* a_manager) {
			for (const auto& [id, sound] : a_manager->activeSounds) {
				if (sound && sound->flags.all(SoundMuted)) {
					sound->flags.reset(SoundMuted);
					sound->SetVolumeImpl();
				}
			}
		}

		//Audio thread stuff. Menu pause only reaches sounds in the kPauseDuringMenuCategory* trees, so loops outside them keep going.
		static void UpdatePausedLoopMute(RE::BSAudioManager* a_manager) {
			//TODO: VR
			static const REL::Relocation<std::uint32_t*> lastSoundID { RelocationIDEx(523570, 410106, 410106, NULL) };

			static constinit std::uint32_t s_lastUnpausedID = 0;
			static constinit bool s_muted = false;

			//Counter before pause, so an ID read here always predates the pause.
			const std::uint32_t newestID = *lastSoundID;
			RE::UI* const ui = RE::UI::GetSingleton();
			const bool paused = ui && ui->GameIsPaused();

			if (!paused) {
				if (s_muted) {
					UnmuteSounds(a_manager);
					s_muted = false;
				}
				s_lastUnpausedID = newestID;
				return;
			}

			if (!s_muted) {
				MuteLoopingSounds(a_manager, s_lastUnpausedID);
				s_muted = true;
			}
		}

		//BSAudioManagerThread::ThreadProc -> BSAudioManager per-sound update, once per audio tick.
		struct AudioUpdateSounds {

			static void thunk(RE::BSAudioManager* a_this) {
				func(a_this);
				if (a_this) UpdatePausedLoopMute(a_this);
			}

			FUNCTYPE_CALL func;
		};

	}

	void Install() {
		//SE, AE16, AE17, VR
		//TODO: VR
		stl::write_call<AudioUpdateSounds>(RelocationIDEx(66482, 67746, 67746, NULL), OffsetEx(0x6B, 0x6C, 0x6C, NULL));
	}

}