#include "Scene.h"

class ExampleScene final : public Scene
{
public:
    void setup(Matrix32& gfx) override;
    void loop(Matrix32& gfx, uint32_t dt) override;
};
