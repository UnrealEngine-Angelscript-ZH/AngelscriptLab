class AExample_EI_PlayerController : APlayerController
{
    UPROPERTY(Category = "Input")
    UInputAction Action;

    UPROPERTY(Category = "Input")
    UInputMappingContext Context;

    UEnhancedInputComponent InputComponent;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
		InputComponent = UEnhancedInputComponent::Create(this);
        PushInputComponent(InputComponent);

        UEnhancedInputLocalPlayerSubsystem EnhancedInputSubsystem = UEnhancedInputLocalPlayerSubsystem::Get(this);
        EnhancedInputSubsystem.AddMappingContext(Context, 0, FModifyContextOptions());

        InputComponent.BindAction(Action, ETriggerEvent::Triggered, FEnhancedInputActionHandlerDynamicSignature(this, n"Input_Action"));
    }

    UFUNCTION()
    void Input_Action(FInputActionValue ActionValue, float32 ElapsedTime, float32 TriggeredTime, UInputAction SourceAction)
    {
        Print(f"Input_Action[{ActionValue.ToString()}, {ElapsedTime}, {TriggeredTime}, {SourceAction.ToString()}]");
    }
};
