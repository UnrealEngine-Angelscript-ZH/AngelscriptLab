// Fill out your copyright notice in the Description page of Project Settings.
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/CoreDelegates.h"
#include "HAL/FileManager.h"

#include "AngelscriptBytecodeView.h"

#include <angelscript.h>
#include <as_scriptengine.h>

#include "AngelscriptManager.h"
#include "imgui.h"


// Sets default values
AAngelscriptBytecodeView::AAngelscriptBytecodeView()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAngelscriptBytecodeView::BeginPlay()
{
	
}

extern FString GetBytecode(asIScriptFunction* func);

// Called every frame
void AAngelscriptBytecodeView::Tick(float DeltaTime)
{
	auto& Manager = FAngelscriptManager::Get();
	auto asEngine = Manager.Engine;
	
	static FString SelectedModule;
	static FString SelectedFunction;
	static TArray<FString> ModuleNames;
	static TMap<FString, TArray<FString>> ModuleFunctions;
	static char FuncFilter[128] = "";
	static FString BytecodeContent;

	ImGui::Begin("AngelScript Bytecode Viewer");
	ImGui::Columns(2, "MainColumns", true);

	// 左边栏：模块和函数选择
	{
		// 获取所有模块
		if (ModuleNames.IsEmpty() && asEngine)
		{
			const asUINT modCount = asEngine->GetModuleCount();
			for (asUINT i = 0; i < modCount; ++i)
			{
				if (asIScriptModule* mod = asEngine->GetModuleByIndex(i))
				{
					FString Name = UTF8_TO_TCHAR(mod->GetName());
					ModuleNames.Add(Name);
				}
			}
		}

		// 模块下拉选择
		if (ImGui::BeginCombo("Modules", TCHAR_TO_ANSI(*SelectedModule)))
		{
			for (const FString& Name : ModuleNames)
			{
				if (ImGui::Selectable(TCHAR_TO_ANSI(*Name)))
				{
					SelectedModule = Name;
					// 加载模块函数
					if (asIScriptModule* mod = asEngine->GetModuleByIndex(ModuleNames.IndexOfByKey(Name)))
					{
						TArray<FString> Functions;
						const asUINT funcCount = mod->GetFunctionCount();
						for (asUINT j = 0; j < funcCount; ++j)
						{
							if (asIScriptFunction* func = mod->GetFunctionByIndex(j))
							{
								Functions.Add(UTF8_TO_TCHAR(func->GetName()));
							}
						}
						ModuleFunctions.Add(Name, Functions);
					}
				}
			}
			ImGui::EndCombo();
		}

		// 函数过滤
		ImGui::InputText("Filter Functions", FuncFilter, IM_ARRAYSIZE(FuncFilter));
		const FString FilterStr = FString(UTF8_TO_TCHAR(FuncFilter)).ToLower();

		// 函数列表
		ImGui::BeginChild("FunctionList", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
		if (!SelectedModule.IsEmpty() && ModuleFunctions.Contains(SelectedModule))
		{
			const TArray<FString>& Functions = ModuleFunctions[SelectedModule];
			
			ImGuiListClipper clipper;
			clipper.Begin(Functions.Num());
			
			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
				{
					const FString& FuncName = Functions[i];
					if (!FilterStr.IsEmpty() && !FuncName.ToLower().Contains(FilterStr))
						continue;

					if (ImGui::Selectable(TCHAR_TO_ANSI(*FuncName), SelectedFunction == FuncName))
					{
						SelectedFunction = FuncName;
						// 获取字节码
						if (asIScriptModule* mod = asEngine->GetModuleByIndex(ModuleNames.IndexOfByKey(SelectedModule)))
						{
							if (asIScriptFunction* func = mod->GetFunctionByIndex(i))
							{
								BytecodeContent = GetBytecode(func);
							}
						}
					}
				}
			}
		}
		ImGui::EndChild();
	}

	ImGui::NextColumn();

	// 右边栏：字节码显示
	{
		ImGui::BeginChild("BytecodeView", ImVec2(0, 0), true);
		if (!BytecodeContent.IsEmpty())
		{
			ImGui::TextUnformatted(TCHAR_TO_ANSI(*BytecodeContent));
		}
		else
		{
			ImGui::TextColored(ImVec4(1,1,0,1), "Select a function to view bytecode");
		}
		ImGui::EndChild();
	}

	ImGui::Columns(1);
	ImGui::End();

}

