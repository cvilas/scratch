// Embeds PPFraktionMono-Regular directly into the binary at compile time.
// Requires C++23 (#embed).  The font is an OTF file; SDL_ttf / FreeType
// handle OTF identically to TTF.
//
// #embed searches relative to this source file's directory, so the path
// below resolves to plotlib/fonts/fraktion-mono/... at build time.

#include "embedded_font.h"

namespace {
    constexpr unsigned char kFontData[] = {
#embed "../fonts/fraktion-mono/PPFraktionMono-Regular-BF675904a6a1564.otf"
    };
} // namespace

std::span<const unsigned char> embeddedFontData() noexcept {
    return {kFontData, sizeof kFontData};
}
