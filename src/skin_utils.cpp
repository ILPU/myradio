#include "skin_utils.h"

COLORREF Color2RGB(COLORREF color)
{
    if(color < 0)
        return GetSysColor(color & 0x7F);
    else
        return color;
}

COLORREF Color2RGBQuad(COLORREF color)
{
    int C = Color2RGB(color);
    C = ((C >> 16) & 0xFF) | ((C << 16) & 0xFF0000) | (C & 0xFF00);
    return C;
}
