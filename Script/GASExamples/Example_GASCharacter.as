/* 
 * This file provides a simple example of how to implement an actor using GAS in Angelscript.
 */

class AExample_GASCharacter : AAngelscriptGASCharacter
{
	UFUNCTION(BlueprintOverride)
	void BeginPlay()
	{
		// This is how we register an attribute set with an actor.
		AbilitySystem.RegisterAttributeSet(UExample_GASAttributes::StaticClass());
	}
};
