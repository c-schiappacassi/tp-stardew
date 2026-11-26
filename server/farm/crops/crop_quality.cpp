#include "crop_quality.h"

#include "server_constants.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {
void accumulate(std::chrono::milliseconds& total, std::chrono::milliseconds elapsed) {
    if (elapsed.count() < 0) {
        throw std::invalid_argument(server_constants::NEGATIVE_TIME);
    }
    total += std::min(elapsed, std::chrono::milliseconds::max() - total);
}
}

CropQuality::CropQuality(std::chrono::milliseconds dayLength,std::chrono::milliseconds wateringInterval):
    dayLength(dayLength), wateringInterval(wateringInterval) {
    if (dayLength.count() <= 0 || wateringInterval.count() <= 0) {
        throw std::invalid_argument(server_constants::CROP_TIMES_MUST_BE_POSITIVE);
    }
}

void CropQuality::addWateringDelay(std::chrono::milliseconds elapsed) {
    accumulate(wateringDelay, elapsed);
}

void CropQuality::addHarvestDelay(std::chrono::milliseconds elapsed) {
    accumulate(harvestDelay, elapsed);
}

void CropQuality::recordWaterRequest() {
    if (waterRequests < std::numeric_limits<uint64_t>::max()) {
        ++waterRequests;
    }
}
void CropQuality::reset() {
    wateringDelay = std::chrono::milliseconds{0};
    harvestDelay = std::chrono::milliseconds{0};
    waterRequests = 1;
}

Quality CropQuality::evaluate(double luck) const {
    if (!std::isfinite(luck) || luck < 0.0) {
        throw std::invalid_argument(server_constants::LUCK_MUST_BE_FINITE_AND_NONNEGATIVE);
    }
    const double waterPenalty = static_cast<double>(wateringDelay.count()) /
        (static_cast<double>(waterRequests) * wateringInterval.count());
    const double harvestPenalty = static_cast<double>(harvestDelay.count()) / dayLength.count();
    const double careScore = std::clamp(1.0 - waterPenalty - harvestPenalty, 0.0, 1.0);
    const double score = std::clamp(careScore * luck, 0.0, 1.0);
    if (score >= 0.8) {
        return Quality::Perfect;
    } else if (score >= 0.6) {
        return Quality::Incredible;
    } else if (score >= 0.4) {
        return Quality::Excellent;
    } else if (score >= 0.2) {
        return Quality::Good;
    } else {
        return Quality::Base;
    }
}
