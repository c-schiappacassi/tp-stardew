#include "community_center.h"

#include <algorithm>
#include <stdexcept>

CommunityCenter::CommunityCenter(const CommunityConfig& config) : config(config) {
    config.validate();
    for (const auto& [item, quantity] : config.requirements) progress.donations.emplace(item, 0);
}

const CommunityConfig& CommunityCenter::getConfig() const { return config; }
const CommunityProgress& CommunityCenter::getProgress() const { return progress; }
uint64_t CommunityCenter::getRevision() const { return revision; }
bool CommunityCenter::isComplete(CommunityBundle bundle) const { return progress.completed.contains(bundle); }
bool CommunityCenter::finished() const { return progress.completed.size() == community_bundle::all.size(); }

uint32_t CommunityCenter::remaining(ItemId item) const {
    const auto bundle = community_bundle::forItem(item);
    if (!bundle || isComplete(*bundle)) return 0;
    return config.requirements.at(item) - progress.donations.at(item);
}

uint32_t CommunityCenter::remainingCoins() const {
    return isComplete(CommunityBundle::Money) ? 0 : config.moneyRequired - progress.coins;
}

void CommunityCenter::donate(ItemId item, uint32_t quantity) {
    if (quantity == 0 || quantity > remaining(item)) throw std::invalid_argument("Invalid item donation");
    progress.donations.at(item) += quantity;
    checkBundles();
    ++revision;
}

void CommunityCenter::donateCoins(uint32_t quantity) {
    if (quantity == 0 || quantity > remainingCoins()) throw std::invalid_argument("Invalid coin donation");
    progress.coins += quantity;
    checkBundles();
    ++revision;
}

void CommunityCenter::observeCombat(const CombatProgressData& combat) {
    const auto& current = progress.combat;
    if ((!combat.skeletons || current.skeletons) && (!combat.vampires || current.vampires) &&
        (!combat.orcs || current.orcs) && (!combat.skull || current.skull)) return;
    progress.combat.skeletons = progress.combat.skeletons || combat.skeletons;
    progress.combat.vampires = progress.combat.vampires || combat.vampires;
    progress.combat.orcs = progress.combat.orcs || combat.orcs;
    progress.combat.skull = progress.combat.skull || combat.skull;
    checkBundles();
    ++revision;
}

void CommunityCenter::checkBundles() {
    for (const auto bundle : {CommunityBundle::Crops, CommunityBundle::Metals, CommunityBundle::Animals}) {
        const bool ready = std::all_of(config.requirements.begin(), config.requirements.end(),
            [&](const auto& requirement) {
                return community_bundle::forItem(requirement.first) != bundle ||
                    progress.donations.at(requirement.first) == requirement.second;
            });
        if (ready) progress.completed.insert(bundle);
    }
    if (progress.coins == config.moneyRequired) progress.completed.insert(CommunityBundle::Money);
    const auto& combat = progress.combat;
    if (combat.skeletons && combat.vampires && combat.orcs && combat.skull) {
        progress.completed.insert(CommunityBundle::Combat);
    }
}

void CommunityCenter::complete(CommunityBundle bundle) {
    if (std::find(community_bundle::all.begin(), community_bundle::all.end(), bundle) == community_bundle::all.end()) {
        throw std::invalid_argument("Unknown community bundle");
    }
    if (isComplete(bundle)) return;
    for (auto& [item, quantity] : progress.donations) {
        if (community_bundle::forItem(item) == bundle) quantity = config.requirements.at(item);
    }
    if (bundle == CommunityBundle::Combat) progress.combat = {true, true, true, true};
    if (bundle == CommunityBundle::Money) progress.coins = config.moneyRequired;
    checkBundles();
    ++revision;
}

void CommunityCenter::validate(const CommunityProgress& saved) const {
    if (saved.donations.size() != config.requirements.size() || saved.coins > config.moneyRequired) {
        throw std::invalid_argument("Invalid saved community progress");
    }
    for (const auto& [item, required] : config.requirements) {
        const auto donation = saved.donations.find(item);
        if (donation == saved.donations.end() || donation->second > required) {
            throw std::invalid_argument("Invalid saved donation");
        }
    }
    CommunityCenter checked(config);
    checked.progress = saved;
    checked.progress.completed.clear();
    checked.checkBundles();
    if (checked.progress.completed != saved.completed) throw std::invalid_argument("Inconsistent saved bundles");
}

void CommunityCenter::restore(const CommunityProgress& saved) {
    validate(saved);
    progress = saved;
    ++revision;
}
