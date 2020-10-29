#pragma once

struct GrBitmap; // Defined by host
struct GrUserContext; // Defined by application

struct GrContext {
    GrBitmap* framebuffer;
    GrUserContext* user;
};

enum class GrMessage {
    Init, // Used to initialize user context
    Render,
};

struct GrColor {
    unsigned char r, g, b, a;

    constexpr GrColor(unsigned v)
        : r((v >> 0) & 0xFF), g((v >> 8) & 0xFF), b((v >> 16) & 0xFF), a((v >> 24) & 0xFF) {}
    
    constexpr operator unsigned() const {
        return unsigned(r) | (unsigned(g) << 8) | (unsigned(b) << 16) | (unsigned(a) << 24);
    }
};

struct GrPos {
    long x, y;
};

extern void* GetPixels(GrBitmap* bitmap);
extern void GetBitmapSize(const GrBitmap* bitmap, long& width, long& height);

extern void GrClearBuffer(GrBitmap* bmap, GrColor color);
extern void GrDrawLine(GrBitmap* bmap, GrColor color, GrPos start, GrPos end);

// Client-defined callback
extern void Update(GrContext& ctx, GrMessage msg);
