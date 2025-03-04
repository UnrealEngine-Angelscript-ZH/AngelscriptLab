
/** 
 * This is an example for an ACharacter that takes input, which can be used as
 * a baseclass for your main game player / pawn.
 */
class AExampleInputCharacter : ACharacter
{
    // An input component that we will set up to handle input from the player 
    // that is possessing this pawn.
    UPROPERTY(DefaultComponent)
    UInputComponent ScriptInputComponent;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        // Set up any action mappings we want to use while possessed
        //  The action names used can be configured within the project's input settings or DefaultInput.ini
        //  Note that these bindings consume the input and override any InputAction nodes in the blueprint
        ScriptInputComponent.BindAction(n"Jump", EInputEvent::IE_Pressed, FInputActionHandlerDynamicSignature(this, n"OnJumpPressed"));
        ScriptInputComponent.BindAction(n"Jump", EInputEvent::IE_Released, FInputActionHandlerDynamicSignature(this, n"OnJumpReleased"));

        // Set up some axis bindings to receive the values of control axes
        //  Note that these bindings consume the input and override any InputAxis nodes in the blueprint
        ScriptInputComponent.BindAxis(n"MoveForward", FInputAxisHandlerDynamicSignature(this, n"OnMoveForwardAxisChanged"));
        ScriptInputComponent.BindAxis(n"MoveRight", FInputAxisHandlerDynamicSignature(this, n"OnMoveRightAxisChanged"));

        // You can also bind to a specific hardcoded key, bypassing action mappings
        //   These bindings do not consume input, and work alongside action mappings.
        ScriptInputComponent.BindKey(EKeys::LeftShift, EInputEvent::IE_Pressed, FInputActionHandlerDynamicSignature(this, n"OnShiftPressed"));

        // You can bind to AnyKey to receive all key events and do your own manual mapping if you wish
        //   These bindings do not consume input, and work alongside action mappings.
        ScriptInputComponent.BindKey(EKeys::AnyKey, EInputEvent::IE_Pressed, FInputActionHandlerDynamicSignature(this, n"OnKeyPressed"));

        // Don't forget to call to parent if you override BeginPlay from blueprint!
    }

    UFUNCTION()
    void OnJumpPressed(FKey Key)
    {
        Print("Jump was pressed!", Duration=5.0);

        Jump();
    }

    UFUNCTION()
    void OnJumpReleased(FKey Key)
    {
        Print("Jump was released!", Duration=5.0);

        StopJumping();
    }

    UFUNCTION()
    void OnMoveForwardAxisChanged(float32 AxisValue)
    {
        Print("Move Forward Axis Value: "+AxisValue, Duration=0.0);

        AddMovementInput(ControlRotation.ForwardVector, AxisValue);
    }

    UFUNCTION()
    void OnMoveRightAxisChanged(float32 AxisValue)
    {
        Print("Move Right Axis Value: "+AxisValue, Duration=0.0);

        AddMovementInput(ControlRotation.RightVector, AxisValue);
    }

    UFUNCTION()
    void OnShiftPressed(FKey Key)
    {
        Print("Shift key pressed!", Duration=5.0);
    }

    UFUNCTION()
    void OnKeyPressed(FKey Key)
    {
        Print("Key Pressed: " + Key.KeyName, Duration=5.0);
    }
};