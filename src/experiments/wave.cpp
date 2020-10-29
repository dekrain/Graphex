#include "../context.hpp"

#include <cmath>

constexpr static int buf_size = 1024;
static float buffer[buf_size];

static const float C_tau = 2 * std::acos(-1);

void Update(GrContext& ctx, GrMessage msg) {
    switch (msg) {
        case GrMessage::Init:
        for (int i = 0; i < buf_size; ++i) {
            // Generate waveform
            float sample = 0.8f / 1 * std::sin(C_tau * 1.f * i / float(buf_size))
                         + 0.8f / 3 * std::sin(C_tau * 3.f * i / float(buf_size))
                         + 0.8f / 5 * std::sin(C_tau * 5.f * i / float(buf_size))
                         + 0.8f / 7 * std::sin(C_tau * 7.f * i / float(buf_size))
                         + 0.8f / 9 * std::sin(C_tau * 9.f * i / float(buf_size));
            buffer[i] = sample;
        }
        break;
        case GrMessage::Render:
        {
            GrClearBuffer(ctx.framebuffer, 0xFF'FF'FF'FF);
            long bw, bh;
            GetBitmapSize(ctx.framebuffer, bw, bh);

            long center_y = bh/2;

            // Draw the waveform
            GrColor color = 0xFF'CC5011; // r = 11h; g = 50h; b = CCh;
            for (int i = 0; i < buf_size; ++i) {
                long px = i * bw/buf_size;
                long py = center_y - buffer[i] * center_y;

                GrDrawLine(ctx.framebuffer, color, {px, center_y}, {px, py});
            }
        }
        break;
    }
}
