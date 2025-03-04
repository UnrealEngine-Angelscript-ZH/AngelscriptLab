// An AI Behavior Tree Decorator written in Angelscript
class UBTDecorator_ExampleDecorator : UBTDecorator_BlueprintBase
{
	default NodeName = "Example Decorator";

	// Changing this
	UPROPERTY(Category=ExampleDecorator)
	bool bShouldExecute = false;


	// Called whenever this Decorator performs a condition check in the Behavior Tree
	UFUNCTION(BlueprintOverride)
	bool PerformConditionCheckAI(AAIController OwnerController, APawn ControlledPawn)
	{
		Print("Performing condition check for Example Decorator!");
		return bShouldExecute;
	}

	// Called whenever this Decorator is ticked by the Behavior Tree
	UFUNCTION(BlueprintOverride)
	void TickAI(AAIController OwnerController, APawn ControlledPawn, float DeltaSeconds)
	{
		Print("Ticking Example Decorator!");
	}
};


// An AI Behavior Tree Service written in Angelscript
class UBTService_ExampleService : UBTService_BlueprintBase
{
	default NodeName = "Example Service";

	// Some value we want to expose to the graph
	UPROPERTY(Category=ExampleService)
	float SomeValue = 50.0;


	// Called whenever this Service is activated by the Behavior Tree
	UFUNCTION(BlueprintOverride)
	void ActivationAI(AAIController OwnerController, APawn ControlledPawn)
	{
		Print("Activating Example Service!");
	}

	// Called whenever this Service is ticked by the Behavior Tree
	UFUNCTION(BlueprintOverride)
	void TickAI(AAIController OwnerController, APawn ControlledPawn, float DeltaSeconds)
	{
		Print("Ticking Example Service!");
	}
};


// An AI Behavior Tree Task written in Angelscript
class UBTTask_ExampleTask : UBTTask_BlueprintBase
{
	default NodeName = "Example Task";

	// Some value we want to expose to the graph
	UPROPERTY(Category=ExampleTask)
	float SomeValue = 50.0;


	// Called whenever this Task is first executed by the Behavior Tree
	UFUNCTION(BlueprintOverride)
	void ExecuteAI(AAIController OwnerController, APawn ControlledPawn)
	{
		Print("Executing Example Task!");
		FinishExecute(true); // Pass false here if this node should inform the Behavior Tree that it failed
	}

	// Called while this Task is active in the Behavior Tree
	UFUNCTION(BlueprintOverride)
	void TickAI(AAIController OwnerController, APawn ControlledPawn, float DeltaSeconds)
	{
		Print("Ticking Example Task!");
	}

	// Called when the Behavior Tree aborts this Task's execution
	UFUNCTION(BlueprintOverride)
	void AbortAI(AAIController OwnerController, APawn ControlledPawn)
	{
		Print("Aborting Example Task!");
	}
};
