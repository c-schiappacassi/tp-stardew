#include "../entity_view.h"
#include "protocol_types.h"

class OreView : public EntityView{
private:
    uint64_t entityId;
    ResourceType type;
    // TODO: TBD animacionde romperse?
    
public:
    void update(const OreData& data);
    virtual std::optional<float> depth(const RenderInfo& ctx) const = 0;
    virtual void draw(const RenderInfo& ctx) = 0;
};