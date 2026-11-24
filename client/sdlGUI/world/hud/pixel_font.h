#pragma once

#include <cstdint>
#include <string>

#include <SDL2/SDL.h>

#include "../../render_info.h"

namespace pixel_font {
    constexpr int GLYPH_W = 3;
    constexpr int GLYPH_H = 5;
    constexpr int GLYPH_GAP = 1;

    inline constexpr uint8_t DIGITS[10][GLYPH_H] = {
        {0b111, 0b101, 0b101, 0b101, 0b111}, {0b010, 0b110, 0b010, 0b010, 0b111},
        {0b111, 0b001, 0b111, 0b100, 0b111}, {0b111, 0b001, 0b111, 0b001, 0b111},
        {0b101, 0b101, 0b111, 0b001, 0b001}, {0b111, 0b100, 0b111, 0b001, 0b111},
        {0b111, 0b100, 0b111, 0b101, 0b111}, {0b111, 0b001, 0b001, 0b001, 0b001},
        {0b111, 0b101, 0b111, 0b101, 0b111}, {0b111, 0b101, 0b111, 0b001, 0b111},
    };

    inline constexpr uint8_t LETTERS[26][GLYPH_H] = {
        {0b010, 0b101, 0b111, 0b101, 0b101},
        {0b110, 0b101, 0b110, 0b101, 0b110},
        {0b011, 0b100, 0b100, 0b100, 0b011},
        {0b110, 0b101, 0b101, 0b101, 0b110},
        {0b111, 0b100, 0b110, 0b100, 0b111},
        {0b111, 0b100, 0b110, 0b100, 0b100},
        {0b011, 0b100, 0b101, 0b101, 0b011},
        {0b101, 0b101, 0b111, 0b101, 0b101},
        {0b111, 0b010, 0b010, 0b010, 0b111},
        {0b001, 0b001, 0b001, 0b101, 0b010},
        {0b101, 0b101, 0b110, 0b101, 0b101},
        {0b100, 0b100, 0b100, 0b100, 0b111},
        {0b101, 0b111, 0b111, 0b101, 0b101},
        {0b110, 0b101, 0b101, 0b101, 0b101},
        {0b010, 0b101, 0b101, 0b101, 0b010},
        {0b110, 0b101, 0b110, 0b100, 0b100},
        {0b010, 0b101, 0b101, 0b111, 0b011},
        {0b110, 0b101, 0b110, 0b101, 0b101},
        {0b011, 0b100, 0b010, 0b001, 0b110},
        {0b111, 0b010, 0b010, 0b010, 0b010},
        {0b101, 0b101, 0b101, 0b101, 0b111},
        {0b101, 0b101, 0b101, 0b101, 0b010},
        {0b101, 0b101, 0b111, 0b111, 0b101},
        {0b101, 0b101, 0b010, 0b101, 0b101},
        {0b101, 0b101, 0b010, 0b010, 0b010},
        {0b111, 0b001, 0b010, 0b100, 0b111},
    };

    struct SymbolGlyph {
        char ch;
        uint8_t rows[GLYPH_H];
    };

    inline constexpr SymbolGlyph SYMBOLS[] = {
        {'!',  {0b010, 0b010, 0b010, 0b000, 0b010}},
        {'"',  {0b101, 0b101, 0b000, 0b000, 0b000}},
        {'#',  {0b101, 0b111, 0b101, 0b111, 0b101}},
        {'$',  {0b011, 0b110, 0b010, 0b011, 0b110}},
        {'%',  {0b101, 0b001, 0b010, 0b100, 0b101}},
        {'&',  {0b010, 0b101, 0b010, 0b101, 0b011}},
        {'\'', {0b010, 0b010, 0b000, 0b000, 0b000}},
        {'(',  {0b001, 0b010, 0b010, 0b010, 0b001}},
        {')',  {0b100, 0b010, 0b010, 0b010, 0b100}},
        {'*',  {0b000, 0b101, 0b010, 0b101, 0b000}},
        {'+',  {0b000, 0b010, 0b111, 0b010, 0b000}},
        {',',  {0b000, 0b000, 0b000, 0b010, 0b100}},
        {'-',  {0b000, 0b000, 0b111, 0b000, 0b000}},
        {'.',  {0b000, 0b000, 0b000, 0b000, 0b010}},
        {'/',  {0b001, 0b001, 0b010, 0b100, 0b100}},
        {':',  {0b000, 0b010, 0b000, 0b010, 0b000}},
        {';',  {0b000, 0b010, 0b000, 0b010, 0b100}},
        {'<',  {0b001, 0b010, 0b100, 0b010, 0b001}},
        {'=',  {0b000, 0b111, 0b000, 0b111, 0b000}},
        {'>',  {0b100, 0b010, 0b001, 0b010, 0b100}},
        {'?',  {0b110, 0b001, 0b010, 0b000, 0b010}},
        {'@',  {0b010, 0b101, 0b111, 0b100, 0b011}},
        {'[',  {0b110, 0b100, 0b100, 0b100, 0b110}},
        {'\\', {0b100, 0b100, 0b010, 0b001, 0b001}},
        {']',  {0b011, 0b001, 0b001, 0b001, 0b011}},
        {'^',  {0b010, 0b101, 0b000, 0b000, 0b000}},
        {'_',  {0b000, 0b000, 0b000, 0b000, 0b111}},
        {'`',  {0b100, 0b010, 0b000, 0b000, 0b000}},
        {'{',  {0b011, 0b010, 0b110, 0b010, 0b011}},
        {'|',  {0b010, 0b010, 0b010, 0b010, 0b010}},
        {'}',  {0b110, 0b010, 0b011, 0b010, 0b110}},
        {'~',  {0b000, 0b011, 0b110, 0b000, 0b000}},
    };

    inline constexpr SDL_Color TEXT_COLOR{255, 255, 255, 255};
    inline constexpr SDL_Color SHADOW_COLOR{60, 30, 10, 255};

    inline const uint8_t* glyph_of(char ch) {
        if (ch >= '0' && ch <= '9') {
            return DIGITS[ch - '0'];
        }
        if (ch >= 'a' && ch <= 'z') {
            ch = static_cast<char>(ch - 'a' + 'A');
        }
        if (ch >= 'A' && ch <= 'Z') {
            return LETTERS[ch - 'A'];
        }
        for (const SymbolGlyph& s: SYMBOLS) {
            if (s.ch == ch) {
                return s.rows;
            }
        }
        return nullptr;
    }

    inline int width(const std::string& text, int px) {
        return (static_cast<int>(text.size()) * (GLYPH_W + GLYPH_GAP) - GLYPH_GAP) * px;
    }

    inline int height(int px) { return GLYPH_H * px; }

    inline void draw(const RenderInfo& ctx, const std::string& text, int x, int y, int px, const SDL_Color& color) {
        ctx.r.SetDrawColor(color.r, color.g, color.b, color.a);
        for (const char ch: text) {
            if (const uint8_t* glyph = glyph_of(ch)) {
                for (int row = 0; row < GLYPH_H; ++row) {
                    for (int col = 0; col < GLYPH_W; ++col) {
                        if (glyph[row] & (0b100 >> col)) {
                            ctx.r.FillRect(SDL2pp::Rect(x + col * px, y + row * px, px, px));
                        }
                    }
                }
            }
            x += (GLYPH_W + GLYPH_GAP) * px;
        }
    }

    inline void draw_shadowed(const RenderInfo& ctx, const std::string& text, int x, int y, int px) {
        draw(ctx, text, x + px, y + px, px, SHADOW_COLOR);
        draw(ctx, text, x, y, px, TEXT_COLOR);
    }
}
