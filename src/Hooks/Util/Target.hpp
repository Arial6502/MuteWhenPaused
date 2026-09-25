#pragma once

namespace Hooks {

	//Address library epochs.
	enum class Epoch : std::uint8_t {
		k1_5,
		k1_6,
		k1_7,
		kVR
	};

	[[nodiscard]] constexpr std::string_view ToString(Epoch a_epoch) noexcept {
		switch (a_epoch) {
			case Epoch::k1_5: return "1.5";
			case Epoch::k1_6: return "1.6";
			case Epoch::k1_7: return "1.7";
			case Epoch::kVR:  return "VR";
		}
		return "?";
	}

	[[nodiscard]] inline Epoch CurrentEpoch() noexcept {
		if (REL::Module::IsVR()) {
			return Epoch::kVR;
		}
		const auto minor = REL::Module::get().version().minor();
		return minor >= 7 ? Epoch::k1_7 : minor == 6 ? Epoch::k1_6 : Epoch::k1_5;
	}

	namespace detail {

		//One value per epoch, ordered (1.5, 1.6, 1.7, VR). Omitted values fall back: 1.7 to 1.6, VR to 1.5.
		template <class V>
		class PerEpoch {
		public:
			constexpr PerEpoch() noexcept = default;
			constexpr explicit PerEpoch(V a_all) noexcept : m_values{ a_all, a_all, a_all, a_all } {}
			constexpr PerEpoch(V a_15, V a_16) noexcept : m_values{ a_15, a_16, a_16, a_15 } {}
			constexpr PerEpoch(V a_15, V a_16, V a_17) noexcept : m_values{ a_15, a_16, a_17, a_15 } {}
			constexpr PerEpoch(V a_15, V a_16, V a_17, V a_vr) noexcept : m_values{ a_15, a_16, a_17, a_vr } {}

			[[nodiscard]] constexpr V Get(Epoch a_epoch) const noexcept {
				return m_values[std::to_underlying(a_epoch)];
			}

		private:
			std::array<V, 4> m_values{};
		};
	}

	struct RelocationEx : detail::PerEpoch<std::uint64_t> {
		using detail::PerEpoch<std::uint64_t>::PerEpoch;
	};

	//Byte offsets added to the RelocationEx address, same order and fallbacks.
	struct OffsetEx : detail::PerEpoch<std::size_t> {
		using detail::PerEpoch<std::size_t>::PerEpoch;
	};

	//Restricts a hook to one epoch or one exact game version (major.minor.patch). Default: every runtime.
	class When {
	public:
		constexpr When() noexcept = default;
		constexpr When(Epoch a_epoch) noexcept : m_epoch(a_epoch) {}
		constexpr When(REL::Version a_version) noexcept : m_version(a_version) {}

		[[nodiscard]] bool Matches() const {
			if (m_epoch) {
				return *m_epoch == CurrentEpoch();
			}
			if (m_version) {
				const auto running = REL::Module::get().version();
				return running.major() == m_version->major() && running.minor() == m_version->minor() && running.patch() == m_version->patch();
			}
			return true;
		}

		[[nodiscard]] std::string Describe() const {
			if (m_epoch) {
				return std::string(ToString(*m_epoch));
			}
			if (m_version) {
				return m_version->string(".");
			}
			return "any runtime";
		}

	private:
		std::optional<Epoch> m_epoch;
		std::optional<REL::Version> m_version;
	};
}
