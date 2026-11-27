#pragma once

#include <chrono>
#include <cstdint>
#include "protocol_types.h"

class CropQuality {
private:
    std::chrono::milliseconds dayLength;
    std::chrono::milliseconds wateringInterval;
    std::chrono::milliseconds wateringDelay{0};
    std::chrono::milliseconds harvestDelay{0};
    uint64_t waterRequests = 1;

    static void accumulate(std::chrono::milliseconds& total, std::chrono::milliseconds elapsed);

public:
    CropQuality(std::chrono::milliseconds dayLength, std::chrono::milliseconds wateringInterval);
    void addWateringDelay(std::chrono::milliseconds elapsed);
    void addHarvestDelay(std::chrono::milliseconds elapsed);
    void recordWaterRequest();
    void reset();
    Quality evaluate(double luck) const;
};
