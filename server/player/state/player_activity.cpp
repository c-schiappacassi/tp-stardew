#include "player_activity.h"

PlayerActivity::PlayerActivity(const PlayerData& initialData):
        state(initialData.state), direction(initialData.dir) {
    if (state == PlayerState::Walking)
        movementDirection = direction;
}

PlayerState PlayerActivity::getState() const { return state; }

Direction PlayerActivity::getDirection() const { return direction; }

bool PlayerActivity::canAct() const {
    return state == PlayerState::Idle || state == PlayerState::Walking;
}

bool PlayerActivity::isResting() const {
    return state == PlayerState::Sleeping || state == PlayerState::Unconscious;
}

bool PlayerActivity::isSleeping() const { return state == PlayerState::Sleeping; }

bool PlayerActivity::isBedReady() const { return bedReady; }

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
    if (state == PlayerState::UsingTool)
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
    if (isResting())
        return false;
    state = PlayerState::Unconscious;
    movementDirection = Direction::None;
    return true;
}

void PlayerActivity::sleep() {
    state = PlayerState::Sleeping;
    movementDirection = Direction::None;
}

bool PlayerActivity::cancelSleep() {
    if (!isSleeping())
        return false;
    state = PlayerState::Idle;
    movementDirection = Direction::None;
    bedReady = false;
    return true;
}

void PlayerActivity::wakeUp() {
    bedReady = false;
    reset();
}

void PlayerActivity::leaveBedArea() { bedReady = true; }

void PlayerActivity::reset() {
    state = PlayerState::Idle;
    movementDirection = Direction::None;
    nextActionTime = {};
}

void PlayerActivity::restoreData(const PlayerData& saved) {
    state = saved.state;
    direction = saved.dir;
    movementDirection = Direction::None;
    bedReady = false;
}

std::chrono::steady_clock::time_point PlayerActivity::actionReadyAt() const {
    return nextActionTime;
}
