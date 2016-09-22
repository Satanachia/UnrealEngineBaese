// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "WidgetTemplateClass.h"
#include "AssetData.h"

class UImage;

/**
 * A template for classes generated by UTexture or UMaterial classes, or implements a USlateTextureAtlasInterface
 */

class FWidgetTemplateImageClass : public FWidgetTemplateClass
{

public:
	/**
	 * Constructor.
	 * @param	InAssetData		The asset data used to create the widget
	 */
	FWidgetTemplateImageClass(const FAssetData& InAssetData);

	virtual ~FWidgetTemplateImageClass();

	/** Creates an instance of the widget for the tree */
	virtual UWidget* Create(UWidgetTree* WidgetTree) override;

	/** Returns the asset data for this widget */
	FAssetData GetWidgetAssetData();

	/** Returns true if the supplied class is supported by this template */
	static bool Supports(UClass* InClass);

private:
	FAssetData WidgetAssetData;
};