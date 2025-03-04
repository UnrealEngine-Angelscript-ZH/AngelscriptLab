class AFVectorOperator : AActor
{
    UPROPERTY(DefaultComponent, RootComponent)
    USceneComponent Root;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        float F = 0.0f;
        FVector V = FVector(0.0f, 0.0f, 0.0f);

        auto X = V * F;
        // ZH: 下面这行会报错, 原因是FVector没有实现 float * FVector 运算符
        // EN: The following line will cause an error, because FVector does not implement the float * FVector operator
        // auto y = F * V;

    }
};