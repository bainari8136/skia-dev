#pragma once
#include "skWidget.h"
#include <string>
#include <vector>

// Block-level Markdown renderer (read-only).
// Supports: H1/H2/H3, paragraphs (word-wrapped), bullet lists,
// code blocks (```), horizontal rules (---).
// Call setContent() to set the Markdown text; call layout() after resize.
class skMarkdownView : public skWidget {
public:
    skMarkdownView(int x, int y, int w, int h);

    void setContent(const std::string& md);

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;

private:
    enum class BlockType { H1, H2, H3, Para, BulletItem, CodeLine, Rule };
    struct Block {
        BlockType type;
        std::string text;
    };

    std::string m_raw;
    std::vector<Block> m_blocks;
    int m_scroll = 0;   // pixel scroll offset
    int m_contentH = 0; // total rendered height

    static constexpr int kPad  = 12;
    static constexpr int kSbW  = 8;

    void parse();
};
