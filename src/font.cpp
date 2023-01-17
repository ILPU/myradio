#include "font.h"

font::font(HWND pForm)
{
    if(pForm)
    {
        NONCLIENTMETRICS nonClientMetrics;
        nonClientMetrics.cbSize = sizeof(nonClientMetrics);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, nonClientMetrics.cbSize, &nonClientMetrics, 0);
        hFont = CreateFontIndirect(&nonClientMetrics.lfMessageFont);

        HDC displayDevice = GetDC(pForm);
        SelectObject(displayDevice, hFont);

        TEXTMETRIC textMetrics;
        GetTextMetrics(displayDevice, &textMetrics);
        fontHeightInPixels = textMetrics.tmHeight;
    }
}

font::~font()
{
    if(hFont)
        DeleteObject(hFont);
}

void font::ApplyFont(HWND apHWND)
{
    if(hFont)
        SendMessage(apHWND, WM_SETFONT, (WPARAM)hFont, 0);
}

HFONT font::GetHWNDFont()
{
    if(hFont)
        return hFont;
    else
        return NULL;
}

int font::GetHeightInPixels()
{
    return fontHeightInPixels;
}
