# Gameplay Message Router

This is the Gameplay Message Router plugin provided in the Lyra template, forked to add support for AngelScript. Below is example usage:

``` as
struct FSimpleBool {
  bool bValue;
}

class AMyActor : AActor {
  UFUNCTION(BlueprintOverride)
  void BeginPlay() {
    UGameplayMessageSubsystem::Get().RegisterListener(
      GameplayTags::Input_Enabled,
      this,
      n"HandlePlayerEnabled",
      FSimpleBool()
    );
  }

  UFUNCTION()
  void HandlePlayerEnabled(FGameplayTag ActualTag, FSimpleBool Data) {
    Print("Player enabled: " + Data.bValue ? "true" : "false");
  }
}

class AMyOtherActor : AActor {
  void SetEnabled(bool bEnabled) {
    FSimpleBool Data;
    Data.bValue = bEnabled;
    UGameplayMessageSubsystem::Get().BroadcastMessage(
      GameplayTags::Input_Enabled,
      Data
    );
  }
}
```
