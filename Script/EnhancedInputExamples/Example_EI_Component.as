// This example shows you that you can move all of the
// input handling to its own Actor Components as opposed
// to putting it all in PlayerController/PlayerCharacter
// as seen in `Example_EI_PlayerController.as`

class UExample_EI_InputComponent : UEnhancedInputComponent
{
    UPROPERTY(Category = "Input Actions")
    UInputAction Action;

    UPROPERTY(Category = "Input Actions")
    UInputMappingContext Context;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        APlayerController PlayerController = Cast<APlayerController>(GetOwner());
        PlayerController.PushInputComponent(this);
        UEnhancedInputLocalPlayerSubsystem EnhancedInputSubsystem = UEnhancedInputLocalPlayerSubsystem::Get(PlayerController);
        EnhancedInputSubsystem.AddMappingContext(Context, 0, FModifyContextOptions());

        BindAction(Action, ETriggerEvent::Triggered, FEnhancedInputActionHandlerDynamicSignature(this, n"Input_Action"));
    }

    UFUNCTION()
    void Input_Action(FInputActionValue ActionValue, float32 ElapsedTime, float32 TriggeredTime, UInputAction SourceAction)
    {
        Print(f"Input_Action[{ActionValue.ToString()}, {ElapsedTime}, {TriggeredTime}, {SourceAction.ToString()}]");
    }
};

// class AExample_EI_PlayerController : APlayerController
// {
//   UPROPERTY(DefaultComponent)
//   UExample_EI_InputComponent InputComponent;
// };
