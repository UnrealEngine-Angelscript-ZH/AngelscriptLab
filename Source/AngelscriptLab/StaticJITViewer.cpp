// Fill out your copyright notice in the Description page of Project Settings.

#include "StaticJITViewer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/CoreDelegates.h"
#include "HAL/FileManager.h"


#define STATIC_JIT_DEBUG 0
// need some engine change
// struct ANGELSCRIPTCODE_API FAngelscriptStaticJIT : public asIJITCompiler
// struct ANGELSCRIPTCODE_API FAngelscriptPrecompiledData
// AngelscriptCode.Build.cs - PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

#if STATIC_JIT_DEBUG

#include <as_scriptengine.h>

#include "AngelscriptManager.h"
#include "imgui.h"
#include "StaticJIT/AngelscriptStaticJIT.h"
#include "StaticJIT/PrecompiledData.h"

#if AS_CAN_GENERATE_JIT
const FString STANDARD_HEADER =
TEXT("#include \"StaticJIT/StaticJITConfig.h\"\n")
#if UE_BUILD_DEBUG
TEXT("#if UE_BUILD_DEBUG\n")
#elif UE_BUILD_DEVELOPMENT
TEXT("#if UE_BUILD_DEVELOPMENT\n")
#elif UE_BUILD_TEST
TEXT("#if UE_BUILD_TEST\n")
#elif UE_BUILD_SHIPPING
TEXT("#if UE_BUILD_SHIPPING\n")
#endif
TEXT("#ifndef AS_SKIP_JITTED_CODE\n\n")
;

const FString STANDARD_INCLUDES =
TEXT("#include \"StaticJIT/StaticJITHeader.h\"\n")
;

const FString STANDARD_FOOTER =
TEXT("#endif\n")
TEXT("#endif\n")
;
#endif

struct FAngelscriptStaticJIT_Debug : public FAngelscriptStaticJIT
{
	FString GetCPPJITCode(asIScriptFunction* ScriptFunction)
	{
		FString Code;

		Engine = (asCScriptEngine*)FAngelscriptManager::Get().Engine;
		bAllowDevirtualize = true;
		bAllowComprehensiveJIT = true;

		// Detect all script types in the engine
		for (int32 i = 0, Count = Engine->scriptModules.GetLength(); i < Count; ++i)
		{
			auto* Module = Engine->scriptModules[i];
			if (Module == nullptr)
				continue;
			for (int32 j = 0, jCount = Module->classTypes.GetLength(); j < jCount; ++j)
			{
				asCObjectType* objType = Module->classTypes[j];
				if (objType == nullptr)
					continue;
				DetectScriptType(objType);
			}
		}

		// Pre-Analyze the functions we're going to generate
		for (auto& Elem : FunctionsToGenerate)
			AnalyzeScriptFunction(Elem.Key, Elem.Value);

		// Run all scriptfunctions through the generate process
		for (auto& Elem : FunctionsToGenerate)
			GenerateCppCode(Elem.Key, Elem.Value);

		FString GenDir = FPaths::RootDir() / TEXT("AS_JITTED_CODE");

		// Delete and recreate the folder so we don't keep any old code
		auto& FileManager = IFileManager::Get();
		FileManager.MakeDirectory(*GenDir, true);

		TArray<FString> PreviousFiles;
		FileManager.FindFiles(PreviousFiles, *GenDir);

		TSet<FString> CurrentFiles;

		int32 CombinedFileIndex = 0;
		int32 LinesInCombinedFile = 0;

		TArray<FString> FilesToCombine;

		// Mark which shader headers are used by more than one module
		for (auto ModuleElem : JITFiles)
		{
			for (const FString& HeaderName : ModuleElem.Value->SharedHeaderDependencies)
			{
				auto Header = SharedHeaders[HeaderName];
				Header->DependentModules.Add(ModuleElem.Key);
			}
		}

		// Write the shared header files to disk
		for (auto& HeaderElem : SharedHeaders)
		{
			FSharedHeader& Header = *HeaderElem.Value.Get();
			if (Header.Content.IsEmpty())
				continue;
			if (Header.DependentModules.Num() == 0)
				continue;

			// If there's only one dependency, we can add the header straight into the module's code file
			if (Header.DependentModules.Num() == 1 && !Header.bHasDependentSharedHeaders)
			{
				for (asIScriptModule* Dependent : Header.DependentModules)
				{
					TSharedPtr<FJITFile> DependentFile = JITFiles[Dependent];
					for (auto& Include : Header.Includes)
						DependentFile->Headers.Add(Include);
					for (auto& SharedHeaderDependency : Header.SharedHeaderDependencies)
						DependentFile->SharedHeaderDependencies.Add(SharedHeaderDependency);

					DependentFile->Content.Insert(Header.Content, 0);
					DependentFile->SharedHeaderDependencies.Remove(HeaderElem.Key);
				}

				continue;
			}

			FString FullFilename = GenDir / Header.Filename;

			FString FullContent = TEXT("#pragma once\n\n");
			for (const FString& Content : Header.Includes)
				FullContent.Append(Content + TEXT("\n"));
			for (const FString& Content : Header.SharedHeaderDependencies)
				FullContent.Append(FString::Printf(TEXT("#include \"%s\"\n"), *Content));
			FullContent += TEXT("\n");
			for (const FString& Content : Header.Content)
				FullContent.Append(Content);
			FullContent.Append(TEXT("\n\n"));
		}

		// Write each file we have generate code for
		int32 FileCount = JITFiles.Num();
		int32 FileIndex = 0;
		for (auto ModuleElem : JITFiles)
		{
			FJITFile& File = *ModuleElem.Value;
			FString FullFilename = GenDir / File.Filename;

			FString FullContent = STANDARD_HEADER;
			for (const FString& Content : File.Headers)
				FullContent.Append(Content + "\n");
			FullContent += TEXT("\n");
			FullContent += STANDARD_INCLUDES;
			FullContent += TEXT("\n");
			for (const FString& Content : File.SharedHeaderDependencies)
				FullContent.Append(FString::Printf(TEXT("#include \"%s\"\n"), *Content));
			FullContent += TEXT("\n");
			for (auto& Elem : File.ExternalDeclarations)
			{
				FullContent.Append(Elem.Value + "\n");
			}
			FullContent += TEXT("\n");
			for (auto* ExternFunc : File.ExternFunctions)
			{
				FGenerateFunction& GenerateData = FunctionsToGenerate.FindChecked(ExternFunc);
				check(GenerateData.FunctionDeclaration.Len() != 0);
				FullContent.Append(FString::Printf(TEXT("extern %s;\n"), *GenerateData.FunctionDeclaration));
			}
			FullContent.Append(TEXT("\n\n"));

			FString FilenameSymbol = GetUniqueSymbolName(TEXT("MODULENAME_") + File.ModuleName);
			FullContent.Append(FString::Printf(
				TEXT("#if AS_JIT_DEBUG_CALLSTACKS\n")
				TEXT("#undef SCRIPT_DEBUG_FILENAME\n")
				TEXT("static const char* %s = \"%s\";\n")
				TEXT("#define SCRIPT_DEBUG_FILENAME %s\n")
				TEXT("#endif\n"),
				*FilenameSymbol,
				*File.ModuleName,
				*FilenameSymbol
			));

			FullContent.Append(TEXT("\n\n"));
			for (const FString& Content : File.Content)
				FullContent.Append(Content);
			FullContent += STANDARD_FOOTER;

			return FullContent;
		}

		return Code;
	}
	
};

#endif

// Sets default values
AStaticJITViewer::AStaticJITViewer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AStaticJITViewer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AStaticJITViewer::Tick(float DeltaTime)
{

#if STATIC_JIT_DEBUG
	auto& Manager = FAngelscriptManager::Get();
	auto asEngine = Manager.Engine;
	
	static FString SelectedModule;
	static FString SelectedFunction;
	static TArray<FString> ModuleNames;
	static TMap<FString, TArray<FString>> ModuleFunctions;
	static char FuncFilter[128] = "";
	static FString JITContent;

	ImGui::Begin("AngelScript JIT Viewer");
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
						// 生成JIT代码
						if (asIScriptModule* mod = asEngine->GetModuleByIndex(ModuleNames.IndexOfByKey(SelectedModule)))
						{
							if (asIScriptFunction* func = mod->GetFunctionByIndex(i))
							{
								FAngelscriptStaticJIT_Debug* JIT = new FAngelscriptStaticJIT_Debug();
								FAngelscriptPrecompiledData* PrecompiledData = new FAngelscriptPrecompiledData(asEngine);
								JIT->PrecompiledData = PrecompiledData;
								JIT->FunctionsToGenerate.Add((asCScriptFunction*)func, FAngelscriptStaticJIT::FGenerateFunction());
								JITContent = JIT->GetCPPJITCode(func);
								delete PrecompiledData;
								delete JIT;
							}
						}
					}
				}
			}
		}
		ImGui::EndChild();
	}

	ImGui::NextColumn();

	// 右边栏：JIT代码显示
	{
		ImGui::BeginChild("JITView", ImVec2(0, 0), true);
		if (!JITContent.IsEmpty())
		{
			ImGui::TextUnformatted(TCHAR_TO_ANSI(*JITContent));
		}
		else
		{
			ImGui::TextColored(ImVec4(1,1,0,1), "Select a function to view JIT code");
		}
		ImGui::EndChild();
	}

	ImGui::Columns(1);
	ImGui::End();
#endif
}

