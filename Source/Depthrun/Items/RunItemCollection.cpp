// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "RunItemCollection.h"
#include "RunItemConfig.h"

URunItemConfig* URunItemCollection::FindByName(const FString& Name) const
{
	for (URunItemConfig* Item : Items)
	{
		if (Item && Item->ItemName.ToString().ToLower().Contains(Name.ToLower()))
		{
			return Item;
		}
	}
	return nullptr;
}
