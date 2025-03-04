/* 
 * This file provides a simple example of how a consumer class can use GAS in Angelscript.
 */

class UExample_GASAnimInstance : UAnimInstance
{
	// It's generally a good idea to store a cached version of the attribute if you intend to use it outside of the immediate callbacks.
	private float32 CachedSpeed = 1.0f;

	UFUNCTION(BlueprintOverride)
	void BlueprintBeginPlay()
	{
		// This will register our interest for attribute sets changing. We need to do this if we want to be guaranteed that an attribute set is registered before we attempt to hook up any callbacks.
		// OnAttributeSetRegistered is set up so that it will immediately call the callback function if the set has already been registered when you perform this registration, so you don't have to worry about order here.
		Cast<AAngelscriptGASCharacter>(TryGetPawnOwner()).AbilitySystem.OnAttributeSetRegistered(this, n"OnAttributeSetRegistered");
	}

	UFUNCTION()
	void OnAttributeSetRegistered(UAngelscriptAttributeSet NewAttributeSet)
	{
		// In here, we can register our attribute callback, and also hook up the callback itself.
		if(NewAttributeSet.IsA(UExample_GASAttributes::StaticClass()))
		{
			// Here we use the auxillary namespace to avoid having magic strings in our code as much as possible.
			AAngelscriptGASCharacter Character = Cast<AAngelscriptGASCharacter>(OwningActor);
			Character.AbilitySystem.GetAndRegisterCallbackForAttribute(UExample_GASAttributes::StaticClass(), UExample_GASAttributes::SpeedName, CachedSpeed);
			Character.AbilitySystem.OnAttributeChanged.AddUFunction(this, n"OnAttributeChanged");
		}
	}

	UFUNCTION()
	private void OnAttributeChanged(const FAngelscriptModifiedAttribute&in AttributeChangeData)
	{
		// Here we can test the attribute name using the auxillary namespace again, and perform our callback logic.
		if(AttributeChangeData.Name == UExample_GASAttributes::SpeedName)
		{
			CachedSpeed = AttributeChangeData.NewValue;	
		}
	}
};
