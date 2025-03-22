
#include "Misc/CoreDelegates.h"
#include "HAL/FileManager.h"
#include "AngelscriptManager.h"

#include <as_scriptengine.h>
#include <as_scriptfunction.h>
#include <as_typeinfo.h>
#include <as_context.h>
#include <as_generic.h>

#include "AngelscriptBinds.h"


void EnumToString(asIScriptGeneric* Generic)
{
	auto Engine = static_cast<asCScriptEngine*>(Generic->GetEngine());

	auto CGeneric = static_cast<asCGeneric*>(Generic);
	asDWORD *l_sp = CGeneric->stackPointer;

	asDWORD TypeId = *(l_sp + AS_PTR_SIZE);
	int TypeIdInt = static_cast<int>(TypeId);
	auto* TypeInfo = (asCTypeInfo*)Engine->GetTypeInfoById(TypeIdInt);
	if (TypeInfo && (TypeInfo->flags & asOBJ_ENUM))
	{
		auto TypeUsage = FAngelscriptTypeUsage::FromTypeId(TypeIdInt);
		auto Arg0 = Generic->GetArgAddress(0);
		FString& OutputString = *static_cast<FString*>(Generic->GetArgAddress(1));

		TypeUsage.GetStringIdentifier(Arg0, OutputString);
	}
}

AS_FORCE_LINK const FAngelscriptBinds::FBind Bind_EnumToString(FAngelscriptBinds::EOrder::Late, []
{
	FAngelscriptBinds::BindGlobalGenericFunction("void EnumToString(const ?& Arg0, FString& OutString) no_discard", &EnumToString);
});

