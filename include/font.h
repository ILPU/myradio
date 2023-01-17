#ifndef FONT_H
#define FONT_H

#include "common.h"

class font
{
public:
    font(HWND pForm);
    virtual ~font();
    void ApplyFont(HWND apHWND);
    HFONT GetHWNDFont();
    int GetHeightInPixels();

private:
    HFONT hFont;
    int fontHeightInPixels;
};

#endif // FONT_H
