#pragma once
#include <include/core/SkTypeface.h>

// Returns the shared system typeface (Segoe UI via DirectWrite), initialized once.
sk_sp<SkTypeface> skGetSystemTypeface();
