// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class GameplayMessageRuntime : ModuleRules
{
	public GameplayMessageRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		string EnginePath = Path.GetFullPath(Target.RelativeEnginePath);

		PrivateIncludePaths.AddRange(
			new string[]
			{
				EnginePath + "Plugins/Angelscript/Source/AngelscriptCode/Private",
			});

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Engine",
				"GameplayTags"
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"AngelscriptCode",
			});

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			});
	}
}
