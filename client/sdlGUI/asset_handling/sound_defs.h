#pragma once

#include <cstddef>
#include <string>

#include "../utils.h"

namespace SoundDefs {
    struct SoundDef {
        SoundId id;
        std::string file;
        float gain;
    };

    inline const SoundDef SOUNDS[] = {
        {SoundId::StepGrass, "miscellaneous/grassyStep.wav", 0.45f},
        {SoundId::StepStone, "miscellaneous/stoneStep.wav",  0.45f},
        {SoundId::StepWood,  "miscellaneous/woodyStep.wav",  0.45f},
        
        {SoundId::PlayerHurt, "miscellaneous/ow.wav",          1.0f},
        {SoundId::SwordSwing, "miscellaneous/daggerswipe.wav", 0.8f},
        
        {SoundId::PlantWatered, "miscellaneous/water_scoop1.wav", 0.7f},
        {SoundId::SoilTilled,   "miscellaneous/hoe.wav",          0.8f},
        
        {SoundId::ChestOpen,  "miscellaneous/openChest.wav", 0.9f},
        {SoundId::ChestClose, "miscellaneous/doorClose.wav", 0.7f},

        {SoundId::TrapSpikes, "miscellaneous/shwip.wav",      0.7f},
        {SoundId::TrapFlame,  "miscellaneous/fireball.wav", 0.7f},

        {SoundId::OreHit,   "miscellaneous/hammer.wav",       0.9f},
        {SoundId::OreBreak, "miscellaneous/boulderBreak.wav", 1.0f},
        
        {SoundId::EnemyHurtSkeleton, "characters/skeletonHit.wav",  1.0f},
        {SoundId::EnemyHurtGolem,    "characters/rockGolemHit.wav", 1.0f},
        {SoundId::EnemyHurtVampire,  "characters/batScreech.wav",    0.5f},
        {SoundId::VampireFrenzy,     "characters/batScreech.wav",   1.0f},
        {SoundId::SkullFire,         "miscellaneous/flameSpell.wav",  0.8f},
        {SoundId::EnemyHurtGeneric,  "miscellaneous/hitEnemy.wav",  1.0f},
 
        {SoundId::ChickenCall, "characters/chicken_cluck.wav", 0.7f},
        {SoundId::CowMoo,      "characters/cow_moo1.wav",      0.9f},
 
        {SoundId::AmbienceFarm,    "ambience/spring_day.wav", 0.5f},
        {SoundId::AmbienceTown,    "ambience/spring_day.wav", 0.5f},
        {SoundId::AmbienceDungeon, "ambience/dark_cave.wav",  0.5f},
    };
}
