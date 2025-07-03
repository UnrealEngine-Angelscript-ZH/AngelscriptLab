class APCGVolumeExample_AS : APCGVolumeExample
{
    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
    }

    UFUNCTION(CallInEditor)
    void CreateNewPCGGraph()
    {
        auto PCGGraph = Cast<UPCGGraph>(NewObject(this, UPCGGraph::StaticClass()));
        auto SelectPoints = Cast<UPCGSelectPointsSettings>(NewObject(this, UPCGSelectPointsSettings::StaticClass()));
        SelectPoints.bDebug = true;
        auto ToPonint = Cast<UPCGCollapseSettings>(NewObject(this, UPCGCollapseSettings::StaticClass()));
        
        auto SelectPointsNode = PCGGraph.AddNodeInstance(SelectPoints);
        // SelectPointsNode.GetSettings().bDebug = true;
        // SelectPointsNode.GetSettings().Modify();
        auto ToPonintNode = PCGGraph.AddNodeInstance(ToPonint);

        auto InputNode = PCGGraph.GetInputNode();
        auto OutPutNode = PCGGraph.GetOutputNode();
        InputNode.AddEdgeTo(n"In", ToPonintNode, n"In");
        ToPonintNode.AddEdgeTo(n"Out", SelectPointsNode, n"In");
        SelectPointsNode.AddEdgeTo(n"Out", OutPutNode, n"Out");

        PCGComponent.Graph = PCGGraph;
    }
};