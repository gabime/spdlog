// Copyright(c) 2015-present, Gabi Melman & spdlog and fmtlib contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// ansicolor and rgb color helper&definitions.
#pragma once
#include <spdlog/common.h>

//#define SPDLOG_CONSTEXPR_FUNC

namespace spdlog {

enum class color : uint32_t
{
    alice_blue = 0xF0F8FF,              // rgb(240,248,255)
    antique_white = 0xFAEBD7,           // rgb(250,235,215)
    aqua = 0x00FFFF,                    // rgb(0,255,255)
    aquamarine = 0x7FFFD4,              // rgb(127,255,212)
    azure = 0xF0FFFF,                   // rgb(240,255,255)
    beige = 0xF5F5DC,                   // rgb(245,245,220)
    bisque = 0xFFE4C4,                  // rgb(255,228,196)
    black = 0x000000,                   // rgb(0,0,0)
    blanched_almond = 0xFFEBCD,         // rgb(255,235,205)
    blue = 0x0000FF,                    // rgb(0,0,255)
    blue_violet = 0x8A2BE2,             // rgb(138,43,226)
    brown = 0xA52A2A,                   // rgb(165,42,42)
    burly_wood = 0xDEB887,              // rgb(222,184,135)
    cadet_blue = 0x5F9EA0,              // rgb(95,158,160)
    chartreuse = 0x7FFF00,              // rgb(127,255,0)
    chocolate = 0xD2691E,               // rgb(210,105,30)
    coral = 0xFF7F50,                   // rgb(255,127,80)
    cornflower_blue = 0x6495ED,         // rgb(100,149,237)
    cornsilk = 0xFFF8DC,                // rgb(255,248,220)
    crimson = 0xDC143C,                 // rgb(220,20,60)
    cyan = 0x00FFFF,                    // rgb(0,255,255)
    dark_blue = 0x00008B,               // rgb(0,0,139)
    dark_cyan = 0x008B8B,               // rgb(0,139,139)
    dark_golden_rod = 0xB8860B,         // rgb(184,134,11)
    dark_gray = 0xA9A9A9,               // rgb(169,169,169)
    dark_green = 0x006400,              // rgb(0,100,0)
    dark_khaki = 0xBDB76B,              // rgb(189,183,107)
    dark_magenta = 0x8B008B,            // rgb(139,0,139)
    dark_olive_green = 0x556B2F,        // rgb(85,107,47)
    dark_orange = 0xFF8C00,             // rgb(255,140,0)
    dark_orchid = 0x9932CC,             // rgb(153,50,204)
    dark_red = 0x8B0000,                // rgb(139,0,0)
    dark_salmon = 0xE9967A,             // rgb(233,150,122)
    dark_sea_green = 0x8FBC8F,          // rgb(143,188,143)
    dark_slate_blue = 0x483D8B,         // rgb(72,61,139)
    dark_slate_gray = 0x2F4F4F,         // rgb(47,79,79)
    dark_turquoise = 0x00CED1,          // rgb(0,206,209)
    dark_violet = 0x9400D3,             // rgb(148,0,211)
    deep_pink = 0xFF1493,               // rgb(255,20,147)
    deep_sky_blue = 0x00BFFF,           // rgb(0,191,255)
    dim_gray = 0x696969,                // rgb(105,105,105)
    dodger_blue = 0x1E90FF,             // rgb(30,144,255)
    fire_brick = 0xB22222,              // rgb(178,34,34)
    floral_white = 0xFFFAF0,            // rgb(255,250,240)
    forest_green = 0x228B22,            // rgb(34,139,34)
    fuchsia = 0xFF00FF,                 // rgb(255,0,255)
    gainsboro = 0xDCDCDC,               // rgb(220,220,220)
    ghost_white = 0xF8F8FF,             // rgb(248,248,255)
    gold = 0xFFD700,                    // rgb(255,215,0)
    golden_rod = 0xDAA520,              // rgb(218,165,32)
    gray = 0x808080,                    // rgb(128,128,128)
    green = 0x008000,                   // rgb(0,128,0)
    green_yellow = 0xADFF2F,            // rgb(173,255,47)
    honey_dew = 0xF0FFF0,               // rgb(240,255,240)
    hot_pink = 0xFF69B4,                // rgb(255,105,180)
    indian_red = 0xCD5C5C,              // rgb(205,92,92)
    indigo = 0x4B0082,                  // rgb(75,0,130)
    ivory = 0xFFFFF0,                   // rgb(255,255,240)
    khaki = 0xF0E68C,                   // rgb(240,230,140)
    lavender = 0xE6E6FA,                // rgb(230,230,250)
    lavender_blush = 0xFFF0F5,          // rgb(255,240,245)
    lawn_green = 0x7CFC00,              // rgb(124,252,0)
    lemon_chiffon = 0xFFFACD,           // rgb(255,250,205)
    light_blue = 0xADD8E6,              // rgb(173,216,230)
    light_coral = 0xF08080,             // rgb(240,128,128)
    light_cyan = 0xE0FFFF,              // rgb(224,255,255)
    light_golden_rod_yellow = 0xFAFAD2, // rgb(250,250,210)
    light_gray = 0xD3D3D3,              // rgb(211,211,211)
    light_green = 0x90EE90,             // rgb(144,238,144)
    light_pink = 0xFFB6C1,              // rgb(255,182,193)
    light_salmon = 0xFFA07A,            // rgb(255,160,122)
    light_sea_green = 0x20B2AA,         // rgb(32,178,170)
    light_sky_blue = 0x87CEFA,          // rgb(135,206,250)
    light_slate_gray = 0x778899,        // rgb(119,136,153)
    light_steel_blue = 0xB0C4DE,        // rgb(176,196,222)
    light_yellow = 0xFFFFE0,            // rgb(255,255,224)
    lime = 0x00FF00,                    // rgb(0,255,0)
    lime_green = 0x32CD32,              // rgb(50,205,50)
    linen = 0xFAF0E6,                   // rgb(250,240,230)
    magenta = 0xFF00FF,                 // rgb(255,0,255)
    maroon = 0x800000,                  // rgb(128,0,0)
    medium_aquamarine = 0x66CDAA,       // rgb(102,205,170)
    medium_blue = 0x0000CD,             // rgb(0,0,205)
    medium_orchid = 0xBA55D3,           // rgb(186,85,211)
    medium_purple = 0x9370DB,           // rgb(147,112,219)
    medium_sea_green = 0x3CB371,        // rgb(60,179,113)
    medium_slate_blue = 0x7B68EE,       // rgb(123,104,238)
    medium_spring_green = 0x00FA9A,     // rgb(0,250,154)
    medium_turquoise = 0x48D1CC,        // rgb(72,209,204)
    medium_violet_red = 0xC71585,       // rgb(199,21,133)
    midnight_blue = 0x191970,           // rgb(25,25,112)
    mint_cream = 0xF5FFFA,              // rgb(245,255,250)
    misty_rose = 0xFFE4E1,              // rgb(255,228,225)
    moccasin = 0xFFE4B5,                // rgb(255,228,181)
    navajo_white = 0xFFDEAD,            // rgb(255,222,173)
    navy = 0x000080,                    // rgb(0,0,128)
    old_lace = 0xFDF5E6,                // rgb(253,245,230)
    olive = 0x808000,                   // rgb(128,128,0)
    olive_drab = 0x6B8E23,              // rgb(107,142,35)
    orange = 0xFFA500,                  // rgb(255,165,0)
    orange_red = 0xFF4500,              // rgb(255,69,0)
    orchid = 0xDA70D6,                  // rgb(218,112,214)
    pale_golden_rod = 0xEEE8AA,         // rgb(238,232,170)
    pale_green = 0x98FB98,              // rgb(152,251,152)
    pale_turquoise = 0xAFEEEE,          // rgb(175,238,238)
    pale_violet_red = 0xDB7093,         // rgb(219,112,147)
    papaya_whip = 0xFFEFD5,             // rgb(255,239,213)
    peach_puff = 0xFFDAB9,              // rgb(255,218,185)
    peru = 0xCD853F,                    // rgb(205,133,63)
    pink = 0xFFC0CB,                    // rgb(255,192,203)
    plum = 0xDDA0DD,                    // rgb(221,160,221)
    powder_blue = 0xB0E0E6,             // rgb(176,224,230)
    purple = 0x800080,                  // rgb(128,0,128)
    rebecca_purple = 0x663399,          // rgb(102,51,153)
    red = 0xFF0000,                     // rgb(255,0,0)
    rosy_brown = 0xBC8F8F,              // rgb(188,143,143)
    royal_blue = 0x4169E1,              // rgb(65,105,225)
    saddle_brown = 0x8B4513,            // rgb(139,69,19)
    salmon = 0xFA8072,                  // rgb(250,128,114)
    sandy_brown = 0xF4A460,             // rgb(244,164,96)
    sea_green = 0x2E8B57,               // rgb(46,139,87)
    sea_shell = 0xFFF5EE,               // rgb(255,245,238)
    sienna = 0xA0522D,                  // rgb(160,82,45)
    silver = 0xC0C0C0,                  // rgb(192,192,192)
    sky_blue = 0x87CEEB,                // rgb(135,206,235)
    slate_blue = 0x6A5ACD,              // rgb(106,90,205)
    slate_gray = 0x708090,              // rgb(112,128,144)
    snow = 0xFFFAFA,                    // rgb(255,250,250)
    spring_green = 0x00FF7F,            // rgb(0,255,127)
    steel_blue = 0x4682B4,              // rgb(70,130,180)
    tan = 0xD2B48C,                     // rgb(210,180,140)
    teal = 0x008080,                    // rgb(0,128,128)
    thistle = 0xD8BFD8,                 // rgb(216,191,216)
    tomato = 0xFF6347,                  // rgb(255,99,71)
    turquoise = 0x40E0D0,               // rgb(64,224,208)
    violet = 0xEE82EE,                  // rgb(238,130,238)
    wheat = 0xF5DEB3,                   // rgb(245,222,179)
    white = 0xFFFFFF,                   // rgb(255,255,255)
    white_smoke = 0xF5F5F5,             // rgb(245,245,245)
    yellow = 0xFFFF00,                  // rgb(255,255,0)
    yellow_green = 0x9ACD32             // rgb(154,205,50)
};                                      // enum class color

enum class terminal_color : uint8_t
{
    black = 30,
    red,
    green,
    yellow,
    blue,
    magenta,
    cyan,
    white,
    bright_black = 90,
    bright_red,
    bright_green,
    bright_yellow,
    bright_blue,
    bright_magenta,
    bright_cyan,
    bright_white
};

enum class emphasis : uint8_t
{
    bold = 1,
    faint = 1 << 1,
    italic = 1 << 2,
    underline = 1 << 3,
    blink = 1 << 4,
    reverse = 1 << 5,
    conceal = 1 << 6,
    strikethrough = 1 << 7,
};

struct rgb
{
    SPDLOG_CONSTEXPR_FUNC rgb()
        : r(0)
        , g(0)
        , b(0)
    {}
    SPDLOG_CONSTEXPR_FUNC rgb(uint8_t r_, uint8_t g_, uint8_t b_)
        : r(r_)
        , g(g_)
        , b(b_)
    {}
    SPDLOG_CONSTEXPR_FUNC rgb(uint32_t hex)
        : r((hex >> 16) & 0xFF)
        , g((hex >> 8) & 0xFF)
        , b(hex & 0xFF)
    {}
    SPDLOG_CONSTEXPR_FUNC rgb(color hex)
        : r((uint32_t(hex) >> 16) & 0xFF)
        , g((uint32_t(hex) >> 8) & 0xFF)
        , b(uint32_t(hex) & 0xFF)
    {}
    uint8_t r;
    uint8_t g;
    uint8_t b;
};


namespace details {

struct color_t
{
    SPDLOG_CONSTEXPR_FUNC color_t() SPDLOG_NOEXCEPT
        : is_rgb()
        , value{}
    {}
    SPDLOG_CONSTEXPR_FUNC color_t(color rgb_color) SPDLOG_NOEXCEPT
        : is_rgb(true)
        , value{}
    {
        value.rgb_color = static_cast<uint32_t>(rgb_color);
    }
    SPDLOG_CONSTEXPR_FUNC color_t(rgb rgb_color) SPDLOG_NOEXCEPT
        : is_rgb(true)
        , value{}
    {
        value.rgb_color = (static_cast<uint32_t>(rgb_color.r) << 16) | (static_cast<uint32_t>(rgb_color.g) << 8) | rgb_color.b;
    }
    SPDLOG_CONSTEXPR_FUNC color_t(terminal_color term_color) SPDLOG_NOEXCEPT
        : is_rgb()
        , value{}
    {
        value.term_color = static_cast<uint8_t>(term_color);
    }
    bool is_rgb;
    union color_union
    {
        uint8_t term_color;
        uint32_t rgb_color;
    } value;
};

// build ansi escape codes from styles
struct ansi_color_escape
{
    SPDLOG_CONSTEXPR_FUNC ansi_color_escape(details::color_t text_color, const char *esc) SPDLOG_NOEXCEPT : buffer{}
    {
        // If we have a terminal color, we need to output another escape code
        // sequence.
        if (!text_color.is_rgb)
        {
            bool is_background = esc == string_view_t("\x1b[48;2;");
            uint32_t value = text_color.value.term_color;
            // Background ASCII codes are the same as the foreground ones but with
            // 10 more.
            if (is_background)
                value += 10u;

            size_t index = 0;
            buffer[index++] = static_cast<char>('\x1b');
            buffer[index++] = static_cast<char>('[');

            if (value >= 100u)
            {
                buffer[index++] = static_cast<char>('1');
                value %= 100u;
            }
            buffer[index++] = static_cast<char>('0' + value / 10u);
            buffer[index++] = static_cast<char>('0' + value % 10u);

            buffer[index++] = static_cast<char>('m');
            buffer[index++] = static_cast<char>('\0');
            return;
        }

        for (int i = 0; i < 7; i++)
        {
            buffer[i] = static_cast<char>(esc[i]);
        }
        rgb color(text_color.value.rgb_color);
        to_esc(color.r, buffer + 7, ';');
        to_esc(color.g, buffer + 11, ';');
        to_esc(color.b, buffer + 15, 'm');
        buffer[19] = static_cast<char>(0);
    }

    SPDLOG_CONSTEXPR_FUNC ansi_color_escape(emphasis em) SPDLOG_NOEXCEPT : buffer{}
    {
        uint8_t em_codes[num_emphases] = {};
            if (has_emphasis(em, emphasis::bold)) em_codes[0] = 1;
            if (has_emphasis(em, emphasis::faint)) em_codes[1] = 2;
            if (has_emphasis(em, emphasis::italic)) em_codes[2] = 3;
            if (has_emphasis(em, emphasis::underline)) em_codes[3] = 4;
            if (has_emphasis(em, emphasis::blink)) em_codes[4] = 5;
            if (has_emphasis(em, emphasis::reverse)) em_codes[5] = 7;
            if (has_emphasis(em, emphasis::conceal)) em_codes[6] = 8;
            if (has_emphasis(em, emphasis::strikethrough)) em_codes[7] = 9;

        size_t index = 0;
        for (size_t i = 0; i < num_emphases; ++i)
        {
            if (!em_codes[i])
                continue;
            buffer[index++] = static_cast<char>('\x1b');
            buffer[index++] = static_cast<char>('[');
            buffer[index++] = static_cast<char>('0' + em_codes[i]);
            buffer[index++] = static_cast<char>('m');
        }
        buffer[index++] = static_cast<char>(0);
    }

    SPDLOG_CONSTEXPR_FUNC operator const char *() const SPDLOG_NOEXCEPT
    {
        return buffer;
    }
    SPDLOG_CONSTEXPR_FUNC const char *begin() const SPDLOG_NOEXCEPT
    {
        return buffer;
    }
    SPDLOG_CONSTEXPR_CHAR_TRAITS const char *end() const SPDLOG_NOEXCEPT
    {
        return buffer + std::char_traits<char>::length(buffer);
    }

private:
    static constexpr size_t num_emphases = 8;
    char buffer[7u + 3u * num_emphases + 1u];

    static SPDLOG_CONSTEXPR_FUNC void to_esc(uint8_t c, char *out, char delimiter) SPDLOG_NOEXCEPT
    {
        out[0] = static_cast<char>('0' + c / 100);
        out[1] = static_cast<char>('0' + c / 10 % 10);
        out[2] = static_cast<char>('0' + c % 10);
        out[3] = static_cast<char>(delimiter);
    }
    static SPDLOG_CONSTEXPR_FUNC bool has_emphasis(emphasis em, emphasis mask) SPDLOG_NOEXCEPT
    {
        return static_cast<uint8_t>(em) & static_cast<uint8_t>(mask);
    }
};

// make functions for color and emphasis
SPDLOG_INLINE SPDLOG_CONSTEXPR_FUNC ansi_color_escape make_foreground_color(details::color_t foreground) SPDLOG_NOEXCEPT
{
    return ansi_color_escape(foreground, "\x1b[38;2;");
}

SPDLOG_INLINE SPDLOG_CONSTEXPR_FUNC ansi_color_escape make_background_color(details::color_t background) SPDLOG_NOEXCEPT
{
    return ansi_color_escape(background, "\x1b[48;2;");
}

SPDLOG_INLINE SPDLOG_CONSTEXPR_FUNC ansi_color_escape make_emphasis(emphasis em) SPDLOG_NOEXCEPT
{
    return ansi_color_escape(em);
}

SPDLOG_INLINE SPDLOG_CONSTEXPR_FUNC auto reset_color()
{
    return string_view_t("\x1b[0m");
}

} // namespace details


class style
{
public:
    SPDLOG_CONSTEXPR_FUNC style(emphasis em = emphasis()) SPDLOG_NOEXCEPT
        : set_fg()
        , set_bg()
        , ems(em)
    {}

    SPDLOG_CONSTEXPR_FUNC style &operator|=(const style &rhs)
    {
        if (!set_fg)
        {
            set_fg = rhs.set_fg;
            fg_ = rhs.fg_;
        }
        else if (rhs.set_fg)
        {
            if (!fg_.is_rgb || !rhs.fg_.is_rgb)
                SPDLOG_THROW(spdlog_ex("can't OR a terminal color"));
            fg_.value.rgb_color |= rhs.fg_.value.rgb_color;
        }

        if (!set_bg)
        {
            set_bg = rhs.set_bg;
            bg_ = rhs.bg_;
        }
        else if (rhs.set_bg)
        {
            if (!bg_.is_rgb || !rhs.bg_.is_rgb)
                SPDLOG_THROW(spdlog_ex("can't OR a terminal color"));
            bg_.value.rgb_color |= rhs.bg_.value.rgb_color;
        }

        ems = static_cast<emphasis>(static_cast<uint8_t>(ems) | static_cast<uint8_t>(rhs.ems));
        return *this;
    }

    friend SPDLOG_CONSTEXPR_FUNC style operator|(style lhs, const style &rhs)
    {
        return lhs |= rhs;
    }

    SPDLOG_CONSTEXPR_FUNC bool has_foreground() const SPDLOG_NOEXCEPT
    {
        return set_fg;
    }
    SPDLOG_CONSTEXPR_FUNC bool has_background() const SPDLOG_NOEXCEPT
    {
        return set_bg;
    }
    SPDLOG_CONSTEXPR_FUNC bool has_emphasis() const SPDLOG_NOEXCEPT
    {
        return static_cast<uint8_t>(ems) != 0;
    }
    SPDLOG_CONSTEXPR_FUNC details::color_t get_foreground() const SPDLOG_NOEXCEPT
    {
        SPDLOG_ASSERT(has_foreground(), "no foreground specified for this style");
        return fg_;
    }
    SPDLOG_CONSTEXPR_FUNC details::color_t get_background() const SPDLOG_NOEXCEPT
    {
        SPDLOG_ASSERT(has_background(), "no background specified for this style");
        return bg_;
    }
    SPDLOG_CONSTEXPR_FUNC emphasis get_emphasis() const SPDLOG_NOEXCEPT
    {
        SPDLOG_ASSERT(has_emphasis(), "no emphasis specified for this style");
        return ems;
    }

private:
    SPDLOG_CONSTEXPR_FUNC style(bool is_foreground, details::color_t text_color)SPDLOG_NOEXCEPT
        : set_fg()
        , set_bg()
        , ems()
    {
        if (is_foreground)
        {
            fg_ = text_color;
            set_fg = true;
        }
        else
        {
            bg_ = text_color;
            set_bg = true;
        }
    }

    friend SPDLOG_CONSTEXPR_FUNC style fg(details::color_t foreground) SPDLOG_NOEXCEPT;
    friend SPDLOG_CONSTEXPR_FUNC style bg(details::color_t background) SPDLOG_NOEXCEPT;

    details::color_t fg_;
    details::color_t bg_;
    bool set_fg;
    bool set_bg;
    emphasis ems;
};


/** Creates a text style from the foreground (text) color. */
SPDLOG_INLINE SPDLOG_CONSTEXPR_FUNC style fg(details::color_t foreground) SPDLOG_NOEXCEPT
{
    return style(true, foreground);
}

/** Creates a text style from the background color. */
SPDLOG_INLINE SPDLOG_CONSTEXPR_FUNC style bg(details::color_t background) SPDLOG_NOEXCEPT
{
    return style(false, background);
}

SPDLOG_INLINE SPDLOG_CONSTEXPR_FUNC style operator|(emphasis lhs, emphasis rhs) SPDLOG_NOEXCEPT
{
    return style(lhs) | rhs;
}

namespace details {

std::string parse_style(const style &ts)
{
    std::string style_str;
    if (ts.has_emphasis())
    {
        auto emphasis = details::make_emphasis(ts.get_emphasis());
        style_str.append(emphasis.begin(), emphasis.end());
    }
    if (ts.has_foreground())
    {
        auto foreground = details::make_foreground_color(ts.get_foreground());
        style_str.append(foreground.begin(), foreground.end());
    }
    if (ts.has_background())
    {
        auto background = details::make_background_color(ts.get_background());
        style_str.append(background.begin(), background.end());
    }
    return style_str;
}

} // namespace details

} // namespace spdlog
