/* 
 * This file provides a simple example of how to use attribute sets from Angelscript
 */

// We use this auxillary namespace to make sure we have the 
namespace UExample_GASAttributes
{
    const FName HealthName = n"Health";
    const FName SpeedName = n"Speed";
}

// We use FAngelscriptGameplayAttributeData here to get access to some helpers. It's also required for replication.
class UExample_GASAttributes : UAngelscriptAttributeSet
{
    // Example non-replicated health attribute.
    UPROPERTY(BlueprintReadOnly, Category = "Pawn Attributes")
    FAngelscriptGameplayAttributeData Health;
    
    // Example replicated speed attribute.
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ReplicationTrampoline, Category = "Pawn Attributes")
    FAngelscriptGameplayAttributeData Speed;
    
    UExample_GASAttributes()
    {
        // Giving an initial value can be a nice way to detect errors and if you don't want to have more advanced initialization. 
        // Two good options for initializating attributes are by using data tables or gameplay effects. See the general GAS docs for more info on this.
        Health.Initialize(100.0f);
        Speed.Initialize(1.0f);
    }

    // To get attribute replication to properly work, GAS needs to know when replication has occurred, so we need to forward the data like this.
    // It's not well documented, but if you add an argument of the attribute data type to the callback, you can get access to the old attribute data to send along.
    // We use this to make GAS attribute replication to work.
    UFUNCTION()
    void OnRep_ReplicationTrampoline(FAngelscriptGameplayAttributeData& OldAttributeData)
    {
        OnRep_Attribute(OldAttributeData);
    }
};
