
#include "CoreMinimal.h"
#include "AngelscriptBinds.h"

#include "imgui.h"


namespace AngelscriptImGui
{
	bool Begin(const FString& Name)
	{
		const char* NameAnsi = TCHAR_TO_UTF8(*Name);
		return ImGui::Begin(NameAnsi);
	}
	
	bool BeginChild(const FString& Name)
	{
		const char* NameAnsi = TCHAR_TO_UTF8(*Name);
		return ImGui::BeginChild(NameAnsi);
	}

	void Text(const FString& Name)
	{
		const char* NameAnsi = TCHAR_TO_UTF8(*Name);
		ImGui::Text(NameAnsi);
	}
}

AS_FORCE_LINK const FAngelscriptBinds::FBind Bind_ImGui(FAngelscriptBinds::EOrder::Late, []
{
		
	FAngelscriptBinds::FNamespace ns("ImGui");

	// Windows
	// - Begin() = push window to the stack and start appending to it. End() = pop window from the stack.
	// - Passing 'bool* p_open != NULL' shows a window-closing widget in the upper-right corner of the window,
	//   which clicking will set the boolean to false when clicked.
	// - You may append multiple times to the same window during the same frame by calling Begin()/End() pairs multiple times.
	//   Some information such as 'flags' or 'p_open' will only be considered by the first call to Begin().
	// - Begin() return false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting
	//   anything to the window. Always call a matching End() for each Begin() call, regardless of its return value!
	//   [Important: due to legacy reason, Begin/End and BeginChild/EndChild are inconsistent with all other functions
	//    such as BeginMenu/EndMenu, BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding
	//    BeginXXX function returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]
	// - Note that the bottom of window stack always contains a window called "Debug".
	FAngelscriptBinds::BindGlobalFunction("bool Begin(const FString& Name)", FUNC_TRIVIAL(AngelscriptImGui::Begin));
	FAngelscriptBinds::BindGlobalFunction("void End()", FUNC_TRIVIAL(ImGui::End));

	
	// Child Windows
	// - Use child windows to begin into a self-contained independent scrolling/clipping regions within a host window. Child windows can embed their own child.
	// - Before 1.90 (November 2023), the "ImGuiChildFlags child_flags = 0" parameter was "bool border = false".
	//   This API is backward compatible with old code, as we guarantee that ImGuiChildFlags_Borders == true.
	//   Consider updating your old code:
	//      BeginChild("Name", size, false)   -> Begin("Name", size, 0); or Begin("Name", size, ImGuiChildFlags_None);
	//      BeginChild("Name", size, true)    -> Begin("Name", size, ImGuiChildFlags_Borders);
	// - Manual sizing (each axis can use a different setting e.g. ImVec2(0.0f, 400.0f)):
	//     == 0.0f: use remaining parent window size for this axis.
	//      > 0.0f: use specified size for this axis.
	//      < 0.0f: right/bottom-align to specified distance from available content boundaries.
	// - Specifying ImGuiChildFlags_AutoResizeX or ImGuiChildFlags_AutoResizeY makes the sizing automatic based on child contents.
	//   Combining both ImGuiChildFlags_AutoResizeX _and_ ImGuiChildFlags_AutoResizeY defeats purpose of a scrolling region and is NOT recommended.
	// - BeginChild() returns false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting
	//   anything to the window. Always call a matching EndChild() for each BeginChild() call, regardless of its return value.
	//   [Important: due to legacy reason, Begin/End and BeginChild/EndChild are inconsistent with all other functions
	//    such as BeginMenu/EndMenu, BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding
	//    BeginXXX function returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]
	FAngelscriptBinds::BindGlobalFunction("bool BeginChild(const FString& Name)", FUNC_TRIVIAL(AngelscriptImGui::BeginChild));
	FAngelscriptBinds::BindGlobalFunction("void EndChild()", FUNC_TRIVIAL(ImGui::EndChild));
	


	
	FAngelscriptBinds::BindGlobalFunction("void Text(const FString& Name)", FUNC_TRIVIAL(AngelscriptImGui::Text));
	
	


});