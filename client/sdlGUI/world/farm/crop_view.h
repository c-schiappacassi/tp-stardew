#pragma once
#include <optional>

#include "../../utils.h"
#include "../entity_view.h"

#include "position.h"
#include "protocol_types.h"
#include "dtos.h"

class CropView: public EntityView {
private:
    Position pos{0, 0};
    CropType type;
    CropState state;
    bool wattered;
    // TODO: TDB: puntero a un campo del HUD? para que se active el cartel de "ready" para cada tipo de crop

public:
    explicit CropView(const CropData& data);

    void update(const CropData& data);

    std::optional<float> depth(const RenderInfo& ctx) const override;
    void draw(const RenderInfo& ctx) override;
};