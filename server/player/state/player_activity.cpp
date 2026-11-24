#include "player_activity.h"

PlayerActivity::PlayerActivity(const PlayerData& initialData):
        state(initialData.state == PlayerState::LookingChest ? PlayerState::Idle : initialData.state), direction(initialData.dir) {
    if (state == PlayerState::Walking)
        movementDirection = direction;
}

PlayerState PlayerActivity::getState() const { return state; }

Direction PlayerActivity::getDirection() const { return direction; }

bool PlayerActivity::canAct() const {
    return state == PlayerState::Idle || state == PlayerState::Walking || state == PlayerState::LookingChest;
}

bool PlayerActivity::isResting() const {
    return state == PlayerState::Sleeping || state == PlayerState::Unconscious;
}

bool PlayerActivity::isSleeping() const { return state == PlayerState::Sleeping; }

bool PlayerActivity::cooldownEnded(std::chrono::steady_clock::time_point now) const {
    return now >= nextActionTime;
}

void PlayerActivity::setMovement(Direction requested) {
    switch (requested) {
        case Direction::None:
        case Direction::Up:
        case Direction::Down:
        case Direction::Left:
        case Direction::Right:
            break;
        default:
            return;
    }
    if (isResting())
        return;

    movementDirection = requested;
    if (requested != Direction::None)
        closeChest();
    if (state == PlayerState::UsingTool || state == PlayerState::LookingChest)
        return;
    if (requested == Direction::None) {
        state = PlayerState::Idle;
    } else {
        direction = requested;
        state = PlayerState::Walking;
    }
}

void PlayerActivity::faceDirection(Direction requested) {
    switch (requested) {
        case Direction::Up:
        case Direction::Down:
        case Direction::Left:
        case Direction::Right:
            direction = requested;
            break;
        default:
            break;
    }
}

void PlayerActivity::stopMovementAfterRelocation() {
    closeChest();
    movementDirection = Direction::None;
    if (state == PlayerState::Walking)
        state = PlayerState::Idle;
}

void PlayerActivity::resumeMovement() { setMovement(movementDirection); }

void PlayerActivity::startCooldown(std::chrono::steady_clock::time_point now,std::chrono::milliseconds interval) {
    nextActionTime = now + interval;
}

void PlayerActivity::startTool(Direction toolDirection) {
    if (state == PlayerState::Unconscious)
        return;
    closeChest();
    direction = toolDirection;
    state = PlayerState::UsingTool;
}

void PlayerActivity::updateToolState(std::chrono::steady_clock::time_point now) {
    if (state == PlayerState::UsingTool && cooldownEnded(now)) {
        state = PlayerState::Idle;
        resumeMovement();
    }
}

bool PlayerActivity::passOut() {
    closeChest();
    if (isResting())
        return false;
    state = PlayerState::Unconscious;
    direction = Direction::Down;
    movementDirection = Direction::None;
    return true;
}

void PlayerActivity::sleep() {
    closeChest();
    state = PlayerState::Sleeping;
    direction = Direction::Down;
    movementDirection = Direction::None;
}

bool PlayerActivity::cancelSleep() {
    if (!isSleeping())
        return false;
    state = PlayerState::Idle;
    movementDirection = Direction::None;
    return true;
}

void PlayerActivity::wakeUp() {
    reset();
}

void PlayerActivity::reset() {
    closeChest();
    state = PlayerState::Idle;
    movementDirection = Direction::None;
    nextActionTime = {};
}

void PlayerActivity::restoreData(const PlayerData& saved) {
    closeChest();
    state = saved.state == PlayerState::LookingChest ? PlayerState::Idle : saved.state;
    direction = saved.dir;
    movementDirection = Direction::None;
}

std::chrono::steady_clock::time_point PlayerActivity::actionReadyAt() const {
    return nextActionTime;
}

void PlayerActivity::openChest(uint32_t id) {
    setMovement(Direction::None);
    openChestId = id;
    state = PlayerState::LookingChest;
}

void PlayerActivity::closeChest() {
    openChestId.reset();
    if (state == PlayerState::LookingChest)
        state = PlayerState::Idle;
}

std::optional<uint32_t> PlayerActivity::getOpenChestId() const { return openChestId; }
