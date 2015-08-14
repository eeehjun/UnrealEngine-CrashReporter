// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "HierarchicalLODOutlinerPrivatePCH.h"
#include "HLODOutliner.h"
#include "LevelEditor.h"
#include "LODLevelItem.h"
#include "HierarchicalLODUtils.h"
#include "ScopedTransaction.h"
#include "SlateBasics.h"
#include "TreeItemID.h"

#define LOCTEXT_NAMESPACE "LODLevelItem"

HLODOutliner::FLODLevelItem::FLODLevelItem(const uint32 InLODIndex)
	: LODLevelIndex(InLODIndex), ID(nullptr)
{
	Type = ITreeItem::HierarchicalLODLevel;
	ID.SetCachedHash(GetTypeHash(FString("LODLevel - ") + FString::FromInt(LODLevelIndex)));
}

bool HLODOutliner::FLODLevelItem::CanInteract() const
{
	return true;
}

void HLODOutliner::FLODLevelItem::GenerateContextMenu(FMenuBuilder& MenuBuilder, SHLODOutliner& Outliner)
{

}

FString HLODOutliner::FLODLevelItem::GetDisplayString() const
{
	return FString("LODLevel - ") + FString::FromInt(LODLevelIndex);
}

HLODOutliner::FTreeItemID HLODOutliner::FLODLevelItem::GetID()
{
	return ID;
}

void HLODOutliner::FLODLevelItem::PopulateDragDropPayload(FDragDropPayload& Payload) const
{

}

HLODOutliner::FDragValidationInfo HLODOutliner::FLODLevelItem::ValidateDrop(FDragDropPayload& DraggedObjects) const
{
	FLODLevelDropTarget Target(LODLevelIndex);
	return Target.ValidateDrop(DraggedObjects);
}

void HLODOutliner::FLODLevelItem::OnDrop(FDragDropPayload& DraggedObjects, const FDragValidationInfo& ValidationInfo, TSharedRef<SWidget> DroppedOnWidget)
{
	FLODLevelDropTarget Target(LODLevelIndex);
	return Target.OnDrop(DraggedObjects, ValidationInfo, DroppedOnWidget);
}

HLODOutliner::FDragValidationInfo HLODOutliner::FLODLevelDropTarget::ValidateDrop(FDragDropPayload& DraggedObjects) const
{
	if (DraggedObjects.StaticMeshActors.IsSet() && DraggedObjects.StaticMeshActors->Num() > 0)
	{
		const int32 NumStaticMeshActors = DraggedObjects.StaticMeshActors->Num();		
		return FDragValidationInfo(FHLODOutlinerDragDropOp::ToolTip_CompatibleNewCluster, LOCTEXT("CreateNewCluster", "Create new Cluster"));
	}
	else if (DraggedObjects.LODActors.IsSet() && DraggedObjects.LODActors->Num() > 0)
	{
		const int32 NumLODActors = DraggedObjects.LODActors->Num();

		if (NumLODActors > 1)
		{
			// Check if all the dragged LOD actors fall within the same LOD level
			auto LODActors = DraggedObjects.LODActors.GetValue();
			int32 LevelIndex = -1;
			bool bSameLODLevel = true;
			for (auto Actor : LODActors)
			{
				ALODActor* LODActor = Cast<ALODActor>(Actor.Get());
				if (LevelIndex == -1)
				{
					LevelIndex = LODActor->LODLevel;
				}
				else if (LevelIndex != LODActor->LODLevel)
				{
					bSameLODLevel = false;
				}
			}

			if ( bSameLODLevel && LevelIndex < (int32)( LODLevelIndex + 1 ) )
			{
				return FDragValidationInfo(FHLODOutlinerDragDropOp::ToolTip_MultipleSelection_CompatibleNewCluster, LOCTEXT("CreateNewCluster", "Create new Cluster"));
			}
			
		}
	}

	return FDragValidationInfo(FHLODOutlinerDragDropOp::ToolTip_Incompatible, LOCTEXT("NotImplemented", "Not implemented"));
}

void HLODOutliner::FLODLevelDropTarget::OnDrop(FDragDropPayload& DraggedObjects, const FDragValidationInfo& ValidationInfo, TSharedRef<SWidget> DroppedOnWidget)
{
	if (ValidationInfo.TooltipType == FHLODOutlinerDragDropOp::ToolTip_CompatibleNewCluster || ValidationInfo.TooltipType == FHLODOutlinerDragDropOp::ToolTip_MultipleSelection_CompatibleNewCluster)
	{
		CreateNewCluster(DraggedObjects);
	}
}

void HLODOutliner::FLODLevelDropTarget::CreateNewCluster(FDragDropPayload &DraggedObjects)
{	
	// Change this to pass world into OnDrop?
	UWorld* World = nullptr;
	if (DraggedObjects.StaticMeshActors.IsSet() && DraggedObjects.StaticMeshActors->Num() > 0)
	{
		World = DraggedObjects.StaticMeshActors.GetValue()[0]->GetWorld();
	}
	else if (DraggedObjects.LODActors.IsSet() && DraggedObjects.LODActors->Num() > 0)
	{
		World = DraggedObjects.LODActors.GetValue()[0]->GetWorld();
	}

	const FScopedTransaction Transaction(LOCTEXT("UndoAction_CreateNewCluster", "Create new Cluster"));
	World->Modify();
	
	ALODActor* NewCluster = HierarchicalLODUtils::CreateNewClusterActor(World, LODLevelIndex);

	for (TWeakObjectPtr<AActor> StaticMeshActor : DraggedObjects.StaticMeshActors.GetValue())
	{
		AActor* InActor = StaticMeshActor.Get();
		ALODActor* CurrentParentActor = HierarchicalLODUtils::GetParentLODActor(InActor);
		if (CurrentParentActor)
		{
			CurrentParentActor->RemoveSubActor(InActor);
		}

		NewCluster->AddSubActor(InActor);
	}

	for (TWeakObjectPtr<AActor> LODActor : DraggedObjects.LODActors.GetValue())
	{
		AActor* InActor = LODActor.Get();
		ALODActor* CurrentParentActor = HierarchicalLODUtils::GetParentLODActor(InActor);
		if (CurrentParentActor)
		{
			CurrentParentActor->RemoveSubActor(InActor);
		}

		NewCluster->AddSubActor(InActor);
	}
}

#undef LOCTEXT_NAMESPACE
