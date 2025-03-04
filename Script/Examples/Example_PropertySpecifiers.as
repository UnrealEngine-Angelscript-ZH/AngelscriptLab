class AExamplePropertySpecifierActor : AActor
{
    /* When declaring properties, Property Specifiers can be added to the declaration to control how 
     * the property behaves with various aspects of the Engine and Editor.
     */

    /* Adding UPROPERTY allows the property to be visible to Unreal.
     * Properties using UPROPERTY are editable by default.
     */
    UPROPERTY()
    float UnrealFloatProperty;

    /* Properties set to DefaultComponent will be created as components on the class automatically.
	 * Without it, it would just be a reference to any component.
    */
    UPROPERTY(DefaultComponent)
    USceneComponent SceneComponent;

    // Setting RootComponent makes this component the default root for this class.
    UPROPERTY(DefaultComponent, RootComponent)
    USceneComponent RootSceneComponent;

    // Attach will attach the component to the specified component.
    UPROPERTY(DefaultComponent, Attach = RootSceneComponent)
    UBillboardComponent DinoComponent;

	// AttachSocket will attach to the specified socket, if valid
	UPROPERTY(DefaultComponent, Attach = RootSceneComponent, AttachSocket = Face)
    USceneComponent FaceAttachedComponent;

    /* ShowOnActor makes the component's properties visible when you select the actor,
     * not just when you select the component. 
	 * 'ShowOnlyInnerProperties' will expand the properties to outer layer
     */
    UPROPERTY(DefaultComponent, ShowOnActor, meta = (ShowOnlyInnerProperties))
    USplineComponent SplineComponent;

    /* Category allows you to specifiy a category for the property
     * Using | allows you to add sub categories
     */
    UPROPERTY(Category = "Main Category|Sub Category")
    float CategorizedFloat = 1337.0;

    // NotEditable hides the property from the details panel
    UPROPERTY(NotEditable)
    bool bCoolBool;

    /* EditConst makes the property uneditable
     * (Still visible in the details panel, but uneditable)
     */
    UPROPERTY(EditConst)
    bool bReallyCoolBool;

    // BlueprintReadOnly: Can Get, but cannot Set in blueprints.
    UPROPERTY(BlueprintReadOnly)
    bool bReadOnlyBool;

    // The meta tag MakeEditWidget turns the FVector or FTransform into a 3D edit widget
    UPROPERTY(meta = (MakeEditWidget))
    FVector WidgetEditableVector;


    /* Using the meta EditCondition and InlineEditConditionToggle you can create a float assigned to a bool.
     * While false, the float is unable to be edited.
     */
    UPROPERTY(meta = (EditCondition = "bEditConditionBool"))
	float ConditionalFloat;
	UPROPERTY(meta = (InlineEditConditionToggle))
	bool bEditConditionBool = true;

    /* Using the meta EditCondition you can create a float assigned to an enum.
	 * The 'EditConditionHides' will hide the propperty, else it will be grey 
     */
    UPROPERTY()
	EExampleEnum ShowType = EExampleEnum::A;
    UPROPERTY(meta = (EditCondition="ShowType == EExampleEnum::A"))
    float ShownOnA = 1.0;
	 UPROPERTY(meta = (EditCondition="ShowType == EExampleEnum::B", EditConditionHides))
    float ShownOnB = 1.0;

	// This will clamp the values in the editor window
	UPROPERTY(meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float ExampleClampedValue;

};   