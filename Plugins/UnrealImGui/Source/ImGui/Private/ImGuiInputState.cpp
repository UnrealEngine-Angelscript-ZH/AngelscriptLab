// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "ImGuiInputState.h"

#include <algorithm>
#include <limits>
#include <type_traits>


FImGuiInputState::FImGuiInputState()
{
	Reset();
}

void FImGuiInputState::AddCharacter(TCHAR Char)
{
	IOFunctions.AddInputCharacter(ImGuiInterops::CastInputChar(Char));
}

void FImGuiInputState::SetKeyDown(const FKeyEvent& KeyEvent, bool bIsDown)
{
	const FKey& Key = KeyEvent.GetKey();
	SetKeyDown(Key, bIsDown);
}

void FImGuiInputState::SetKeyDown(const FKey& Key, bool bIsDown)
{
	const ImGuiKey& ImKey = ImGuiInterops::UnrealToImGuiKey(Key);
	IOFunctions.AddKeyEvent(ImKey, bIsDown);

	if (ImKey == ImGuiKey_LeftCtrl || ImKey == ImGuiKey_RightCtrl)
	{
		bIsControlDown = bIsDown;
	}
	else if (ImKey == ImGuiKey_LeftShift || ImKey == ImGuiKey_RightShift)
	{
		bIsShiftDown = bIsDown;
	}
	else if (ImKey == ImGuiKey_LeftAlt || ImKey == ImGuiKey_RightAlt)
    {
        bIsAltDown = bIsDown;
    }
	else if (ImKey == ImGuiKey_LeftSuper || ImKey == ImGuiKey_RightSuper)
	{
		bIsCommandDown = bIsDown;
	}

	const ImGuiKey& ImMod = ImGuiInterops::UnrealToImGuiMod(Key);
	if (ImMod != ImGuiKey_None)
	{
		IOFunctions.AddKeyEvent(ImMod, bIsDown);
	}
}

void FImGuiInputState::SetMouseDown(const FPointerEvent& MouseEvent, bool bIsDown)
{
	const uint32 MouseIndex = ImGuiInterops::GetMouseIndex(MouseEvent);
	IOFunctions.AddMouseButtonEvent(MouseIndex, bIsDown);
}

void FImGuiInputState::SetMouseDown(const FKey& MouseButton, bool bIsDown)
{
	const uint32 MouseIndex = ImGuiInterops::GetMouseIndex(MouseButton);
	IOFunctions.AddMouseButtonEvent(MouseIndex, bIsDown);
}

void FImGuiInputState::AddMouseWheelDelta(float DeltaValue)
{
	IOFunctions.AddMouseWheelEvent(0, DeltaValue);
}

void FImGuiInputState::SetMousePosition(const FVector2D& Position)
{
	IOFunctions.AddMousePosEvent(Position.X, Position.Y);
	MousePosition = Position;
}

void FImGuiInputState::SetMousePointer(bool bInHasMousePointer)
{
	IOFunctions.MouseDrawCursor = bInHasMousePointer;
	bHasMousePointer = bInHasMousePointer;
}

void FImGuiInputState::SetTouchDown(bool bIsDown)
{
	IOFunctions.AddMouseButtonEvent(0, bIsDown);
	bTouchDown = bIsDown;
}

void FImGuiInputState::SetTouchPosition(const FVector2D& Position)
{
	IOFunctions.AddMousePosEvent(Position.X, Position.Y);
}

void FImGuiInputState::SetGamepadNavigationAxis(const FAnalogInputEvent& AnalogInputEvent, float Value)
{
	ImGuiInterops::SetGamepadNavigationAxis(IOFunctions, AnalogInputEvent.GetKey(), Value);
}

void FImGuiInputState::SetKeyboardNavigationEnabled(bool bEnabled)
{
	bKeyboardNavigationEnabled = bEnabled;
}

void FImGuiInputState::SetGamepadNavigationEnabled(bool bEnabled)
{
	bGamepadNavigationEnabled = bEnabled;
}

void FImGuiInputState::SetGamepad(bool bInHasGamepad)
{
	bHasGamepad = bInHasGamepad;
}

void FImGuiInputState::ClearUpdateState()
{
	bTouchProcessed = bTouchDown;
}

void FImGuiInputState::ClearMouseAnalogue()
{
	MousePosition = FVector2D::ZeroVector;
}

void FImGuiInputState::ClearModifierKeys()
{
	bIsControlDown = false;
	bIsShiftDown = false;
	bIsAltDown = false;
	bIsCommandDown = false;
}

