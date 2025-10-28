#ifndef SAVE_SCORE_SCENE_H
#define SAVE_SCORE_SCENE_H

#include "Scene.h"
#include "ScoreData.h"

class SaveScoreScene : public Scene
{
    static constexpr millis_t kShowTextDuration = (3000); // ms to show a text
    const SceneKind origin_;
    const char *originLabel_;
    ScoreData payload_;
    millis_t startTime = 0;

    enum Stage
    {
        ShowIntro,
        InputName,
        ShowSaved,
        End
    };
    Stage stage = Stage::ShowIntro;

    void setStage(AppContext &ctx, Stage newStage);
    void showIntro(AppContext &ctx);

public:
    SaveScoreScene(SceneKind kind, const char *label, int newScore) : origin_(kind), originLabel_(label)
    {
        payload_.score = newScore;
    }

    SceneKind kind() const override { return SceneKind::SaveScore; }
    const char *label() const override { return "SaveScore"; }

    void setup(AppContext &ctx) override;

    void loop(AppContext &ctx) override;
};

#endif
