#include "AngelscriptBinds.h"

#include "AngelscriptDocs.h"
#include "AngelscriptManager.h"
#include "ClassGenerator/ASClass.h"
#include "GameFramework/GameplayMessageSubsystem.h"

namespace {
  // Build a debug string for a
  FString GetSignatureStringForFunction(UFunction* Function)
  {
    FString Name;
    if (const auto* DelegateFunc = Cast<UDelegateFunction>(Function))
    {
      Name = TEXT("F");
      Name += DelegateFunc->GetName();
      Name.RemoveFromEnd(TEXT("__DelegateSignature"));

      // Delegates declared inside classes get suffixed with the class they're in,
      // so we don't run into conflicts binding them globally.
      if (DelegateFunc->GetOuter()->IsA<UClass>())
        Name += TEXT("__") + DelegateFunc->GetOuter()->GetName();
    }
    else
      Name = Function->GetName();

    FString ReturnType = TEXT("void");
    FString Arguments;

    for (TFieldIterator<FProperty> It(Function); It && (It->PropertyFlags & CPF_Parm); ++It)
    {
      FProperty* Property = *It;
      FAngelscriptTypeUsage Type = FAngelscriptTypeUsage::FromProperty(Property);

      if( Property->PropertyFlags & CPF_ReturnParm )
      {
        ReturnType = Type.GetAngelscriptDeclaration();
      }
      else
      {
        if (Arguments.Len() != 0)
          Arguments += TEXT(", ");

        Arguments += Type.GetAngelscriptDeclaration();

        // Hint that we should make this &in to match the signature
        if ((Property->PropertyFlags & CPF_ReferenceParm) != 0 && (Property->PropertyFlags & CPF_ConstParm) != 0)
          Arguments += TEXT("in");

        Arguments += TEXT(" ");
        Arguments += Property->GetName();
      }
    }

    return FString::Printf(TEXT("%s %s(%s)"),
      *ReturnType, *Name, *Arguments);
  }
}

  void BroadcastMessageAngelscript(
    UGameplayMessageSubsystem* Self,
    FGameplayTag Channel,
    const void* Data,
    const int TypeId
  ) {
    const FAngelscriptTypeUsage Usage = FAngelscriptTypeUsage::FromTypeId(TypeId);

    const UStruct* StructDef = Usage.GetUnrealStruct();
    if (StructDef == nullptr) {
      UE_LOG(LogGameplayMessageSubsystem, Warning, TEXT("BroadcastMessage - not a valid USTRUCT"));
      return;
    }

    const UScriptStruct* ScriptStructDef = dynamic_cast<const UScriptStruct*>(StructDef);
    if (ScriptStructDef == nullptr) {
      UE_LOG(LogGameplayMessageSubsystem, Warning, TEXT("BroadcastMessage - not a valid UScriptStruct"));
      return;
    }

    Self->BroadcastMessageInternal(Channel, ScriptStructDef, Data);
  }

  FGameplayMessageListenerHandle RegisterListenerAngelscript(
    UGameplayMessageSubsystem* Self,
    FGameplayTag Channel,
    UObject* InObject,
    const FName& InFunctionName,
    void* Data,
    int TypeId,
    EGameplayMessageMatch MatchType,
    bool bUnregisterOnWorldDestroyed,
    AActor* UnregisterOnActorDestroyed
  ) {
    UFunction* CallFunction = InObject->FindFunction(InFunctionName);
    if (CallFunction == nullptr) {
      const FString Debug = FString::Printf(TEXT("\nCould not find function %s\nIs it declared UFUNCTION()?"), *InFunctionName.ToString());
      FAngelscriptManager::Throw(TCHAR_TO_ANSI(*Debug));
      return {};
    }

    UASFunction* ASFunction = dynamic_cast<UASFunction*>(CallFunction);
    if (ASFunction == nullptr) {
      UE_LOG(LogGameplayMessageSubsystem, Warning, TEXT("RegisterListener - not a AS function? This shouldn't happen"));
      return {};
    }

    const FAngelscriptTypeUsage Usage = FAngelscriptTypeUsage::FromTypeId(TypeId);

    const UStruct* StructDef = Usage.GetUnrealStruct();
    if (StructDef == nullptr) {
      UE_LOG(LogGameplayMessageSubsystem, Warning, TEXT("RegisterListener - not a valid USTRUCT"));
      return {};
    }

    const UScriptStruct* ScriptStructDef = dynamic_cast<const UScriptStruct*>(StructDef);
    if (ScriptStructDef == nullptr) {
      UE_LOG(LogGameplayMessageSubsystem, Warning, TEXT("RegisterListener - not a valid UScriptStruct"));
      return {};
    }
    const FString PassedInStructType = ScriptStructDef->GetStructCPPName();

    if (
      ASFunction->Arguments.Num() != 2 ||
      ASFunction->Arguments[0].Property == nullptr ||
      ASFunction->Arguments[1].Property == nullptr
    ) {
      const FString Debug = FString::Printf(
        TEXT(
          "\nSpecified function %s is not compatible with callback function.\n"
          "\nCallback: void(FGameplayTag,%s)\n"
          "\nAttempted Bind: %s"
        ),
        *InFunctionName.ToString(),
        *PassedInStructType,
        *GetSignatureStringForFunction(ASFunction)
      );
      FAngelscriptManager::Throw(TCHAR_TO_ANSI(*Debug));
      return {};
    }

    const FProperty* TagProperty = ASFunction->Arguments[0].Property;
    const FProperty* DataProperty = ASFunction->Arguments[1].Property;
    const FString TagType = TagProperty->GetCPPType();
    const FString DataType = DataProperty->GetCPPType();

    if (
      TagType != "FGameplayTag" ||
      DataType != PassedInStructType
    ) {
      const FString Debug = FString::Printf(
        TEXT(
          "\nSpecified function %s is not compatible with callback function.\n"
          "\nCallback: void(FGameplayTag,%s)\n"
          "\nAttempted Bind: %s"
        ),
        *InFunctionName.ToString(),
        *PassedInStructType,
        *GetSignatureStringForFunction(ASFunction)
      );
      FAngelscriptManager::Throw(TCHAR_TO_ANSI(*Debug));
      return {};
    }

    auto ThunkCallback = [ASFunction, InObject, TypeId](
      FGameplayTag ActualTag,
      const UScriptStruct* SenderStructType,
      const void* SenderPayload
    ) {
      constexpr int32 TagSize = sizeof(ActualTag);
      const UScriptStruct::ICppStructOps* StructOps = SenderStructType->GetCppStructOps();
      const int32 StructSize = StructOps->GetSize();

      auto& tagArg = ASFunction->Arguments[0];
      const auto& dataArg = ASFunction->Arguments[1];

      void* Parms = FMemory_Alloca_Aligned(ASFunction->ArgStackSize, 16);
      FMemory::Memzero((char *) Parms, ASFunction->ArgStackSize);
      FMemory::Memcpy((char *) Parms, &ActualTag, TagSize);
      FMemory::Memcpy((char *) Parms + dataArg.PosInParmStruct, SenderPayload, StructSize);

      InObject->ProcessEvent(ASFunction, Parms);
    };

    return Self->RegisterListenerInternal(
      Channel,
      ThunkCallback,
      ScriptStructDef,
      MatchType,
      bUnregisterOnWorldDestroyed,
      UnregisterOnActorDestroyed
    );
  }

  void UnregisterListenerAngelscript(
    UGameplayMessageSubsystem* Self,
    FGameplayMessageListenerHandle Handle
  ) {
    Self->UnregisterListener(Handle);
  }

FAngelscriptBinds::FBind Bind_GameplayMessageSubsystem(FAngelscriptBinds::EOrder::Late, [] {
  const FString BroadcastMessageSignature =
    "void BroadcastMessage("
    "FGameplayTag Channel, "
    "const ?&in MaybeStruct)";
  const FString RegisterListenerSignature =
    "FGameplayMessageListenerHandle RegisterListener("
    "FGameplayTag Channel, "
    "UObject Object, "
    "const FName& FunctionName, "
    "const ?&in EmptyStruct, "
    "EGameplayMessageMatch MatchType = EGameplayMessageMatch::ExactMatch, "
    "bool bUnregisterOnWorldDestroyed = true, "
    "AActor UnregisterOnActorDestroyed = nullptr)";
  const FString UnregisterListenerSignature =
    "void UnregisterListener("
    "FGameplayMessageListenerHandle Handle)";

  auto UGameplayMessageSubsystem_ = FAngelscriptBinds::ExistingClass("UGameplayMessageSubsystem");

  {
    FAngelscriptBinds::FNamespace ns("UGameplayMessageSubsystem");

    FAngelscriptBinds::BindGlobalFunction("bool HasInstance(const UObject WorldContextObject)", &UGameplayMessageSubsystem::HasInstance);
    SCRIPT_BIND_DOCUMENTATION("@return true if a valid GameplayMessageRouter subsystem if active in the provided world")
  }

  UGameplayMessageSubsystem_.Method(BroadcastMessageSignature, BroadcastMessageAngelscript);
  SCRIPT_BIND_DOCUMENTATION(
    "Broadcast a message on the specified channel\n\n"
    "@param Channel The message channel to broadcast on\n"
    "@param Message The message to send (must be the same type of UScriptStruct expected by the listeners for this channel, otherwise an error will be logged)\n"
  )

  UGameplayMessageSubsystem_.Method(RegisterListenerSignature, RegisterListenerAngelscript);
  SCRIPT_BIND_DOCUMENTATION(
    "Register to receive messages on a specified channel\n\n"
    "@param Channel The message channel to listen to\n"
    "@param Callback Function to call with the message when someone broadcasts it (must be the same type of UScriptStruct provided by broadcasters for this channel, otherwise an error will be logged)\n\n"
    "@param bUnregisterOnWorldDestroyed Whether or not this listener should be automatically unregistered when the world is destroyed (set to false for GameInstance listeners)\n"
    "@param UnregisterOnActorDestroyed An optional actor reference that will automatically unregister this listener when it is destroyed\n"
    "@return a handle that can be used to unregister this listener (either by calling Unregister() on the handle or calling UnregisterListener on the router)"
  )

  UGameplayMessageSubsystem_.Method(UnregisterListenerSignature, UnregisterListenerAngelscript);
  SCRIPT_BIND_DOCUMENTATION(
    "Register to receive messages on a specified channel and handle it with a specified member function\n"
    "Executes a weak object validity check to ensure the object registering the function still exists before triggering the callback\n\n"
    "@param Channel The message channel to listen to\n"
    "@param Object The object instance to call the function on\n"
    "@param Function Member function to call with the message when someone broadcasts it (must be the same type of UScriptStruct provided by broadcasters for this channel, otherwise an error will be logged)\n\n"
    "@return a handle that can be used to unregister this listener (either by calling Unregister() on the handle or calling UnregisterListener on the router)"
  )
});

FAngelscriptBinds::FBind Bind_FGameplayMessageListenerHandle(FAngelscriptBinds::EOrder::Late, [] {
  auto FGameplayMessageListenerHandle_ = FAngelscriptBinds::ExistingClass("FGameplayMessageListenerHandle");

  FGameplayMessageListenerHandle_.Method("void Unregister()", METHOD_TRIVIAL(FGameplayMessageListenerHandle, Unregister));
});
