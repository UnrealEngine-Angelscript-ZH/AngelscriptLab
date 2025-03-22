class AEnumToString : AActor
{
    UPROPERTY(DefaultComponent, RootComponent)
    USceneComponent Root;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        FString Out0;
        EnumToString(EAILockSource::Gameplay, Out0);
        Print(Out0);
        FString Out1;
        GetTypeString(this, Out1);
        Print(Out1);
    }
};
