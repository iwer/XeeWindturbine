// Copyright (c) Iwer Petersen. All rights reserved.
#pragma once

#include "Engine.h"
#include "Windshear.generated.h"


/**
 * Windshear calculations according to https://wind-data.ch/tools/profile.php?lng=de
 */
UENUM(BlueprintType)
enum EWindshearGroundType {
    WSGT_Water            UMETA(ToolTip = "Water surfaces: seas and lakes"),
    WSGT_Open             UMETA(ToolTip = "Open terrain with smooth surafce, e.g. concrete, airport runways, mown grass etc."),
    WSGT_Gentle           UMETA(ToolTip = "Open agricultural land without fences, hedges; maybe some far apart buildings and very gentle hills"),
    WSGT_Agricultural1KM  UMETA(ToolTip = "Agricultural land with a few buildings and 8m high hedges separated by more than 1 km"),
    WSGT_Agricultural500m UMETA(ToolTip = "Agricultural land with a few buildings and 8m high hedges separated by approx 500 m"),
    WSGT_Agricultural250m UMETA(ToolTip = "Agricultural land with a few buildings and 8m high hedges separated by approx 250 m"),
    WSGT_Village          UMETA(ToolTip = "Towns, villages, agricultural land with many or high hedges, forests and very rough and uneven terrain"),
    WSGT_Town             UMETA(ToolTip = "Large towns with high buildings"),
    WSGT_City             UMETA(ToolTip = "Large cities with high buildings and skyscrapers")
};

USTRUCT(BlueprintType)
struct FWindshearData {
    GENERATED_BODY()

    static constexpr float RoughnessClass[] = {0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0};
    static constexpr float RoughnessLength[] = {0.0002, 0.0024, 0.03, 0.055, 0.1, 0.2, 0.4, 0.6, 1.6};
};
