#ifndef SAVE_SCORE_SCENE_H
#define SAVE_SCORE_SCENE_H

#include "Scene.h"
#include "ScoreData.h"
#include "Colors.h"

class SaveScoreScene : public Scene
{
    // Basic nav: X left/right to change selection, button to activate
    static constexpr float kHysteresisThreshold = 0.45f;  // Simple hysteresis with thresholds
    static constexpr millis_t kSubmitPause = 100;         // wait after select for drama
    static constexpr millis_t kShowTextDuration = (3000); // ms to show a text
    static constexpr millis_t kAlphabetStrobeDelay = 300; // ms to wait between pulses of select input
    static const Color333 kTextColor;
    static const Color333 kSelectedColor;
    static const Color333 kSubmitColor;
    static const Color333 kErrorColor;
    static constexpr int kAlphabetSize = 27;
    static constexpr char kAlphabet[kAlphabetSize + 1] = "_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const SceneKind origin_;
    const char *originLabel_;
    ScoreData payload_;
    millis_t startTime_ = 0;
    int cursorIndex_ = 0;
    int alphabetIndex_ = 0;
    bool submit_ = false;

    enum Stage
    {
        ShowIntro,
        InputName,
        ShowSaved,
        ShowError,
        End
    };
    Stage stage_ = Stage::ShowIntro;

    void setStage(AppContext &ctx, Stage newStage);
    void showIntro(AppContext &ctx);
    void handleInputName(AppContext &ctx);
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    void drawName(AppContext &ctx);
    void drawCarets(AppContext &ctx, int tx, int ty);
    bool submitScoreData(AppContext &ctx, ScoreData &payload);
    void showSaved(AppContext &ctx);
    void showError(AppContext &ctx);

public:
    SaveScoreScene(SceneKind kind, const char *label, int newScore) : origin_(kind), originLabel_(label)
    {
        payload_.score = newScore;
        for (int i = 0; i < payload_.kMaxNameLength; ++i)
            payload_.name[i] = '_';
        payload_.name[payload_.kMaxNameLength] = 0;
    }

    SceneKind kind() const override { return SceneKind::SaveScore; }
    const char *label() const override { return "SaveScore"; }

    void setup(AppContext &ctx) override;

    void loop(AppContext &ctx) override;
};

#endif
