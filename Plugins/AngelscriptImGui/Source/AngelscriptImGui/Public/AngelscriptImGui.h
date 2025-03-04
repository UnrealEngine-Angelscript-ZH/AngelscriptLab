// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FAngelscriptImGuiModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FAngelscriptImGuiModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FAngelscriptImGuiModule>("AngelscriptImGui");
	}
	
	
};
