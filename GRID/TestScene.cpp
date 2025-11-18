#include "TestScene.h"

void TestScene::test_hue_sweep(Matrix32 &m)
{
    const int W = MATRIX_WIDTH;
    const int H = MATRIX_HEIGHT;

    for (int x = 0; x < W; ++x)
    {
        uint16_t h = static_cast<uint16_t>((x * HUE_RANGE) / std::max(1, W - 1));

        Color333 cTop = ColorHSV333(h, /*s=*/255, /*v=*/255, /*gamma=*/false);
        Color333 cBottom = ColorHSV333(h, /*s=*/255, /*v=*/255, /*gamma=*/true);

        for (int y = 0; y < H; ++y)
        {
            m.drawPixel(x, y, (y < H / 2) ? cTop : cBottom);
        }
    }
    m.show();
}

void TestScene::test_brightness_ramp(Matrix32 &m)
{
    const int W = MATRIX_WIDTH;
    const int H = MATRIX_HEIGHT;
    const uint16_t hue = HUE(240); // blue

    for (int x = 0; x < W; ++x)
    {
        uint8_t v = static_cast<uint8_t>((x * 255) / std::max(1, W - 1));
        Color333 cTop = ColorHSV333(hue, 255, v, /*gamma=*/false);
        Color333 cBottom = ColorHSV333(hue, 255, v, /*gamma=*/true);

        for (int y = 0; y < H; ++y)
        {
            m.drawPixel(x, y, (y < H / 2) ? cTop : cBottom);
        }
    }
    m.show();
}

void TestScene::test_primary_blocks(Matrix32 &m)
{
    const uint16_t hues[] = {HUE(0), HUE(60), HUE(120), HUE(180), HUE(240), HUE(300)};
    const int n = sizeof(hues) / sizeof(hues[0]);
    const int bw = std::max(1, MATRIX_WIDTH / n);

    for (int i = 0; i < n; ++i)
    {
        Color333 c = ColorHSV333(hues[i], 255, 255, /*gamma=*/true);
        const int x0 = i * bw;
        const int x1 = std::min(MATRIX_WIDTH, (i + 1) * bw);
        for (int x = x0; x < x1; ++x)
        {
            for (int y = 0; y < MATRIX_HEIGHT; ++y)
            {
                m.drawPixel(x, y, c);
            }
        }
    }
    m.show();
}

void TestScene::test_saturation_stripes(Matrix32 &m)
{
    for (int y = 0; y < MATRIX_HEIGHT; ++y)
    {
        uint8_t s = (y % 2 == 0) ? 0 : 255;
        for (int x = 0; x < MATRIX_WIDTH; ++x)
        {
            uint16_t h = static_cast<uint16_t>((x * HUE_RANGE) / std::max(1, MATRIX_WIDTH - 1));
            Color333 c = ColorHSV333(h, s, 255, /*gamma=*/true);
            m.drawPixel(x, y, c);
        }
    }
    m.show();
}

void TestScene::run_color_tests(AppContext &ctx)
{
    Matrix32 &m = ctx.gfx;
    m.begin();
    m.clear();

    test_hue_sweep(m);
    ctx.time.sleep(2000);
    m.clear();

    test_brightness_ramp(m);
    ctx.time.sleep(2000);
    m.clear();

    test_primary_blocks(m);
    ctx.time.sleep(2000);
    m.clear();

    test_saturation_stripes(m);
}

#ifdef GRID_EMULATION

#include "FileStorage.h"

// Smoke test using current working directory/save
void run_file_storage_smoke(FileStorage &storage, ILogger &logger)
{
    const char *baseDir = "save"; // creates ./save next to the emulator
    if (!storage.init(baseDir, &logger))
    {
        logger.logf(LogLevel::Warning, "[FileStorageTest] init failed");
        return;
    }

    const char *fname = "emu_test.bin"; // stored under ./save
    const char payload[] = "hello-grid";
    if (!storage.writeAll(fname, payload, sizeof(payload)))
    {
        logger.logf(LogLevel::Warning, "[FileStorageTest] writeAll failed");
        return;
    }
    logger.logf(LogLevel::Info, "[FileStorageTest] write ok");

    char buf[64] = {};
    auto r = storage.readAll(fname, buf, sizeof(buf));
    if (!r)
    {
        logger.logf(LogLevel::Warning, "[FileStorageTest] readAll failed");
        return;
    }
    logger.logf(LogLevel::Info, "[FileStorageTest] read %d bytes, contents='%s'", r.bytes, buf);

    // Clean up
    if (!storage.removeFile(fname))
    {
        logger.logf(LogLevel::Warning, "[FileStorageTest] removeFile failed");
        return;
    }
    logger.logf(LogLevel::Info, "[FileStorageTest] removed ok");
}

void TestScene::run_storage_smoke(IStorage &storage, ILogger &logger)
{
    FileStorage &fs = static_cast<FileStorage &>(storage);
    run_file_storage_smoke(fs, logger);
}

#else

#include "FlashStorage.h"

// Simple helper to run once in setup
void run_flash_storage_smoke(FlashStorage &storage, ILogger &logger)
{
    const char *baseDir = "/save";
    if (!storage.init(baseDir, &logger))
    {
        logger.logf(LogLevel::Warning, "[FlashStorageTest] init failed");
        return;
    }

    const char *fname = "flash_test.bin"; // stored under /save
    const char payload[] = "hello-grid";
    if (!storage.writeAll(fname, payload, sizeof(payload)))
    {
        logger.logf(LogLevel::Warning, "[FlashStorageTest] writeAll failed");
        return;
    }
    logger.logf(LogLevel::Info, "[FlashStorageTest] write ok");

    char buf[64] = {};
    auto r = storage.readAll(fname, buf, sizeof(buf));
    if (!r)
    {
        logger.logf(LogLevel::Warning, "[FlashStorageTest] readAll failed");
        return;
    }
    logger.logf(LogLevel::Info, "[FlashStorageTest] read %d bytes, contents='%s'", r.bytes, buf);

    // Clean up
    if (!storage.removeFile(fname))
    {
        logger.logf(LogLevel::Warning, "[FlashStorageTest] removeFile failed");
        return;
    }
    logger.logf(LogLevel::Info, "[FlashStorageTest] removed ok");
    logger.flush();
}

void TestScene::run_storage_smoke(IStorage &storage, ILogger &logger)
{
    FlashStorage &fs = static_cast<FlashStorage &>(storage);
    run_flash_storage_smoke(fs, logger);
}

#endif // GRID_EMULATION

void TestScene::setup(AppContext &ctx)
{
    run_storage_smoke(ctx.storage, ctx.logger);
    run_color_tests(ctx);
}
