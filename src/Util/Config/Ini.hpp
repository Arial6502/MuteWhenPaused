#pragma once

#include <SimpleIni.h>

//Reflection driven INI (de)serialization of flat settings structs.
//
//	struct General {
//		INI_SECTION("General");
//		bool bEnabled = true;
//		int32_t iCount = 3;
//		float fScale = 1.0f;
//		std::string sName = "Foo";
//	};
//
//	General general;
//	Util::Config::Load(Util::Config::DefaultPath(), general);
//
//Members become keys under the named section. Missing or malformed keys keep their default,
//and Load writes the file back so new members appear in existing configs.

//Static members are invisible to reflect, so this does not add a key.
#define INI_SECTION(a_name) static constexpr const char* IniSection = a_name

namespace Util::Config {

	template <class T>
	concept IniStruct = std::is_aggregate_v<T> && requires {
		{ T::IniSection } -> std::convertible_to<const char*>;
	};

	namespace detail {

		template <class V>
		inline constexpr bool IsChar = std::is_same_v<V, char> || std::is_same_v<V, wchar_t> || std::is_same_v<V, char8_t> || std::is_same_v<V, char16_t> || std::is_same_v<V, char32_t>;

		template <class V>
		inline constexpr bool IsIniValue = std::is_same_v<V, std::string> || (std::is_arithmetic_v<V> && !IsChar<V>);

		template <class V>
		bool Parse(std::string_view a_str, V& a_out) {
			V parsed{};
			const auto [ptr, ec] = std::from_chars(a_str.data(), a_str.data() + a_str.size(), parsed);
			if (ec != std::errc{} || ptr != a_str.data() + a_str.size()) {
				return false;
			}
			a_out = parsed;
			return true;
		}

		inline bool Open(CSimpleIniA& a_ini, const std::filesystem::path& a_path) {
			std::error_code ec;
			if (!std::filesystem::exists(a_path, ec)) {
				return true;
			}
			if (a_ini.LoadFile(a_path.c_str()) < 0) {
				logger::error("INI: could not read {}", a_path.string());
				return false;
			}
			return true;
		}

		inline bool Commit(const CSimpleIniA& a_ini, const std::filesystem::path& a_path) {
			std::error_code ec;
			std::filesystem::create_directories(a_path.parent_path(), ec);
			if (a_ini.SaveFile(a_path.c_str(), false) < 0) {
				logger::error("INI: could not write {}", a_path.string());
				return false;
			}
			return true;
		}
	}

	template <IniStruct T>
	void Read(const CSimpleIniA& a_ini, T& a_out) {
		reflect::for_each([&](const auto I) {
			auto& value = reflect::get<I>(a_out);
			using V = std::remove_cvref_t<decltype(value)>;
			static_assert(detail::IsIniValue<V>, "INI members must be bool, a number or std::string");

			const std::string key{ reflect::member_name<I, T>() };
			const char* raw = a_ini.GetValue(T::IniSection, key.c_str(), nullptr);
			if (!raw) {
				return;
			}

			if constexpr (std::is_same_v<V, std::string>) {
				value = raw;
			}
			else if constexpr (std::is_same_v<V, bool>) {
				value = a_ini.GetBoolValue(T::IniSection, key.c_str(), value);
			}
			else if (!detail::Parse(raw, value)) {
				logger::warn("INI: [{}] {} = \"{}\" is not a valid {}, keeping {}", T::IniSection, key, raw, reflect::type_name<V>(), value);
			}
		}, a_out);
	}

	template <IniStruct T>
	void Write(CSimpleIniA& a_ini, const T& a_in) {
		reflect::for_each([&](const auto I) {
			const auto& value = reflect::get<I>(a_in);
			using V = std::remove_cvref_t<decltype(value)>;
			static_assert(detail::IsIniValue<V>, "INI members must be bool, a number or std::string");

			const std::string key{ reflect::member_name<I, T>() };
			if constexpr (std::is_same_v<V, std::string>) {
				a_ini.SetValue(T::IniSection, key.c_str(), value.c_str());
			}
			else if constexpr (std::is_same_v<V, bool>) {
				a_ini.SetBoolValue(T::IniSection, key.c_str(), value);
			}
			else {
				a_ini.SetValue(T::IniSection, key.c_str(), std::format("{}", value).c_str());
			}
		}, a_in);
	}

	//Data/SKSE/Plugins/<PluginName>.ini, where the plugin name is the DLL name set in CMakeLists.txt.
	inline std::filesystem::path DefaultPath() {
		return std::filesystem::path("Data/SKSE/Plugins") / (std::string(SKSE::PluginDeclaration::GetSingleton()->GetName()) + ".ini");
	}

	//An unreadable existing file is left untouched rather than overwritten with defaults.
	template <IniStruct... T>
	bool Load(const std::filesystem::path& a_path, T&... a_out) {
		CSimpleIniA ini(true);
		if (!detail::Open(ini, a_path)) {
			return false;
		}
		(Read(ini, a_out), ...);
		(Write(ini, a_out), ...);
		return detail::Commit(ini, a_path);
	}

	//Merges into the existing file so comments and unrelated sections survive.
	template <IniStruct... T>
	bool Save(const std::filesystem::path& a_path, const T&... a_in) {
		CSimpleIniA ini(true);
		if (!detail::Open(ini, a_path)) {
			return false;
		}
		(Write(ini, a_in), ...);
		return detail::Commit(ini, a_path);
	}
}
