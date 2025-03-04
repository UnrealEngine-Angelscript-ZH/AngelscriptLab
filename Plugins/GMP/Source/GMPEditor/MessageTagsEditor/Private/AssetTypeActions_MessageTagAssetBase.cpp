// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetTypeActions_MessageTagAssetBase.h"
#if UE_4_24_OR_LATER
#include "ToolMenus.h"
#include "ToolMenuSection.h"
#else
#include "Framework/MultiBox/MultiBoxBuilder.h"
#endif
#include "UObject/UnrealType.h"
#include "UObject/WeakObjectPtr.h"
#include "Framework/Application/SlateApplication.h"

#include "MessageTagContainer.h"
#include "SMessageTagWidget.h"
#include "Interfaces/IMainFrameModule.h"
#include "SMessageTagPicker.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FAssetTypeActions_MessageTagAssetBase::FAssetTypeActions_MessageTagAssetBase(FName InTagPropertyName)
	: OwnedMessageTagPropertyName(InTagPropertyName)
{}

bool FAssetTypeActions_MessageTagAssetBase::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}

#if UE_4_24_OR_LATER
void FAssetTypeActions_MessageTagAssetBase::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
#else
void FAssetTypeActions_MessageTagAssetBase::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
#endif
{
	TArray<UObject*> Objects;
	TArray<FMessageTagContainer> Containers;
	for (int32 ObjIdx = 0; ObjIdx < InObjects.Num(); ++ObjIdx)
	{
		UObject* CurObj = InObjects[ObjIdx];
		if (CurObj)
		{
			FStructProperty* StructProp = FindFProperty<FStructProperty>(CurObj->GetClass(), OwnedMessageTagPropertyName);
			if(StructProp != NULL)
			{
				const FMessageTagContainer& Container = *StructProp->ContainerPtrToValuePtr<FMessageTagContainer>(CurObj); 
				Objects.Add(CurObj);
				Containers.Add(Container);
			}
		}
	}

	if (Containers.Num() > 0)
	{
#if UE_4_24_OR_LATER
		Section.AddMenuEntry(
			"MessageTags_Edit",
			LOCTEXT("MessageTags_Edit", "Edit Message Tags..."),
			LOCTEXT("MessageTags_EditToolTip", "Opens the Message Tag Editor."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &FAssetTypeActions_MessageTagAssetBase::OpenMessageTagEditor, Objects, Containers), FCanExecuteAction()));
#else
		MenuBuilder.AddMenuEntry(
			LOCTEXT("MessageTags_Edit", "Edit Message Tags..."),
			LOCTEXT("MessageTags_EditToolTip", "Opens the Message Tag Editor."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &FAssetTypeActions_MessageTagAssetBase::OpenMessageTagEditor, Objects, Containers), FCanExecuteAction()));
#endif
	}
}

void FAssetTypeActions_MessageTagAssetBase::OpenMessageTagEditor(TArray<UObject*> Objects, TArray<FMessageTagContainer> Containers) const
{
#if 1
	if (!Objects.Num() || !Containers.Num())
	{
		return;
	}

	check(Objects.Num() == Containers.Num());

	for (UObject* Object : Objects)
	{
		check (IsValid(Object));
		Object->SetFlags(RF_Transactional);
	}

	FText Title;
	FText AssetName;

	const int32 NumAssets = Containers.Num();
	if (NumAssets > 1)
	{
		AssetName = FText::Format( LOCTEXT("AssetTypeActions_MessageTagAssetBaseMultipleAssets", "{0} Assets"), FText::AsNumber( NumAssets ) );
		Title = FText::Format( LOCTEXT("AssetTypeActions_MessageTagAssetBaseEditorTitle", "Tag Editor: Owned Message Tags: {0}"), AssetName );
	}
	else
	{
		AssetName = FText::FromString(GetNameSafe(Objects[0]));
		Title = FText::Format( LOCTEXT("AssetTypeActions_MessageTagAssetBaseEditorTitle", "Tag Editor: Owned Message Tags: {0}"), AssetName );
	}

	TSharedPtr<SWindow> Window = SNew(SWindow)
		.Title(Title)
		.ClientSize(FVector2D(500, 600))
		[
			SNew(SMessageTagPicker)
			.TagContainers(Containers)
			.MaxHeight(0) // unbounded
			.OnRefreshTagContainers_Lambda([Objects, OwnedMessageTagPropertyName = OwnedMessageTagPropertyName](SMessageTagPicker& TagPicker)
			{
				// Refresh tags from objects, this is called e.g. on post undo/redo. 
				TArray<FMessageTagContainer> Containers;
				for (UObject* Object : Objects)
				{
					// Adding extra entry even if the object has gone invalid to keep the container count the same as object count.
					FMessageTagContainer& NewContainer = Containers.AddDefaulted_GetRef();
					if (IsValid(Object))
					{
						const FStructProperty* Property = FindFProperty<FStructProperty>(Object->GetClass(), OwnedMessageTagPropertyName);
						if (Property != nullptr)
						{
							NewContainer = *Property->ContainerPtrToValuePtr<FMessageTagContainer>(Object); 
						}
					}
				}
				TagPicker.SetTagContainers(Containers);
			})
			.OnTagChanged_Lambda([Objects, OwnedMessageTagPropertyName = OwnedMessageTagPropertyName](const TArray<FMessageTagContainer>& TagContainers)
			{
				// Sanity check that our arrays are in sync.
				if (Objects.Num() != TagContainers.Num())
				{
					return;
				}
				
				for (int32 Index = 0; Index < Objects.Num(); Index++)
				{
					UObject* Object = Objects[Index];
					if (!IsValid(Object))
					{
						continue;
					}

					FStructProperty* Property = FindFProperty<FStructProperty>(Object->GetClass(), OwnedMessageTagPropertyName);
					if (!Property)
					{
						continue;
					}

					Object->Modify();
					FMessageTagContainer& Container = *Property->ContainerPtrToValuePtr<FMessageTagContainer>(Object); 

					FEditPropertyChain PropertyChain;
					PropertyChain.AddHead(Property);
					Object->PreEditChange(PropertyChain);

					Container = TagContainers[Index];
					
					FPropertyChangedEvent PropertyEvent(Property);
					Object->PostEditChangeProperty(PropertyEvent);
				}
			})
		];
#else
	TArray<SMessageTagWidget::FEditableMessageTagContainerDatum> EditableContainers;
	for (int32 ObjIdx = 0; ObjIdx < Objects.Num() && ObjIdx < Containers.Num(); ++ObjIdx)
	{
		EditableContainers.Add(SMessageTagWidget::FEditableMessageTagContainerDatum(Objects[ObjIdx], Containers[ObjIdx]));
	}

	FText Title;
	FText AssetName;

	const int32 NumAssets = EditableContainers.Num();
	if (NumAssets > 1)
	{
		AssetName = FText::Format( LOCTEXT("AssetTypeActions_MessageTagAssetBaseMultipleAssets", "{0} Assets"), FText::AsNumber( NumAssets ) );
		Title = FText::Format( LOCTEXT("AssetTypeActions_MessageTagAssetBaseEditorTitle", "Tag Editor: Owned Message Tags: {0}"), AssetName );
	}
	else if (NumAssets > 0 && EditableContainers[0].TagContainerOwner.IsValid())
	{
		AssetName = FText::FromString( EditableContainers[0].TagContainerOwner->GetName() );
		Title = FText::Format( LOCTEXT("AssetTypeActions_MessageTagAssetBaseEditorTitle", "Tag Editor: Owned Message Tags: {0}"), AssetName );
	}

	TSharedPtr<SWindow> Window = SNew(SWindow)
		.Title(Title)
		.ClientSize(FVector2D(600, 400))
		[
			SNew(SMessageTagWidget, EditableContainers)
		];
#endif
	IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
	if (MainFrameModule.GetParentWindow().IsValid())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(Window.ToSharedRef(), MainFrameModule.GetParentWindow().ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddWindow(Window.ToSharedRef());
	}
}

uint32 FAssetTypeActions_MessageTagAssetBase::GetCategories()
{ 
	return EAssetTypeCategories::Misc; 
}

#undef LOCTEXT_NAMESPACE
