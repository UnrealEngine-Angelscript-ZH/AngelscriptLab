class AGMPTest : AActor
{
    UPROPERTY(DefaultComponent, RootComponent)
    USceneComponent Root;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
		GMP::ListenObjectMessage(n"Event.Test", this, this, n"Test");

        float a = 123.0f;
		GMP::SendObjectMessage(n"Event.Test", this, a, 321);
    }

    UFUNCTION()
	void Test(float x, int32 y)
	{
		Print(f"Test {x}, {y}");
	}

    UFUNCTION(CallInEditor)
    void Send()
    {
        float a = 123.0f;
		GMP::SendObjectMessage(n"Event.Test", this, a, 321);
    }
};