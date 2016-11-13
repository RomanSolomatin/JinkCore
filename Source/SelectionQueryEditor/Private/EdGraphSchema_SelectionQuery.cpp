// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "SelectionQueryEditorPrivatePCH.h"
#include "EdGraphSchema_SelectionQuery.h"
#include "Toolkits/ToolkitManager.h"
#include "SelectionQuery/SelQueryGenerator.h"
#include "SelectionQuery/Generators/SelQueryGenerator_Composite.h"
#include "SelectionQuery/SelQueryTest.h"

#define LOCTEXT_NAMESPACE "SelectionQueryEditor"

//////////////////////////////////////////////////////////////////////////

UEdGraphSchema_SelectionQuery::UEdGraphSchema_SelectionQuery(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UEdGraphSchema_SelectionQuery::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	FGraphNodeCreator<USelectionQueryGraphNode_Root> NodeCreator(Graph);
	USelectionQueryGraphNode_Root* MyNode = NodeCreator.CreateNode();
	NodeCreator.Finalize();
	SetNodeMetaData(MyNode, FNodeMetadata::DefaultGraphNode);
}

void UEdGraphSchema_SelectionQuery::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	USelectionQueryGraphNode* ParentGraphNode = ContextMenuBuilder.FromPin ? Cast<USelectionQueryGraphNode>(ContextMenuBuilder.FromPin->GetOuter()) : NULL;
	if  (ParentGraphNode && !ParentGraphNode->IsA(USelectionQueryGraphNode_Root::StaticClass()))
	{
		return;
	}

	FSelectionQueryEditorModule& EditorModule = FModuleManager::GetModuleChecked<FSelectionQueryEditorModule>(TEXT("SelectionQueryEditor"));
	FGraphNodeClassHelper* ClassCache = EditorModule.GetClassCache().Get();
	
	TArray<FGraphNodeClassData> NodeClasses;
	ClassCache->GatherClasses(USelQueryGenerator::StaticClass(), NodeClasses);

	FCategorizedGraphActionListBuilder GeneratorsBuilder(TEXT("Generators"));
	for (const auto& NodeClass : NodeClasses)
	{
		const FText NodeTypeName = FText::FromString(FName::NameToDisplayString(NodeClass.ToString(), false));

		USelectionQueryGraphNode_Option* OpNode = NewObject<USelectionQueryGraphNode_Option>(ContextMenuBuilder.OwnerOfTemporaries);
		OpNode->ClassData = NodeClass;

		TSharedPtr<FAISchemaAction_NewNode> AddOpAction = AddNewNodeAction(GeneratorsBuilder, NodeClass.GetCategory(), NodeTypeName, "");
		AddOpAction->NodeTemplate = OpNode;
	}

	ContextMenuBuilder.Append(GeneratorsBuilder);
}

void UEdGraphSchema_SelectionQuery::GetSubNodeClasses(int32 SubNodeFlags, TArray<FGraphNodeClassData>& ClassData, UClass*& GraphNodeClass) const
{
	FSelectionQueryEditorModule& EditorModule = FModuleManager::GetModuleChecked<FSelectionQueryEditorModule>(TEXT("SelectionQueryEditor"));
	FGraphNodeClassHelper* ClassCache = EditorModule.GetClassCache().Get();

	ClassCache->GatherClasses(USelQueryTest::StaticClass(), ClassData);
	GraphNodeClass = USelectionQueryGraphNode_Test::StaticClass();
}

const FPinConnectionResponse UEdGraphSchema_SelectionQuery::CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const
{
	// Make sure the pins are not on the same node
	if (PinA->GetOwningNode() == PinB->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Both are on the same node"));
	}

	if ((PinA->Direction == EGPD_Input && PinA->LinkedTo.Num()>0) || 
		(PinB->Direction == EGPD_Input && PinB->LinkedTo.Num()>0))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT(""));
	}

	// Compare the directions
	bool bDirectionsOK = false;

	if ((PinA->Direction == EGPD_Input) && (PinB->Direction == EGPD_Output))
	{
		bDirectionsOK = true;
	}
	else if ((PinB->Direction == EGPD_Input) && (PinA->Direction == EGPD_Output))
	{
		bDirectionsOK = true;
	}

	if (bDirectionsOK)
	{
		if ( (PinA->Direction == EGPD_Input && PinA->LinkedTo.Num()>0) || (PinB->Direction == EGPD_Input && PinB->LinkedTo.Num()>0))
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Already connected with other"));
		}
	}
	else
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT(""));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
}

const FPinConnectionResponse UEdGraphSchema_SelectionQuery::CanMergeNodes(const UEdGraphNode* NodeA, const UEdGraphNode* NodeB) const
{
	// Make sure the nodes are not the same 
	if (NodeA == NodeB)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Both are the same node"));
	}

	const bool bNodeAIsTest = NodeA->IsA(USelectionQueryGraphNode_Test::StaticClass());
	const bool bNodeAIsOption = NodeA->IsA(USelectionQueryGraphNode_Option::StaticClass());
	const bool bNodeBIsTest = NodeB->IsA(USelectionQueryGraphNode_Test::StaticClass());
	const bool bNodeBIsOption = NodeB->IsA(USelectionQueryGraphNode_Option::StaticClass());

	if (bNodeAIsTest && (bNodeBIsOption || bNodeBIsTest))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT(""));
}

int32 UEdGraphSchema_SelectionQuery::GetNodeSelectionCount(const UEdGraph* Graph) const
{
	if (Graph)
	{
		TSharedPtr<ISelectionQueryEditor> SelQueryEditor;
		if (USelQuery* QueryAsset = Cast<USelQuery>(Graph->GetOuter()))
		{
			TSharedPtr< IToolkit > QueryAssetEditor = FToolkitManager::Get().FindEditorForAsset(QueryAsset);
			if (QueryAssetEditor.IsValid())
			{
				SelQueryEditor = StaticCastSharedPtr<ISelectionQueryEditor>(QueryAssetEditor);
			}
		}
		if (SelQueryEditor.IsValid())
		{
			return SelQueryEditor->GetSelectedNodesCount();
		}
	}

	return 0;
}

#undef LOCTEXT_NAMESPACE
