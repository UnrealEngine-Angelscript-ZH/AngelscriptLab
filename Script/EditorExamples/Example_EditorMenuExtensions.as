#if EDITOR

/**
 * Shows how to add new options to the context menu for actors in a level,
 * using UScriptActorMenuExtension.
 */
class UExampleActorMenuExtension : UScriptActorMenuExtension
{
	// Specify one or more classes for which the menu options show 
	default SupportedClasses.Add(AActor::StaticClass());

	// Every function with the CallInEditor specifier will become a context menu option
	UFUNCTION(CallInEditor)
	void ExampleActorMenuExtension()
	{
	}

	// The function's Category will be used to create sub-menus
	UFUNCTION(CallInEditor, Category = "Example Category")
	void ExampleOptionInSubCategory()
	{
	}

	// Custom icons can be specified with the `EditorIcon` meta tag:
	UFUNCTION(CallInEditor, Category = "Example Category", Meta = (EditorIcon = "Icons.Link"))
	void ExampleOptionWithIcon()
	{
	}

	// If the function takes an actor parameter, it will be called once for every selected actor
	UFUNCTION(CallInEditor, Category = "Example Category")
	void CalledForEverySelectedActor(AActor SelectedActor)
	{
		Print(f"Actor {SelectedActor} is selected");
	}

	// If the function has any other parameters, a dialog popup will be shown prompting for values
	UFUNCTION(CallInEditor, Category = "Example Category")
	void OpensPromptForParameters(AActor SelectedActor, bool bCheckboxParameter = true, FVector VectorParameter = FVector::ZeroVector)
	{
	}
};

/**
 * Using UScriptAssetMenuExtension allows adding options to the context menu
 * of assets in the Content Browser.
 */
class UExampleAssetMenuExtension : UScriptAssetMenuExtension
{
	// These options should only show up for textures
	default SupportedClasses.Add(UTexture2D::StaticClass());

	// When clicked, will be called once for every selected texture asset
	UFUNCTION(CallInEditor, Category = "Example Texture Actions")
	void ExampleModifyTextureLODBias(FAssetData SelectedAsset, int LODBias = 0)
	{
		UTexture2D SelectedTexture = Cast<UTexture2D>(SelectedAsset.GetSoftObjectPath().TryLoad());
		SelectedTexture.Modify();
		SelectedTexture.LODBias = LODBias;
	}
};

/**
 * Other editor menus can be extended with UScriptEditorMenuExtension.
 */
class UExampleEditorMenuExtension : UScriptEditorMenuExtension
{
	// This is the same extension point used by UToolMenus::ExtendMenu
	// In this example, we extend the top menu of the main window:
	default ExtensionPoint = n"MainFrame.MainMenu";

	UFUNCTION(CallInEditor, Category = "My Example Menu")
	void ExampleMainMenuOption()
	{
	}

	UFUNCTION(CallInEditor, Category = "My Example Menu | Example Sub Menu")
	void ExampleSubMenuOption()
	{
	}
};

/**
 * Toolbars can also be extended. Each function will become its own toolbar button.
 */
class UExampleToolbarExtension : UScriptEditorMenuExtension
{
	// Add to the extension point at the end of the level editor toolbar:
	default ExtensionPoint = n"LevelEditor.LevelEditorToolBar.User";

	// Will become a toolbar button displayed using the editor's "Paste" icon
	UFUNCTION(CallInEditor, Meta = (EditorIcon = "GenericCommands.Paste"))
	void ExampleToolbarButton()
	{
	}

	// Customize the style to make the display name appear in the toolbar instead of just the icon
	UFUNCTION(CallInEditor, DisplayName = "Extension", Meta = (EditorIcon = "Icons.Plus", EditorButtonStyle = "CalloutToolbar"))
	void ExampleToolbarButtonWithDisplayName()
	{
	}
};

#endif
