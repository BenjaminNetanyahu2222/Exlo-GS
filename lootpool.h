#pragma once
#include "pch.h"

#include <algorithm>
#include <random>
#include <vector>
#include <stdexcept>

class Lootpool_ {
public:
	static std::mt19937& GetRNG() noexcept {
		static thread_local std::mt19937 gen([] {
			std::random_device rd;
			return std::mt19937(rd());
			}());

		return gen;
	}

	template<typename T>
	static T& Random(std::vector<T>& vec)  {
		if (vec.empty())
			throw std::out_of_range("vector is empty");

		return vec[std::uniform_int_distribution<size_t>(0, vec.size() - 1)(GetRNG())];
	}

	template<typename T>
	static const T& Random(const std::vector<T>& vec) {
		if (vec.empty())
			throw std::out_of_range("vector is empty");

		return vec[std::uniform_int_distribution<size_t>(0, vec.size() - 1)(GetRNG())];
	}
};

class Lootpool {
public:
	struct ItemEntry {
		UFortItemDefinition* Item;
		int Count;
		int Ammo;
	};

	void AddItem(UFortItemDefinition* Item, int Count = 1, int Ammo = 0) {
		items.push_back({ Item, Count, Ammo });
	}

	const std::vector<ItemEntry>& GetItems() const {
		return items;
	}

	void Clear() {
		items.clear();
	}

	static ItemEntry GetShotgun() {
		std::vector<ItemEntry> Shotguns{
			ItemEntry(StaticLoadObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_SR_Ore_T03.WID_Shotgun_Standard_Athena_SR_Ore_T03"), 1, 5),
			ItemEntry(StaticLoadObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_VR_Ore_T03.WID_Shotgun_Standard_Athena_VR_Ore_T03"), 1, 5),
		};
		return Lootpool_::Random(Shotguns);
	}

	static ItemEntry GetAR() {
		std::vector<ItemEntry> ARs{
			ItemEntry(StaticLoadObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_VR_Ore_T03.WID_Assault_AutoHigh_Athena_VR_Ore_T03"), 1, 30),
			ItemEntry(StaticLoadObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_SR_Ore_T03.WID_Assault_AutoHigh_Athena_SR_Ore_T03"), 1, 30),
		};
		return Lootpool_::Random(ARs);
	}

	static ItemEntry GetMovement() {
		std::vector<ItemEntry> Movement{
			ItemEntry(StaticLoadObject<UFortItemDefinition>(L"/Game/Athena/Items/Consumables/ShockwaveGrenade/Athena_ShockGrenade.Athena_ShockGrenade"), 3, 1),
		};
		return Lootpool_::Random(Movement);
	}

	static ItemEntry GetHeal() {
		std::vector<ItemEntry> Heals{
			ItemEntry(StaticLoadObject<UFortItemDefinition>(L"/Game/Athena/Items/Consumables/ShieldSmall/Athena_ShieldSmall.Athena_ShieldSmall"), 3, 1),
			ItemEntry(StaticLoadObject<UFortItemDefinition>(L"/Game/Athena/Items/Consumables/PurpleStuff/Athena_PurpleStuff.Athena_PurpleStuff"), 2, 1),
		};
		return Lootpool_::Random(Heals);
	}
private:
	std::vector<ItemEntry> items;
};