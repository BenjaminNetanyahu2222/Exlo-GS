#pragma once
#include "pch.h"

namespace Abilities {
	static inline FGameplayAbilitySpec* FindAbilitySpecFromHandle(UAbilitySystemComponent* AbilitySysComponent, FGameplayAbilitySpecHandle& Handle) {
		for (auto& Item : AbilitySysComponent->ActivatableAbilities.Items)
			if (Item.Handle.Handle == Handle.Handle)
				return &Item;
		
		return nullptr;
	}

    static inline void InternalServerTryActivateAbility(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, FPredictionKey& PredictionKey, FGameplayEventData* TriggerEventData) {
        auto Spec = FindAbilitySpecFromHandle(AbilitySystemComponent, Handle);
        if (!Spec)
            return AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);

        UGameplayAbility* AbilityToActivate = Spec->Ability;
        UGameplayAbility* InstancedAbility = nullptr;

        Spec->InputPressed = true;

        if (!Funcs::InternalTryActivateAbility(AbilitySystemComponent, Handle, PredictionKey, &InstancedAbility, nullptr, TriggerEventData)) {
            AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
            Spec->InputPressed = false;
            AbilitySystemComponent->ActivatableAbilities.MarkItemDirty(*Spec);
        }
    }

    static inline void GiveAbilitySet(AFortPlayerControllerAthena* Controller, UFortAbilitySet* Set) {
        if (Set) {
            for (auto& GameplayAbility : Set->GameplayAbilities) {
                if (!GameplayAbility->DefaultObject)
                    return;

                FGameplayAbilitySpec Spec{};
                Funcs::ConstructSpec(&Spec, GameplayAbility->DefaultObject, 1, -1, nullptr);
                Funcs::GiveAbility(((AFortPlayerStateAthena*)Controller->PlayerState)->AbilitySystemComponent, &Spec.Handle, Spec);
            }
        }
    }
};