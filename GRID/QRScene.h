#ifndef QR_SCENE_H
#define QR_SCENE_H

#include "Scene.h"

class QRScene : public Scene
{
public:
    QRScene() = default;
    ~QRScene() override = default;
    SceneKind kind() const override { return SceneKind::QR; }
    const char *label() const override { return "QR"; }
    void setup(AppContext &ctx) override;
    void loop(AppContext &ctx) override;
};

#endif // QR_SCENE_H
