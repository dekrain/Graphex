#include "../context.hpp"

#include <cmath>
#include <complex>

constexpr static int buf_size = 256;
static float signal_buffer[buf_size];
static std::complex<float> dft_buffer[buf_size];
static float sin_buffer[buf_size]; // Single cycle of sine

static const float C_tau = 2 * std::acos(-1);
static constexpr std::complex<float> C_i{0.f, 1.f};

static void PerformDFTReal(const float* signal, std::complex<float>* output, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        std::complex<float> sum{};
        for (size_t j = 0; j < size; ++j) {
            sum += signal[j] * std::exp(-C_i * C_tau * float(i) * float(j) / float(size));
        }
        output[i] = sum / float(size); // Scaling
    }
}

void Update(GrContext& ctx, GrMessage msg) {
    switch (msg) {
        case GrMessage::Init: {
            float f = 1.f; // Fundamental frequency
            float a = .6f; // Fundamental amplitude
            for (int i = 0; i < buf_size; ++i) {
                // Generate sine buffer
                sin_buffer[i] = std::sin(C_tau * float(i) / buf_size);
            }
            for (int i = 0; i < buf_size; ++i) {
                // Generate waveform
                /*float sample{};
                float s = 1.f;
                for (int j = 1; f*j < buf_size/2.f; j += 1) {
                    sample += s * a / j * sin_buffer[int(f * j * i) % buf_size];
                    s = -s;
                }*/
                float sample = .5f * sin_buffer[3 * i % buf_size]
                             + .3f * sin_buffer[(5 * i + buf_size * 1/4) % buf_size]
                             + .1f;
                signal_buffer[i] = sample;
            }
            // Perform DFT
            PerformDFTReal(signal_buffer, dft_buffer, buf_size);
        }
        break;
        case GrMessage::Render:
        {
            GrClearBuffer(ctx.framebuffer, 0xFF'FF'FF'FF);
            long bw, bh;
            GetBitmapSize(ctx.framebuffer, bw, bh);

            long center_y = bh/2;

            // Draw the original waveform
            GrColor color = 0xFF'CC5011; // r = 11h; g = 50h; b = CCh;
            for (int i = 0; i < buf_size; ++i) {
                long px = i * bw/buf_size /2;
                long py = center_y - signal_buffer[i] * center_y;

                GrDrawLine(ctx.framebuffer, color, {px, center_y}, {px, py});
            }

            // Draw the DFT magnitude waveform
            color = 0xFF'0528C0; // r = C0h; g = 28h; b = 05h;
            for (int i = 0; i < buf_size; ++i) {
                float sample = std::abs(dft_buffer[i]);
                long px = i * bw/buf_size /2 + bw/2;
                long py = center_y - sample * center_y;

                GrDrawLine(ctx.framebuffer, color, {px, center_y}, {px, py});
            }

            // Draw the DFT phase waveform
            color = 0xFF'22E002; // r = 02h; g = E0h; b = 22h;
            for (int i = 0; i < buf_size; ++i) {
                float sample = std::fmod(std::arg(dft_buffer[i]) + C_tau, C_tau) / C_tau;
                long px = i * bw/buf_size /2 + bw/2;
                long py = center_y + sample * center_y;

                GrDrawLine(ctx.framebuffer, color, {px, center_y}, {px, py});
            }
        }
        break;
    }
}
