#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/CoreDelegates.h"
#include "HAL/FileManager.h"

#include <as_scriptengine.h>

#include "AngelscriptManager.h"

void PrintBytecode(asIScriptFunction* func);

static void DumpAngelscriptBytecode(const TArray<FString>& Args, UWorld* InWorld)
{
	auto& Manager = FAngelscriptManager::Get();
	auto asEngine = Manager.Engine;

	// 解析过滤参数
	FString FilterModule, FilterFunction;
	if (Args.Num() >= 1) FilterModule = Args[0].ToLower();
	if (Args.Num() >= 2) FilterFunction = Args[1].ToLower();
	
	if (Args.Num() > 2)
	{
		UE_LOG(LogTemp, Error, TEXT("Usage: Dump.Angelscript.Bytecode [ModuleName] [FunctionName]"));
		return;
	}
	
	// 遍历所有模块
	auto modnum = asEngine->GetModuleCount();
	for(asUINT i = 0; i < modnum; ++i)
	{
		auto mod = asEngine->GetModuleByIndex(i);
		FString ModuleName = UTF8_TO_TCHAR(mod->GetName());
		
		// 过滤模块名称
		if (!FilterModule.IsEmpty() && ModuleName.ToLower() != FilterModule)
			continue;

		UE_LOG(LogTemp, Warning, TEXT("Module %d: %s"), i, *ModuleName);

		// 遍历模块函数
		auto modfnum = mod->GetFunctionCount();
		for(asUINT j = 0; j < modfnum; ++j)
		{
			auto func = mod->GetFunctionByIndex(j);
			FString FuncName = UTF8_TO_TCHAR(func->GetName());
			
			// 过滤函数名称
			if (!FilterFunction.IsEmpty() && FuncName.ToLower() != FilterFunction)
				continue;

			UE_LOG(LogTemp, Warning, TEXT("Function %d: %s"), j, *FuncName);
			PrintBytecode(func);
		}
	}
}

FString GetInstrDebugString(asDWORD* BC, asIScriptFunction* func)
{
	auto engine = static_cast<asCScriptEngine*>(func->GetEngine());

	auto byteCode = BC;
	asEBCInstr Instr = (asEBCInstr)*(asBYTE*)BC;

	// Some special handling
	switch (Instr)
	{
	case asBC_ADDSi:
		return FString::Printf(TEXT("%-8ls %d"),ANSI_TO_TCHAR(asBCInfo[Instr].name), asBC_SWORDARG0(BC));
	case asBC_LoadThisR:
		return FString::Printf(TEXT("%-8ls %d"),ANSI_TO_TCHAR(asBCInfo[Instr].name), asBC_SWORDARG0(BC));
	case asBC_PshGPtr:
		{
			void* GlobalPtr = (void*)asBC_PTRARG(BC);
			auto* Engine = (asCScriptEngine*)FAngelscriptManager::Get().Engine;

			if (asCGlobalProperty** PropPtr = Engine->varAddressMap.Find(GlobalPtr))
			{
				return FString::Printf(TEXT("%-8ls %s::%s"),
				                       ANSI_TO_TCHAR(asBCInfo[Instr].name),
				                       ANSI_TO_TCHAR((*PropPtr)->nameSpace->name.AddressOf()),
				                       ANSI_TO_TCHAR((*PropPtr)->name.AddressOf()));
			}
		}
	}

	FString Out = FString::Printf(TEXT("%-8ls "), ANSI_TO_TCHAR(asBCInfo[Instr].name));

	switch (asBCInfo[Instr].type)
	{
	case asBCTYPE_W_ARG:
		Out += FString::Printf(TEXT("%d"), asBC_SWORDARG0(BC));
		break;

	case asBCTYPE_wW_ARG:
	case asBCTYPE_rW_ARG:
		Out += FString::Printf(TEXT("v%d"), asBC_SWORDARG0(BC));
		break;

	case asBCTYPE_wW_rW_ARG:
	case asBCTYPE_rW_rW_ARG:
		Out += FString::Printf(TEXT("v%d, v%d"), asBC_SWORDARG0(BC), asBC_SWORDARG1(BC));
		break;

	case asBCTYPE_wW_W_ARG:
		Out += FString::Printf(TEXT("v%d, %d"), asBC_SWORDARG0(BC), asBC_SWORDARG1(BC));
		break;

	case asBCTYPE_wW_rW_DW_ARG:
	case asBCTYPE_rW_W_DW_ARG:
		switch (Instr)
		{
		case asBC_ADDIf:
		case asBC_SUBIf:
		case asBC_MULIf:
			// Out += FString::Printf(TEXT("v%d, v%d, %d"), asBC_SWORDARG0(BC), asBC_SWORDARG1(BC), asBC_FLOATARG(BC+1));

			Out += FString::Printf(TEXT("v%d, v%d, %f"),
			                       asBC_SWORDARG0(byteCode), asBC_SWORDARG1(byteCode),
			                       *((float*)asBC_DWORDARG(Instr + 1)));
			break;
		default:
			Out += FString::Printf(TEXT("v%d, v%d, %d"), asBC_SWORDARG0(BC), asBC_SWORDARG1(BC), asBC_INTARG(BC+1));

			
			// Out += FString::Printf(TEXT("v%d, v%d, %d"),
			//                        asBC_SWORDARG0(byteCode),
			//                        asBC_SWORDARG1(byteCode), *((int*)asBC_DWORDARG(Instr + 1)));
			break;
		}
		break;

	case asBCTYPE_DW_ARG:
		{
			asEBCInstr op = asEBCInstr(*(asBYTE*)byteCode);

			switch (op)
			{
			case asBC_OBJTYPE:
				{
					auto* objectType = *reinterpret_cast<asCObjectType**>(asBC_DWORDARG(BC));
					Out += FString::Printf(TEXT("0x%x (type:%hs)"), 
						static_cast<asUINT>(asBC_DWORDARG(BC)), objectType->GetName());
				}
				break;

			case asBC_FuncPtr:
				{
					asCScriptFunction* f = *(asCScriptFunction**)asBC_DWORDARG(byteCode);
					// UE_LOG(LogTemp, Log, TEXT("%-8hs 0x%x    (func:%hs)"), asBCInfo[op].name,
					//        (asUINT) asBC_DWORDARG(byteCode), f->GetDeclaration());

					auto* function = *reinterpret_cast<asCScriptFunction**>(asBC_DWORDARG(BC));
					Out += FString::Printf(TEXT("0x%x (func:%hs)"), 
						static_cast<asUINT>(asBC_DWORDARG(BC)), function->GetDeclaration());
				}
				break;

			case asBC_PshC4:
			case asBC_Cast:
				{
				// Out += FString::Printf(
				// 	TEXT("0x%x    (i:%d, f:%g)"), (asUINT)asBC_DWORDARG(byteCode),
				// 	*((int*)asBC_DWORDARG(byteCode)), *(float*)asBC_DWORDARG(byteCode));
				}

				break;

			case asBC_TYPEID:
				Out += FString::Printf(TEXT("0x%x '%hs'"),
				                       static_cast<asUINT>(asBC_DWORDARG(BC)),
				                       engine->GetTypeDeclaration(static_cast<int>(asBC_DWORDARG(BC))));
				break;

			case asBC_CALL:
			case asBC_CALLSYS:
			case asBC_CALLBND:
			case asBC_CALLINTF:
			case asBC_Thiscall1:
				{
					int funcID = *(int*)asBC_DWORDARG(byteCode);
					asCString decl = engine->GetFunctionDeclaration(funcID);

					Out += FString::Printf(TEXT("%d (%hs)"),
					                       *((int*)asBC_DWORDARG(byteCode)), decl.AddressOf());
				}
				break;

			case asBC_JMP:
			case asBC_JZ:
			case asBC_JLowZ:
			case asBC_JS:
			case asBC_JP:
			case asBC_JNZ:
			case asBC_JLowNZ:
			case asBC_JNS:
			case asBC_JNP:
				// UE_LOG(LogTemp, Log, TEXT("%-8hs %+d       (d:%d)"), asBCInfo[op].name,
				//        *((int*) asBC_DWORDARG(byteCode)), pos + *((int*) asBC_DWORDARG(byteCode)));
				// break;

			default:
				// UE_LOG(LogTemp, Log, TEXT("%-8hs %d"), asBCInfo[op].name, *((int*) asBC_DWORDARG(byteCode)));
				break;
			}
		}
		break;

	case asBCTYPE_QW_ARG:
		{

			asEBCInstr op = asEBCInstr(*(asBYTE*)byteCode);

			switch (op)
			{
			case asBC_OBJTYPE:
				{
					asCObjectType* ot = *(asCObjectType**)asBC_DWORDARG(byteCode);
					Out += FString::Printf(TEXT("0x%x    (type:%hs)"),
					                       static_cast<asUINT>(asBC_DWORDARG(byteCode)),
					                       ot->GetName());
				}
				break;
			
			case asBC_FuncPtr:
				{
					asCScriptFunction* f = *(asCScriptFunction**)asBC_DWORDARG(byteCode);
					// UE_LOG(LogTemp, Log, TEXT("%-8hs 0x%x    (func:%hs)"), asBCInfo[op].name,
					//        (asUINT) asBC_DWORDARG(byteCode), f->GetDeclaration());
					Out += FString::Printf(TEXT("0x%x    (asfunc:%hs)"),
					                       static_cast<asUINT>(asBC_DWORDARG(byteCode)),
					                       f->GetDeclaration());
				}
				break;
			
			case asBC_PshC4:
			case asBC_Cast:
				Out += FString::Printf(TEXT("%d"), asBC_INTARG(BC));
			
				// UE_LOG(LogTemp, Warning, TEXT("%-8hs 0x%x    (i:%d, f:%g)"), asBCInfo[op].name,
				//        (asUINT) asBC_DWORDARG(byteCode), *((int*) asBC_DWORDARG(byteCode)), *((float*) asBC_DWORDARG(byteCode)));
				break;
			
			case asBC_CALL:
			case asBC_CALLSYS:
			case asBC_CALLBND:
			case asBC_CALLINTF:
			case asBC_Thiscall1:
				{
					int funcID = *(int*)ARG_DW(asBC_QWORDARG(byteCode));
					asCScriptFunction* Function = (asCScriptFunction*)asBC_PTRARG(byteCode);
					asCString decl = Function->GetDeclarationStr();
					Out += FString::Printf(TEXT("%hs"),
					                       decl.AddressOf());

				}
				break;

			default:
				// Out += FString::Printf(TEXT("%d"), *reinterpret_cast<const int*>(asBC_DWORDARG(BC)));
				break;
			}
		}
		break;

	case asBCTYPE_wW_QW_ARG:
	case asBCTYPE_rW_QW_ARG:
		Out += FString::Printf(TEXT("v%d, *"), asBC_SWORDARG0(BC));
		break;

	case asBCTYPE_DW_DW_ARG:
		Out += FString::Printf(TEXT("%d, %d"), asBC_DWORDARG(BC), asBC_DWORDARG(BC+1));
		break;

	case asBCTYPE_rW_DW_DW_ARG:
		Out += FString::Printf(TEXT("v%d, %d, %d"), asBC_SWORDARG0(BC), asBC_DWORDARG(BC), asBC_DWORDARG(BC+1));
		break;

	case asBCTYPE_QW_DW_ARG:
		Out += FString::Printf(TEXT("*, %d"), asBC_DWORDARG(BC+AS_PTR_SIZE));
		break;

	case asBCTYPE_rW_DW_ARG:
	case asBCTYPE_wW_DW_ARG:
	case asBCTYPE_W_DW_ARG:
		Out += FString::Printf(TEXT("v%d, %d"), asBC_SWORDARG0(BC), asBC_DWORDARG(BC));
		break;

	case asBCTYPE_wW_rW_rW_ARG:
		Out += FString::Printf(TEXT("v%d, v%d, v%d"), asBC_SWORDARG0(BC), asBC_SWORDARG1(BC), asBC_SWORDARG2(BC));
		break;
	}

	return Out;
}


FString GetBytecode(asIScriptFunction* func)
{
	FString Result;
	auto engine = static_cast<asCScriptEngine*>(func->GetEngine());
	// Get the script byte code
	asUINT length;
	asDWORD* byteCode = func->GetByteCode(&length);
	asDWORD* end = byteCode + length;

	while (byteCode < end)
	{
		// Determine the instruction
		asEBCInstr op = asEBCInstr(*(asBYTE*)byteCode);
		auto debug = GetInstrDebugString(byteCode, func);
		Result += debug + TEXT("\n");
		// UE_LOG(LogTemp, Log, TEXT("%s"), *debug);

		// Move to next instruction
		byteCode += asBCTypeSize[asBCInfo[op].type];
	}

	return Result;
}


void PrintBytecode(asIScriptFunction* func)
{
	auto engine = static_cast<asCScriptEngine*>(func->GetEngine());
	// Get the script byte code
	asUINT length;
	asDWORD* byteCode = func->GetByteCode(&length);
	asDWORD* end = byteCode + length;

	while (byteCode < end)
	{
		// Determine the instruction
		asEBCInstr op = asEBCInstr(*(asBYTE*)byteCode);
		auto debug = GetInstrDebugString(byteCode, func);
		UE_LOG(LogTemp, Log, TEXT("%s"), *debug);

		// Move to next instruction
		byteCode += asBCTypeSize[asBCInfo[op].type];
	}
}

FAutoConsoleCommandWithWorldAndArgs DumpBytecodeCommand(
	TEXT("Dump.Angelscript.Bytecode"),
	TEXT("Dump Angelscript bytecode for debugging\n"
		 "Params: [ModuleName] [FunctionName] (both optional filters)\n\n"
		 "Examples:\n"
		 "  Dump.Angelscript.Bytecode             - Dump all modules/functions\n"
		 "  Dump.Angelscript.Bytecode MyModule    - Dump specific module\n"
		 "  Dump.Angelscript.Bytecode MyModule MyFunction - Dump specific function\n\n"
		 "Output: LogTemp channel with Warning/Log level\n"
		 "Note: Module/Function names are case-insensitive"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(DumpAngelscriptBytecode)
);

