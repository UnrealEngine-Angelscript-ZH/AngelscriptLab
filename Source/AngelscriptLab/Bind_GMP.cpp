
#include "Misc/CoreDelegates.h"
#include "HAL/FileManager.h"
#include "AngelscriptManager.h"

#include <as_scriptengine.h>
#include <as_scriptfunction.h>
#include <as_typeinfo.h>
#include <as_context.h>

#include "GMPHelper.h"

#include "AngelscriptBinds.h"
#include "ClassGenerator/ASClass.h"
#include "GMP/GMPUtils.h"

TSharedRef<FStructOnScope> GetFunctionParams(UFunction* Function, GMP::FMessageBody& MessageBody)
{
	TSharedRef<FStructOnScope> FuncParams = MakeShared<FStructOnScope>(Function);

	uint32 Offset = 0;
	uint32 Index = 0;

	for (TFieldIterator<FProperty> PropIt(Function); PropIt; ++PropIt)
	{
		auto Name = PropIt->GetName();
		auto CPPType = PropIt->GetCPPType();
		auto Size = PropIt->GetSize();

		auto& Addresses = MessageBody.GetParams();
		auto& Address = Addresses[Index];
		FMemory::Memcpy(FuncParams->GetStructMemory() + Offset, Address.ToAddr(), Size);
		Offset += Size;
		Index++;

		// TODO: Only Warning
	}

	return FuncParams;
}

void Listener(FName Event, UObject* Source, UObject* Listener, FName FunctionName)
{

		
	if (!IsValid(Listener))
		return;

	auto EventHub = GMP::FMessageUtils::GetMessageHub();


	if (UFunction* Function = Listener->FindFunction(FunctionName))
	{
		EventHub->ScriptListenMessage(Source, Event, Listener, [&, Listener, Function](GMP::FMessageBody& MessageBody)
		{
			auto FuncParams = GetFunctionParams(Function, MessageBody);

			if (Function && Function->FunctionFlags & FUNC_RuntimeGenerated)
			{
				// Angelscript function
				Function->RuntimeCallEvent(Listener, FuncParams->GetStructMemory());
			}
			else
			{
				// C++ function
				Listener->ProcessEvent(Function, FuncParams->GetStructMemory());
			}
		});
	}
	else // as row function
	{
		auto ASClass = UASClass::GetFirstASOrNativeClass(Listener->GetClass());

		if(!ASClass)
			return;

		FAngelscriptClassDesc* ASClassDesc = nullptr;
		// TSharedRef<FAngelscriptClassDesc> ASClassDesc;

		for(auto& Module : FAngelscriptManager::Get().GetActiveModules())
		{
			for (auto Class : Module->Classes)
			{
				if(Class->Class == ASClass)
				{
					// UE_DEBUG_BREAK();
					ASClassDesc = &Class.Get();
				}
			}
		}

		if(ASClassDesc && ASClassDesc->ScriptType)
		{
			auto ScriptType = ASClassDesc->ScriptType;

			auto FunctionNameString = FunctionName.ToString();
			if(auto ASFunction = ScriptType->GetMethodByName(TCHAR_TO_UTF8(*FunctionNameString)))
			{
				EventHub->ScriptListenMessage(
					Source, Event, Listener,
					[&, Listener, ASFunction, FunctionName ](GMP::FMessageBody& MessageBody)
					{
						asIScriptFunction* RunFunction = ASFunction;
						const bool bInGameThread = IsInGameThread();
						// Develop
						// if(ASClass->ScriptTypePtr == nullptr)
						// 	return;

						auto ASClass1 = UASClass::GetFirstASOrNativeClass(Listener->GetClass());

						auto& Manager = FAngelscriptManager::Get();
						auto Module = Manager.GetModule(((asITypeInfo*)ASClass1->ScriptTypePtr)->GetModule());

						if (Module)
						{
							auto astype = (asITypeInfo*)ASClass1->ScriptTypePtr;

							FAngelscriptClassDesc* ASClassDesc1 = nullptr;
							// TSharedRef<FAngelscriptClassDesc> ASClassDesc;

							for (auto Class : Module->Classes)
							{
								if (Class->Class == ASClass1)
								{
									// UE_DEBUG_BREAK();
									ASClassDesc1 = &Class.Get();
								}
							}
							if (!(ASClassDesc1 && ASClassDesc1->ScriptType))
								UE_DEBUG_BREAK();

							auto ASFunction1 = ASClassDesc1->ScriptType->GetMethodByName(
								TCHAR_TO_UTF8(*FunctionName.ToString()));

							RunFunction = ASFunction1;
							// check(RunFunction && );
							
						}

						//
						
						FAngelscriptManager::AssignWorldContext(Listener);
						UObject* PrevWorldContext = nullptr;
						PrevWorldContext = (UObject*)FAngelscriptManager::CurrentWorldContext;
						

						FAngelscriptPooledContextBase Context;
						Context->Prepare(RunFunction);
						Context->SetObject(Listener);

						uint32 Index = 0;
						for(auto& Item : MessageBody.GetParams())
						{
							Context->SetArgAddress(Index, Item.ToAddr());
							Index++;
						}
						Context->Execute();
						FAngelscriptManager::AssignWorldContext(nullptr);

					});
			}
		}
	}
}

void Send(asIScriptGeneric* Generic)
{
	const FName& Event= *static_cast<FName*>(Generic->GetArgAddress(0));
	UObject* Source = static_cast<UObject*>(Generic->GetArgAddress(1));

	check(Event != NAME_None && Source);

	asCScriptFunction* Function = static_cast<asCScriptFunction*>(Generic->GetFunction());

	GMP::FTypedAddresses Addresses;


	for (int32 Arg = 2; Arg < Generic->GetArgCount(); ++Arg)
	{
		asCDataType ASParamType = Function->parameterTypes[Arg];

		if(ASParamType.IsReference())
		{
			Addresses.Add(FGMPTypedAddr::FromAddr(Generic->GetArgAddress(Arg)));

			// Dynamic check
			auto TypeId = Generic->GetArgTypeId(Arg);
		
			asITypeInfo* TypeInfo = FAngelscriptManager::Get().Engine->GetTypeInfoById(TypeId);

			auto TypeUsage= FAngelscriptTypeUsage::FromTypeId(TypeId);

			bool bIsObjectPointer = TypeUsage.Type.IsValid() && TypeUsage.Type->IsObjectPointer();

			if(TypeUsage.Type.IsValid() && !bIsObjectPointer)
			{
				auto TypeName = TypeUsage.Type->GetAngelscriptTypeName();
				auto& Addresse = Addresses.Last();
				Addresse.TypeName = FName(*TypeName);
			}
			else if(TypeUsage.ScriptClass)
			{
				auto TypeName = TypeUsage.ScriptClass->GetName();
				auto& Addresse = Addresses.Last();
				Addresse.TypeName = FName(TypeName);
			}
		}
		else
		{
			// TODO:
			check(false);
		}
	}

	auto Hub = GMP::FMessageUtils::GetMessageHub();

	Hub->ScriptNotifyMessage(Event, Addresses, Source);
}

AS_FORCE_LINK const FAngelscriptBinds::FBind Bind_ATestActor(FAngelscriptBinds::EOrder::Late, []
{
	FAngelscriptBinds::FNamespace ns("GMP");

	FAngelscriptBinds::BindGlobalFunction("void ListenObjectMessage(FName Event, UObject Source, UObject Listener, FName Function)", FUNC_TRIVIAL(Listener));

	FAngelscriptBinds::BindGlobalGenericFunction("void SendObjectMessage(const FName& Event, UObject Source, const ?& Arg0) no_discard", &Send);
	FAngelscriptBinds::BindGlobalGenericFunction("void SendObjectMessage(const FName& Event, UObject Source, const ?& Arg0, const ?& Arg1) no_discard", &Send);
	FAngelscriptBinds::BindGlobalGenericFunction("void SendObjectMessage(const FName& Event, UObject Source, const ?& Arg0, const ?& Arg1, const ?& Arg2) no_discard", &Send);
	// TODO: more args

});

