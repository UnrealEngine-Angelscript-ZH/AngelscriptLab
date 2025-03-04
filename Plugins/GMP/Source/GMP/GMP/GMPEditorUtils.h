//  Copyright GenericMessagePlugin, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"

#if WITH_EDITOR
#include "TimerManager.h"
namespace GMP
{
namespace FEditorUtils
{
	bool DelayExecImpl(const UObject* InObj, FTimerDelegate Delegate, float InDelay, bool bEnsureExec);
	template<typename F>
	auto DelayExec(const UObject* InObj, F&& Lambda, float InDelay = 0.f, bool bEnsureExec = true)
	{
		if (InObj)
			return DelayExecImpl(InObj, FTimerDelegate::CreateWeakLambda(const_cast<UObject*>(InObj), Forward<F>(Lambda)), InDelay, bEnsureExec);
		else
			return DelayExecImpl(InObj, FTimerDelegate::CreateLambda(Forward<F>(Lambda)), InDelay, bEnsureExec);
	}

	void GetReferenceAssets(const UObject* InObj, const TArray<FString>& PathIdArray, TMap<FName, TArray<FName>>& OutRef, TMap<FName, TArray<FName>>& OutDep, bool bRecur);
	void UnloadPackages(const UObject* InObj, TArray<FString> PathIdArray, TDelegate<void(bool, TArray<FString>)> OnResult);
	void DeletePackages(const UObject* InObj, TArray<FString> PathIdArray, TDelegate<void(bool, TArray<FString>)> OnResult);
}  // namespace FEditorUtils
}  // namespace GMP
#endif
