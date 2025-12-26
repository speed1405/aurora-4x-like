#include "battle_viewer.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <map>
#include <sstream>

namespace {
struct ViewerState {
    std::string title;
    const std::vector<CombatFrame>* frames{};
    int frameIndex{0};

    std::map<std::pair<ShipClass, COLORREF>, HBITMAP> spriteCache;
};

static HBITMAP createSprite(ShipClass shipClass, COLORREF color) {
    // 32x32 32bpp DIB section.
    constexpr int w = 32;
    constexpr int h = 32;

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP bmp = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bmp || !bits) {
        return nullptr;
    }

    auto* pixels = static_cast<std::uint32_t*>(bits);
    auto pack = [](COLORREF c, std::uint8_t a) {
        const std::uint8_t r = GetRValue(c);
        const std::uint8_t g = GetGValue(c);
        const std::uint8_t b = GetBValue(c);
        return (static_cast<std::uint32_t>(a) << 24) |
               (static_cast<std::uint32_t>(b) << 16) |
               (static_cast<std::uint32_t>(g) << 8) |
               (static_cast<std::uint32_t>(r) << 0);
    };

    const std::uint32_t transparent = 0;
    std::fill(pixels, pixels + (w * h), transparent);

    // Simple “sprite” shapes by class.
    // We draw a filled shape + a white outline.
    auto setPixel = [&](int x, int y, std::uint32_t v) {
        if (x < 0 || y < 0 || x >= w || y >= h) return;
        pixels[y * w + x] = v;
    };

    const std::uint32_t fill = pack(color, 0xFF);
    const std::uint32_t outline = pack(RGB(255, 255, 255), 0xFF);

    auto rect = [&](int x0, int y0, int x1, int y1) {
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                setPixel(x, y, fill);
            }
        }
        for (int x = x0; x <= x1; ++x) {
            setPixel(x, y0, outline);
            setPixel(x, y1, outline);
        }
        for (int y = y0; y <= y1; ++y) {
            setPixel(x0, y, outline);
            setPixel(x1, y, outline);
        }
    };

    auto diamond = [&](int cx, int cy, int r) {
        for (int y = -r; y <= r; ++y) {
            for (int x = -r; x <= r; ++x) {
                if (std::abs(x) + std::abs(y) <= r) {
                    setPixel(cx + x, cy + y, fill);
                }
            }
        }
        // crude outline
        for (int d = 0; d <= r; ++d) {
            setPixel(cx + d, cy + (r - d), outline);
            setPixel(cx - d, cy + (r - d), outline);
            setPixel(cx + d, cy - (r - d), outline);
            setPixel(cx - d, cy - (r - d), outline);
        }
    };

    auto triangle = [&](int x0, int y0, int x1, int y1, int x2, int y2) {
        // Barycentric fill (small size; brute-force ok)
        auto edge = [](int ax, int ay, int bx, int by, int px, int py) {
            return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
        };

        const int minX = std::min({x0, x1, x2});
        const int maxX = std::max({x0, x1, x2});
        const int minY = std::min({y0, y1, y2});
        const int maxY = std::max({y0, y1, y2});

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                const int w0 = edge(x1, y1, x2, y2, x, y);
                const int w1 = edge(x2, y2, x0, y0, x, y);
                const int w2 = edge(x0, y0, x1, y1, x, y);
                if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)) {
                    setPixel(x, y, fill);
                }
            }
        }

        // outline
        auto line = [&](int ax, int ay, int bx, int by) {
            int dx = std::abs(bx - ax), sx = ax < bx ? 1 : -1;
            int dy = -std::abs(by - ay), sy = ay < by ? 1 : -1;
            int err = dx + dy;
            while (true) {
                setPixel(ax, ay, outline);
                if (ax == bx && ay == by) break;
                const int e2 = 2 * err;
                if (e2 >= dy) {
                    err += dy;
                    ax += sx;
                }
                if (e2 <= dx) {
                    err += dx;
                    ay += sy;
                }
            }
        };
        line(x0, y0, x1, y1);
        line(x1, y1, x2, y2);
        line(x2, y2, x0, y0);
    };

    switch (shipClass) {
        case ShipClass::FIGHTER: diamond(16, 16, 8); break;
        case ShipClass::SCOUT: triangle(16, 6, 6, 26, 26, 26); break;
        case ShipClass::CORVETTE: rect(8, 10, 24, 22); break;
        case ShipClass::FRIGATE: rect(6, 9, 26, 23); break;
        case ShipClass::DESTROYER: rect(5, 8, 27, 24); break;
        case ShipClass::CRUISER: rect(4, 7, 28, 25); break;
        case ShipClass::BATTLESHIP: rect(3, 6, 29, 26); break;
        case ShipClass::CARRIER: rect(4, 10, 28, 22); break;
        default: rect(8, 10, 24, 22); break;
    }

    return bmp;
}

static HBITMAP getSprite(ViewerState& state, ShipClass shipClass, COLORREF color) {
    const auto key = std::make_pair(shipClass, color);
    auto it = state.spriteCache.find(key);
    if (it != state.spriteCache.end()) {
        return it->second;
    }
    HBITMAP bmp = createSprite(shipClass, color);
    state.spriteCache.emplace(key, bmp);
    return bmp;
}

static void drawBar(HDC hdc, int x, int y, int w, int h, int value, int maxValue, COLORREF fill) {
    Rectangle(hdc, x, y, x + w, y + h);
    if (maxValue <= 0) return;
    value = std::max(0, std::min(value, maxValue));
    const int filled = (value * w) / maxValue;
    HBRUSH b = CreateSolidBrush(fill);
    RECT r{ x + 1, y + 1, x + filled - 1, y + h - 1 };
    if (filled > 2) FillRect(hdc, &r, b);
    DeleteObject(b);
}

static void paintFrame(HWND hwnd, ViewerState& state, HDC hdc) {
    RECT client{};
    GetClientRect(hwnd, &client);

    HBRUSH bg = CreateSolidBrush(RGB(10, 10, 10));
    FillRect(hdc, &client, bg);
    DeleteObject(bg);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(230, 230, 230));

    if (!state.frames || state.frames->empty()) {
        TextOutA(hdc, 10, 10, "No combat frames", 15);
        return;
    }

    const auto& frame = (*state.frames)[static_cast<size_t>(state.frameIndex)];

    std::ostringstream header;
    header << state.title << "  |  Round " << frame.round << "/" << ((*state.frames).back().round) << "  (Space/Click: next, Esc: close)";
    const std::string headerStr = header.str();
    TextOutA(hdc, 10, 10, headerStr.c_str(), static_cast<int>(headerStr.size()));

    const int midX = (client.right - client.left) / 2;

    // Layout: attacker left, defender right.
    auto drawSide = [&](const std::string& name, const std::vector<CombatShipState>& ships, int baseX, COLORREF color) {
        TextOutA(hdc, baseX, 40, name.c_str(), static_cast<int>(name.size()));

        const int spriteSize = 32;
        const int rowH = 52;
        int y = 70;

        HDC memDC = CreateCompatibleDC(hdc);

        for (size_t i = 0; i < ships.size(); ++i) {
            const auto& s = ships[i];
            HBITMAP bmp = getSprite(state, s.shipClass, color);
            HGDIOBJ old = nullptr;
            if (bmp) {
                old = SelectObject(memDC, bmp);
                BitBlt(hdc, baseX, y, spriteSize, spriteSize, memDC, 0, 0, SRCCOPY);
                SelectObject(memDC, old);
            }

            std::ostringstream line;
            line << s.name << " (" << shipClassToString(s.shipClass) << ")";
            const std::string lineStr = line.str();
            TextOutA(hdc, baseX + 40, y, lineStr.c_str(), static_cast<int>(lineStr.size()));

            // Hull/shields bars
            drawBar(hdc, baseX + 40, y + 18, 160, 10, s.hull, s.maxHull, RGB(80, 200, 80));
            drawBar(hdc, baseX + 40, y + 32, 160, 10, s.shields, s.maxShields, RGB(80, 140, 240));

            y += rowH;
            if (y > client.bottom - 60) break;
        }

        DeleteDC(memDC);
    };

    drawSide(frame.attackerName, frame.attackerShips, 20, RGB(80, 180, 255));
    drawSide(frame.defenderName, frame.defenderShips, midX + 20, RGB(255, 120, 120));
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* state = reinterpret_cast<ViewerState*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (msg) {
        case WM_CREATE: {
            auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
            return 0;
        }
        case WM_PAINT: {
            if (!state) break;
            PAINTSTRUCT ps{};
            HDC hdc = BeginPaint(hwnd, &ps);
            paintFrame(hwnd, *state, hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_KEYDOWN: {
            if (!state) break;
            if (wParam == VK_ESCAPE) {
                DestroyWindow(hwnd);
                return 0;
            }
            if (wParam == VK_SPACE || wParam == VK_RETURN || wParam == VK_RIGHT) {
                state->frameIndex = (state->frameIndex + 1) % static_cast<int>(state->frames->size());
                InvalidateRect(hwnd, nullptr, TRUE);
                return 0;
            }
            if (wParam == VK_LEFT) {
                state->frameIndex = (state->frameIndex - 1);
                if (state->frameIndex < 0) state->frameIndex = static_cast<int>(state->frames->size()) - 1;
                InvalidateRect(hwnd, nullptr, TRUE);
                return 0;
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            if (!state) break;
            state->frameIndex = (state->frameIndex + 1) % static_cast<int>(state->frames->size());
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }
        case WM_DESTROY: {
            if (state) {
                for (auto& kv : state->spriteCache) {
                    if (kv.second) DeleteObject(kv.second);
                }
                state->spriteCache.clear();
            }
            PostQuitMessage(0);
            return 0;
        }
        default:
            break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
} // namespace

void showBattleSprites(const std::string& title, const std::vector<CombatFrame>& frames) {
    if (frames.empty()) return;

    ViewerState state;
    state.title = title;
    state.frames = &frames;
    state.frameIndex = 0;

    const char* klass = "AuroraBattleViewer";

    WNDCLASSA wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = klass;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        klass,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        900, 650,
        nullptr, nullptr,
        wc.hInstance,
        &state);

    if (!hwnd) {
        return;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

#else

void showBattleSprites(const std::string&, const std::vector<CombatFrame>&) {
    // No-op on non-Windows.
}

#endif
