#include "QRScene.h"
#include "qrcodegen.hpp"
#include "Colors.h"
#include "Logging.h"

using namespace qrcodegen;

void QRScene::setup(AppContext &ctx)
{
    const char *url = "https://bryanluu.github.io/";

    // Encode the URL as a low-error QR (ECL = LOW gives smallest)
    const QrCode qr = QrCode::encodeText(url, QrCode::Ecc::LOW);

    const int size = qr.getSize();
    ctx.logger.logf(LogLevel::Info, "QR size: %d x %d", size, size);

    // used for centering the QR
    const int kXOffset = (MATRIX_WIDTH - size) / 2;
    const int kYOffset = (MATRIX_HEIGHT - size) / 2;

    for (int y = 0; y < size; y++)
    {
        for (int x = 0; x < size; x++)
        {
            bool black = qr.getModule(x, y);
            ctx.gfx.setSafe(x + kXOffset, y + kYOffset, black ? Colors::Black : Colors::Muted::White);
        }
    }
}

void QRScene::loop(AppContext &ctx)
{
}
