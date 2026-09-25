#include "Logger.hpp"
#include "Util/Text/Text.hpp"
#include "Util/Windows/MessageBox.hpp"

namespace {

	//ANSI Escape Codes: Colors
	#define BLK "\033[30m"  //Black
	#define RED "\033[31m"  //Red
	#define GRN "\033[32m"  //Green
	#define YEL "\033[33m"  //Yellow
	#define BLU "\033[34m"  //Blue
	#define MAG "\033[35m"  //Magenta
	#define CYA "\033[36m"  //Cyan
	#define WHT "\033[37m"  //White

	//ANSI Escape Codes: Bright Colors
	#define GRY "\033[90m"  //Bright Black
	#define BRED "\033[91m"
	#define BGRN "\033[92m"
	#define BYEL "\033[93m"
	#define BBLU "\033[94m"
	#define BMAG "\033[95m"
	#define BCYA "\033[96m"
	#define BWHT "\033[97m"

	//ANSI Escape Codes: Background Colors
	#define BG_BLK "\033[40m"
	#define BG_RED "\033[41m"
	#define BG_GRN "\033[42m"
	#define BG_YEL "\033[43m"
	#define BG_BLU "\033[44m"
	#define BG_MAG "\033[45m"
	#define BG_CYA "\033[46m"
	#define BG_WHT "\033[47m"
	#define BG_GRY "\033[100m"

	//ANSI Escape Codes: 256 Color Palette and Truecolor. Concatenate with string literals, e.g. FG256("208")
	#define FG256(n) "\033[38;5;" n "m"
	#define BG256(n) "\033[48;5;" n "m"
	#define FGRGB(r, g, b) "\033[38;2;" r ";" g ";" b "m"
	#define BGRGB(r, g, b) "\033[48;2;" r ";" g ";" b "m"

	//Palette picks
	#define ORG FG256("208")  //Orange
	#define PNK FG256("213")  //Pink
	#define PRP FG256("141")  //Purple
	#define TEA FG256("37")   //Teal
	#define LIM FG256("154")  //Lime
	#define GLD FG256("220")  //Gold

	//ANSI Escape Codes: Formating
	#define RST "\033[0m"   //Reset
	#define BLD "\033[1m"   //Bold
	#define DIM "\033[2m"   //Dim
	#define ITL "\033[3m"   //Italic
	#define UDL "\033[4m"   //Underline
	#define REV "\033[7m"   //Reverse
	#define STK "\033[9m"   //Strikethrough

	//Logger Fmt
	#define LOG_HDR "[" BCYA "PLUG" WHT "]"


	constexpr const char* PatternDefault = "[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v";
	constexpr const char* PatternConsole = WHT LOG_HDR"[%H:%M:%S.%e][%^%l%$][" YEL "%s:%#" WHT "]: %v" RST;

}

namespace SKSE::log {

	void Initialize() {

		auto path = log_directory();

		if (!path) {
			Util::Win32::ReportAndExit("Could not find a valid log directory.");
		}

		*path /= PluginDeclaration::GetSingleton()->GetName();
		*path += L".log";

		std::shared_ptr <spdlog::logger> logger;

		if (HasConsole()) {
			auto sink = std::make_shared<spdlog::sinks::stdout_sink_st>();

			spdlog::init_thread_pool(8192, 1);

			logger = std::make_shared<spdlog::async_logger>("Global", sink, spdlog::thread_pool(), spdlog::async_overflow_policy::overrun_oldest);

			logger->set_pattern(PatternConsole);
			logger->flush_on(spdlog::level::off);
		}
		/*else if (IsDebuggerPresent()) {
			logger = std::make_shared <spdlog::logger>("Global", std::make_shared <spdlog::sinks::msvc_sink_mt>());
			logger->set_pattern(PatternDefault);
		}*/
		else {
			logger = std::make_shared<spdlog::logger>(
				"Global",
				std::make_shared<spdlog::sinks::basic_file_sink_mt>(
					path->string(), true));

			logger->set_pattern(PatternDefault);
			logger->flush_on(spdlog::level::err);
		}

		spdlog::set_default_logger(std::move(logger));
		SetLevel(HasConsole() ? spdlog::level::trace : spdlog::level::info);
	}

	void SetLevel(spdlog::level::level_enum a_level) {
		spdlog::set_level(a_level);
	}

	void SetLevel(const char* a_level) {

		const auto to_level_enum = [](const char* levelStr) -> std::optional<spdlog::level::level_enum> {
			using enum spdlog::level::level_enum;

			std::string lower = Util::Text::ToLower(levelStr);
			if (lower == "off")                         return off;
			if (lower == "trace")                       return trace;
			if (lower == "debug")                       return debug;
			if (lower == "info")                        return info;
			if (lower == "warning" || lower == "warn")  return warn;
			if (lower == "error" || lower == "err")     return err;
			if (lower == "critical")                    return critical;

			return std::nullopt;
		};

		if (const auto level = to_level_enum(a_level)) {
			SetLevel(*level);
		}
	}
}