#pragma once

#include <cstddef>
#include <span>

/// Returns a read-only view of the Fraktion Mono OTF font data compiled
/// directly into the binary via C++23 #embed.  Prefer this over scanning
/// system font paths – the data is always available and has zero I/O cost.
std::span<const unsigned char> embeddedFontData() noexcept;
