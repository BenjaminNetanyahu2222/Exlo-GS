#pragma once
#include "pch.h"

#include "inventory.h"

namespace Map_1v1 {
	void Give1v1Items(AFortPlayerControllerAthena* PC) {
		Inventory::GiveItem(PC, StaticLoadObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_R_Ore_T03.WID_Assault_Auto_Athena_R_Ore_T03"), 1, 30);
		Inventory::GiveItem(PC, StaticLoadObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_VR_Ore_T03.WID_Shotgun_Standard_Athena_VR_Ore_T03"), 1, 5);
		Inventory::GiveItem(PC, StaticLoadObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Consumables/ShockwaveGrenade/Athena_ShockGrenade.Athena_ShockGrenade"), 6);

		Inventory::GiveItem(PC, Wood, 999);
		Inventory::GiveItem(PC, Stone, 999);
		Inventory::GiveItem(PC, Metal, 999);

		Inventory::GiveItem(PC, Shells, 34);
		Inventory::GiveItem(PC, LightAmmo, 300);
		Inventory::GiveItem(PC, MediumAmmo, 180);
	}
}