#ifndef SAVE_SCORE_SCENE_H
#define SAVE_SCORE_SCENE_H

#include "Scene.h"
#include "ScoreData.h"
#include "Colors.h"

class SaveScoreScene : public Scene
{
    // Basic nav: X left/right to change selection, button to activate
    static constexpr float HYSTERESIS_THRESHOLD = 0.45f;  // Simple hysteresis with thresholds
    static const millis_t SELECT_WAIT = 500;              // wait after select for drama
    static constexpr millis_t kShowTextDuration = (3000); // ms to show a text
    static constexpr Color333 kTextColor = Colors::Muted::White;
    static constexpr Color333 kSelectedColor = ColorHSV333(0, 0, 150);
    const SceneKind origin_;
    const char *originLabel_;
    ScoreData payload_;
    millis_t startTime_ = 0;
    int cursorIndex_ = 0;

    enum Stage
    {
        ShowIntro,
        InputName,
        ShowSaved,
        End
    };
    Stage stage_ = Stage::ShowIntro;

    void setStage(AppContext &ctx, Stage newStage);
    void showIntro(AppContext &ctx);
    void handleInputName(AppContext &ctx);
    void moveLeft();
    void moveRight();
    void drawName(AppContext &ctx);
    void drawCarets(AppContext &ctx, int tx, int ty);
    void showSaved(AppContext &ctx);

public:
    SaveScoreScene(SceneKind kind, const char *label, int newScore) : origin_(kind), originLabel_(label)
    {
        payload_.score = newScore;
        for (int i = 0; i < payload_.kMaxNameLength; ++i)
            payload_.name[i] = '_';
    }

    SceneKind kind() const override { return SceneKind::SaveScore; }
    const char *label() const override { return "SaveScore"; }

    void setup(AppContext &ctx) override;

    void loop(AppContext &ctx) override;
};

#endif
