#pragma once

#include "../listeners/health_listener.h"
#include "../../../audio/sound_emitter.h"

#include "protocol_types.h"

class EnemyHurtSound : public HealthListener {
    SoundEmitter& sfx;
    SoundId sound;

    // TODO: mandar esto a asset manager tambien?
    static SoundId sound_for(EnemyType type) {
        switch (type) {
            case EnemyType::Skeleton: return SoundId::EnemyHurtSkeleton;
            case EnemyType::Orc: return SoundId::EnemyHurtGolem;
            case EnemyType::VampireMale:
            case EnemyType::VampireFemale: return SoundId::EnemyHurtVampire;
            default: return SoundId::EnemyHurtGeneric;
        }
    }

public:
    EnemyHurtSound(SoundEmitter& sfx, EnemyType type): sfx(sfx), sound(sound_for(type)) {}

    void on_damage(const HealthChange& change) override { sfx.cue(sound, change.pos); }
};
