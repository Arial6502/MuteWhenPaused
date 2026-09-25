#include "Util/Windows/MessageBox.hpp"
#include "Util/Text/Text.hpp"

namespace {

	HWND GetHandle() {

		const RE::BSGraphics::Renderer* renderer = RE::BSGraphics::Renderer::GetSingleton();
		if (!renderer) {
			return nullptr;
		}

		const RE::BSGraphics::RendererWindow* window = RE::BSGraphics::Renderer::GetCurrentRenderWindow();
		if (!window) {
			return nullptr;
		}

		return reinterpret_cast<HWND>(window->hWnd);
	}

	void Show(const std::wstring& a_message, UINT a_icon) {
		const std::wstring title = Util::Text::Utf8ToUtf16(SKSE::PluginDeclaration::GetSingleton()->GetName()) + L".dll";

		while (ShowCursor(TRUE) < 0) {}

		MessageBoxW(GetHandle(), a_message.c_str(), title.c_str(), MB_OK | a_icon | MB_TOPMOST | MB_SETFOREGROUND);

		while (ShowCursor(FALSE) >= 0) {}
	}

	constexpr std::wstring_view CloseMsg = L"\nThe game will now close.";
}

namespace Util::Win32 {

	void ReportAndExit(std::string_view a_message) {
		ReportAndExit(Util::Text::Utf8ToUtf16(a_message));
	}

	void ReportAndExit(std::wstring_view a_message) {
		Show(std::wstring(a_message) + std::wstring(CloseMsg), MB_ICONERROR);
		REX::W32::TerminateProcess(REX::W32::GetCurrentProcess(), EXIT_FAILURE);
	}

	void ReportInfo(std::string_view a_message) {
		ReportInfo(Util::Text::Utf8ToUtf16(a_message));
	}

	void ReportInfo(std::wstring_view a_message) {
		Show(std::wstring(a_message), MB_ICONINFORMATION);
	}
}
