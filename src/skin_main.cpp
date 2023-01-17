#include "skin_main.h"

bitmap::bitmap(int w, int h)
{
    fhandletype = DDB;
    fScanLineSize = 0;
    fWidth = w;
    fHeight = h;
    fHandle = 0;
    fDIBBits = NULL;
    fDIBHeader = NULL;
    if(w != 0 && h != 0)
    {
        HDC dc = GetDC(0);
        fHandle = CreateCompatibleBitmap(dc, w, h);
        ReleaseDC(0, dc);
    }
}

bitmap::~bitmap()
{
    Clear();
}

void bitmap::Clear()
{
    ClearData();
    fHeight = 0;
    fWidth = 0;
    fDIBAutoFree = false;
}

bool bitmap::Empty()
{
    return (fWidth == 0) || (fHeight == 0);
}

HBITMAP bitmap::GetHandle()
{
    if(Empty())
        return 0;
    if(!fHandle)
    {
        if(fDIBBits)
        {
            void *OldBits;
            OldBits = fDIBBits;
            HDC dc0 = GetDC(0);
            fDIBBits = NULL;
            fHandle = CreateDIBSection(dc0, fDIBHeader, DIB_RGB_COLORS, &fDIBBits, 0, 0);
            ReleaseDC(0, dc0);
            if(fHandle)
            {
                memcpy(fDIBBits, OldBits, fDIBSize);
                if(!fDIBAutoFree)
                    GlobalFree(OldBits);
                fDIBAutoFree = true;
            }
            else
                fDIBBits = OldBits;
        }
    }
    return fHandle;
}

HBITMAP bitmap::ReleaseHandle()
{
    fhandletype = DIB;
    HBITMAP ret = GetHandle();
    if(ret)
    {
        if(fDIBAutoFree)
        {
            void* OldBits = fDIBBits;
            fDIBBits = reinterpret_cast<void*>(GlobalAlloc(GMEM_FIXED, fDIBSize));
            memcpy(fDIBBits, OldBits, fDIBSize);
            fDIBAutoFree = false;
        }
    }
    fHandle = 0;
    return ret;
}

void bitmap::Dormant()
{
    if(fHandle)
        DeleteObject(ReleaseHandle());
}

void bitmap::draw(HDC DC, int x, int y)
{
TRYAgain:
    if(Empty())
        return;
    if(fHandle)
    {
        int oldHeight = fHeight;
        //tagBITMAP B;
        HDC DCfrom, DC0;
        //if(GetObject(fHandle, sizeof(B), &B))
        //    oldHeight = B.bmHeight;
        DC0    = GetDC(0);
        DCfrom = CreateCompatibleDC(DC0);
        ReleaseDC(0, DC0);

        HBITMAP oldBmp = (HBITMAP)SelectObject(DCfrom, fHandle);
        BitBlt(DC, x, y, fWidth, oldHeight, DCfrom, 0, 0, SRCCOPY);
        SelectObject(DCfrom, oldBmp);
        DeleteDC(DCfrom);
    }
    else if(fDIBBits)
    {
        int oldHeight = abs(fDIBHeader->bmiHeader.biHeight);
        if(StretchDIBits(DC, x, y, fWidth, oldHeight, 0, 0, fWidth, oldHeight,
                         fDIBBits, fDIBHeader, DIB_RGB_COLORS, SRCCOPY) == 0)
        {
            if(GetHandle())
                goto TRYAgain;
        }
    }
}

void* bitmap::GetScanLine(int y)
{
    if(!fDIBHeader)
        return NULL;
    if(fDIBHeader->bmiHeader.biHeight > 0)
        y = fHeight - 1 - y;
    if(!fScanLineSize)
        GetScanLineSize();
    return ((char*)(fDIBBits) + fScanLineSize * y);
}

COLORREF bitmap::GetDIBPixels(int x, int y)
{
    if(fhandletype == DIB)
    {
        PBYTE fScanLine0    = (PBYTE)GetScanLine(0);
        int fScanLineDelta  = (DWORD)GetScanLine(1) - (DWORD)fScanLine0;
        int /*fPixelsPerByteMask,*/ fBytesPerPixel;
        unsigned long Pixel;
        if(fNewPixelFormat == 24)
        {
            //fPixelsPerByteMask = 0;
            fBytesPerPixel = 3;
            Pixel = *PDWORD(PBYTE(fScanLine0) + y * fScanLineDelta + x * fBytesPerPixel) & 0xFFFFFF;
            return Color2RGBQuad((COLORREF)Pixel);
        }
    }
    else
        return 0;
}

RECT bitmap::GetBoundsRect() const
{
    RECT rec = {0, 0, fWidth, fHeight};
    return rec;
}

HRGN bitmap::ToRegion(COLORREF cTransparentColor, COLORREF cTolerance)
{
    HRGN hRgn = NULL;

    if(GetHandle() == 0)
        return hRgn;

    HDC hMemDC = CreateCompatibleDC(NULL);
    if (hMemDC)
    {
        BITMAPINFOHEADER RGB32BITSBITMAPINFO =
        {
            sizeof(BITMAPINFOHEADER),    // biSize
            fWidth,                    // biWidth;
            fHeight,                // biHeight;
            1,                            // biPlanes;
            32,                            // biBitCount
            BI_RGB,                        // biCompression;
            0,                            // biSizeImage;
            0,                            // biXPelsPerMeter;
            0,                            // biYPelsPerMeter;
            0,                            // biClrUsed;
            0                            // biClrImportant;
        };
        VOID * pbits32;
        HBITMAP hbm32 = CreateDIBSection(hMemDC, (BITMAPINFO *)&RGB32BITSBITMAPINFO, DIB_RGB_COLORS, &pbits32, NULL, 0);
        if (hbm32)
        {
            HBITMAP holdBmp = (HBITMAP)SelectObject(hMemDC, hbm32);
            HDC hDC = CreateCompatibleDC(hMemDC);
            if (hDC)
            {
                BITMAP bm32;
                GetObject(hbm32, sizeof(bm32), &bm32);
                while (bm32.bmWidthBytes % 4)
                    bm32.bmWidthBytes++;

                HBITMAP holdBmp = (HBITMAP)SelectObject(hDC, fHandle);
                BitBlt(hMemDC, 0, 0, fWidth, fHeight, hDC, 0, 0, SRCCOPY);

                DWORD maxRects = 100;
                HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects));
                RGNDATA *pData = (RGNDATA *)GlobalLock(hData);
                pData->rdh.dwSize = sizeof(RGNDATAHEADER);
                pData->rdh.iType = RDH_RECTANGLES;
                pData->rdh.nCount = pData->rdh.nRgnSize = 0;
                SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);

                // Keep on hand highest and lowest values for the "transparent" pixels
                BYTE lr = GetRValue(cTransparentColor);
                BYTE lg = GetGValue(cTransparentColor);
                BYTE lb = GetBValue(cTransparentColor);
                BYTE hr = Min(0xff, lr + GetRValue(cTolerance));
                BYTE hg = Min(0xff, lg + GetGValue(cTolerance));
                BYTE hb = Min(0xff, lb + GetBValue(cTolerance));

                // Scan each bitmap row from bottom to top (the bitmap is inverted vertically)
                BYTE *p32 = (BYTE *)bm32.bmBits + (bm32.bmHeight - 1) * bm32.bmWidthBytes;
                for (int y = 0; y < fHeight; y++)
                {
                    // Scan each bitmap pixel from left to right
                    for (int x = 0; x < fWidth; x++)
                    {
                        // Search for a continuous range of "non transparent pixels"
                        int x0 = x;
                        LONG *p = (LONG *)p32 + x;
                        while (x < fWidth)
                        {
                            BYTE b = GetRValue(*p);
                            if (b >= lr && b <= hr)
                            {
                                b = GetGValue(*p);
                                if (b >= lg && b <= hg)
                                {
                                    b = GetBValue(*p);
                                    if (b >= lb && b <= hb)
                                        // This pixel is "transparent"
                                        break;
                                }
                            }
                            p++;
                            x++;
                        }

                        if (x > x0)
                        {
                            // Add the pixels (x0, y) to (x, y+1) as a new rectangle in the region
                            if (pData->rdh.nCount >= maxRects)
                            {
                                GlobalUnlock(hData);
                                maxRects += 100;
                                hData = GlobalReAlloc(hData, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), GMEM_MOVEABLE);
                                pData = (RGNDATA *)GlobalLock(hData);
                            }
                            RECT *pr = (RECT *)&pData->Buffer;
                            SetRect(&pr[pData->rdh.nCount], x0, y, x, y+1);
                            if (x0 < pData->rdh.rcBound.left)
                                pData->rdh.rcBound.left = x0;
                            if (y < pData->rdh.rcBound.top)
                                pData->rdh.rcBound.top = y;
                            if (x > pData->rdh.rcBound.right)
                                pData->rdh.rcBound.right = x;
                            if (y+1 > pData->rdh.rcBound.bottom)
                                pData->rdh.rcBound.bottom = y+1;
                            pData->rdh.nCount++;
                        }
                    }
                    p32 -= bm32.bmWidthBytes;
                }
                hRgn = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);

                GlobalFree(hData);
                SelectObject(hDC, holdBmp);
                DeleteDC(hDC);
            }
            DeleteObject(SelectObject(hMemDC, holdBmp));
        }
        DeleteDC(hMemDC);
    }

    return hRgn;
}

void bitmap::CopyRect(const RECT dstrect, bitmap *srcbmp, const RECT srcrect)
{
    if(GetHandle() == 0 || srcbmp->GetHandle() == 0)
        return;
    HDC DCsrc, DCdst;
    HANDLE SaveSrc, SaveDst;

    DCsrc = CreateCompatibleDC(0);
    SaveSrc = SelectObject(DCsrc, srcbmp->fHandle);
    DCdst = DCsrc;
    SaveDst = 0;

    if(srcbmp != this)
    {
        DCdst = CreateCompatibleDC(0);
        SaveDst = SelectObject(DCdst, fHandle);
    }

    StretchBlt(DCdst, dstrect.left, dstrect.top, dstrect.right - dstrect.left,
               dstrect.bottom - dstrect.top, DCsrc, srcrect.left, srcrect.top,
               srcrect.right - srcrect.left, srcrect.bottom - srcrect.top,
               SRCCOPY);

    if(srcbmp != this)
    {
        SelectObject(DCdst, SaveDst);
        DeleteDC(DCdst);
    }

    SelectObject(DCsrc, SaveSrc);
    DeleteDC(DCsrc);
}

bool bitmap::LoadFromStream(myfile *astream)
{
    if(astream)
    {
        bool ret;
        unsigned long pos;
        pos = astream->curposition();
        if(!read_bitmap(astream, pos))
        {
            astream->seek(pos, FILE_BEGIN);
            ret = false;
        }
        else
            ret = true;
        return ret;
    }
    else
        return false;
}

bool bitmap::LoadFromFile(mystring *afile)
{
    if(file.open(afile, true, true))
    {
        bool ret;
        unsigned long pos;
        Clear();
        pos = file.curposition();
        if(!read_bitmap(&file, pos))
        {
            file.seek(pos, FILE_BEGIN);
            Clear();
            file.close();
            ret = false;
        }
        else
            ret = true;
        file.close();
        return ret;
    }
    else
        return false;
}

bitmap_type bitmap::GetType()
{
    return fhandletype;
}

void bitmap::ClearData()
{
    if(fHandle)
    {
        DeleteObject(fHandle);
        fHandle = 0;
        fDIBBits = NULL;
    }
    if(fDIBBits)
    {
        if(!fDIBAutoFree)
            GlobalFree(fDIBBits);
        fDIBBits = NULL;
    }
    if(fDIBHeader)
    {
        free(fDIBHeader);
        fDIBHeader = NULL;
    }
    fScanLineSize = 0;
}

int bitmap::CalcScanLineSize(BITMAPINFOHEADER *header)
{
    return ((header->biBitCount * header->biWidth + 31) >> 3)  & 0xFFFFFFFC;
}

int bitmap::GetScanLineSize()
{
    if(!fDIBHeader)
        return 0;
    fScanLineSize = CalcScanLineSize(&fDIBHeader->bmiHeader);
    return fScanLineSize;
}

bool bitmap::read_bitmap(myfile *afile, unsigned long apos)
{
    BITMAPFILEHEADER BFH;
    if(afile->read((unsigned char*)&BFH, sizeof(BITMAPFILEHEADER)) == sizeof(BITMAPFILEHEADER))
    {
        fhandletype = DIB;
        unsigned long off = 0;
        int size_, size1;
        if(BFH.bfType != 0x4D42)
        {
            afile->seek(apos, FILE_BEGIN);
        }
        else
        {
            off     = BFH.bfOffBits - sizeof(BFH);
            size_   = BFH.bfSize;
        }
        //unsigned long RGBSize = 4;
        unsigned long HdSz = sizeof(BITMAPINFOHEADER);

        fDIBHeader = static_cast<BITMAPINFO*>(malloc(256 * sizeof(RGBQUAD) + HdSz));
        if(afile->read((unsigned char*)&fDIBHeader->bmiHeader.biSize, sizeof(unsigned long)) != sizeof(unsigned long))
            return false;

        if(fDIBHeader->bmiHeader.biSize == HdSz)
        {
            if(afile->read((unsigned char*)&fDIBHeader->bmiHeader.biWidth, HdSz - sizeof(unsigned long)) != HdSz - sizeof(unsigned long))
                return false;

        }
        else if(fDIBHeader->bmiHeader.biSize == sizeof(BITMAPCOREHEADER))
        {
            //RGBSize = 3;
            HdSz = sizeof(BITMAPCOREHEADER);
            BITMAPCOREHEADER BCH;
            if(afile->read((unsigned char*)&BCH.bcWidth, HdSz - sizeof(unsigned long)) != HdSz - sizeof(unsigned long))
                return false;

            fDIBHeader->bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
            fDIBHeader->bmiHeader.biWidth    = BCH.bcWidth;
            fDIBHeader->bmiHeader.biHeight   = BCH.bcHeight;
            fDIBHeader->bmiHeader.biPlanes   = BCH.bcPlanes;
            fDIBHeader->bmiHeader.biBitCount = BCH.bcBitCount;
        }
        else
        {
            return false;
        }

        fNewPixelFormat = fDIBHeader->bmiHeader.biBitCount * fDIBHeader->bmiHeader.biPlanes;

        if(fDIBHeader->bmiHeader.biCompression != BI_RGB)
            return false;

        fWidth  = fDIBHeader->bmiHeader.biWidth;
        fHeight = abs(fDIBHeader->bmiHeader.biHeight);
        fDIBSize = GetScanLineSize() * fHeight;
        fDIBBits = reinterpret_cast<void*>(GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, fDIBSize));
        /*unsigned long ColorCount = 0;
        if(fDIBHeader->bmiHeader.biBitCount <= 8)
        {
            if(fDIBHeader->bmiHeader.biClrUsed > 0)
                ColorCount = fDIBHeader->bmiHeader.biClrUsed * sizeof(RGBQUAD);
            else
                ColorCount = (1 << fDIBHeader->bmiHeader.biBitCount) * sizeof(RGBQUAD);
        }
        else if((fNewPixelFormat == 16) || (fDIBHeader->bmiHeader.biCompression == BI_BITFIELDS))
            ColorCount = 12;

        if(off > 0)
        {
            off -= HdSz;
            if(off != ColorCount)
                if((fNewPixelFormat != 15 || fNewPixelFormat != 16) || (off == 0))
                    ColorCount = Min((unsigned long)1024, off);
        }
        if(ColorCount != 0)
        {
            if(off >= ColorCount)
                off -= ColorCount;

            if(RGBSize == 4)
            {
                if(afile->read((unsigned char*)&fDIBHeader->bmiColors[1], ColorCount) != ColorCount)
                    return false;
            }
            else
            {
                RGBQUAD *c;
                c = &fDIBHeader->bmiColors[1];
                while(ColorCount > 0)
                {
                    if(afile->read((unsigned char*)c, RGBSize) != RGBSize)
                        return false;

                    ColorCount -= RGBSize;
                    c++;
                }
            }
        }*/

        if(off > 0)
            afile->seek(off, FILE_CURRENT);
        if(size_ == 0 || afile->getsize() <= 0)
            size_ = fDIBSize;
        else
            size_ = Min(fDIBSize, (int)(afile->getsize() - afile->curposition()));

        size1 = Min(size_, fDIBSize);
        if((size1 < fDIBSize) && ((fDIBSize - size1) <= (int)afile->curposition()))
        {
            afile->seek(size1 - fDIBSize, FILE_CURRENT);
            size1 = fDIBSize;
        }
        if(size1 > fDIBSize)
            size1 = fDIBSize;
        if((int)afile->read((unsigned char*)fDIBBits, size1) != size1)
            return false;

        if(size_ > size1)
            afile->seek(size_ - size1, FILE_CURRENT);
        return true;
    }
    else
    {
        return false;
    }
}

bool control::touche(unsigned short px, unsigned short py)
{
    calc_pos();
    if(region)
    {
        return PtInRegion(region, px, py);
    }
    else
    {
        return (px >= x) && (px < x + w) && (py >= y) && (py < y + h);
    }
}

RECT control::getboundsrect() const
{
    RECT rec = {x, y, x+w, y+h};
    return rec;
}

void control::set_dc(HDC pdc_memory, HDC pdc_main)
{
    dc_main     = pdc_main;
    dc_memory   = pdc_memory;
}

void control::_setparent(control *p_parent)
{
    if(p_parent)
    {
        parent = p_parent;
    }
    else
    {
        parent = NULL;
    }
}

bool control::window_bmp_blt(HBITMAP SrcBmp, WORD hdc_x, WORD hdc_y, WORD hdc_w, WORD hdc_h, WORD srcx, WORD srcy)
{
    {
        HBITMAP h = (HBITMAP)SelectObject(dc_memory, SrcBmp);
        int retval = BitBlt(dc_main, hdc_x, hdc_y, hdc_w, hdc_h,
                            dc_memory, srcx, srcy,
                            SRCCOPY);
        SelectObject(dc_memory, h);
        return retval ? true : false;
    }
    return false;
}

bool control::window_bmp_blt_trans(HBITMAP SrcBmp, HBITMAP TrBmp, WORD hdc_x, WORD hdc_y, WORD hdc_w, WORD hdc_h, WORD srcx, WORD srcy)
{
    {
        HBITMAP h;
        int retval;
        h = (HBITMAP)SelectObject(dc_memory, TrBmp);
        retval = BitBlt(dc_main, hdc_x, hdc_y, hdc_w, hdc_h,
                        dc_memory, srcx, srcy,
                        SRCAND);
        SelectObject(dc_memory, h);

        h = (HBITMAP)SelectObject(dc_memory, SrcBmp);
        retval = BitBlt(dc_main, hdc_x, hdc_y, hdc_w, hdc_h,
                        dc_memory, srcx, srcy,
                        SRCPAINT);
        SelectObject(dc_memory, h);

        return retval ? true : false;
    }
    return false;
}

HBITMAP control::create_bitmap_mask(HBITMAP pSrcBitmap)
{
    HDC hdcMem, hdcMem2;
    HBITMAP hbmMask;

    hbmMask = CreateBitmap(w, h, 1, 1, NULL);

    hdcMem = CreateCompatibleDC(0);
    hdcMem2 = CreateCompatibleDC(0);

    SelectObject(hdcMem, pSrcBitmap);
    SelectObject(hdcMem2, hbmMask);

    SetBkColor(hdcMem, color_t);

    BitBlt(hdcMem2, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);
    BitBlt(hdcMem, 0, 0, w, h, hdcMem2, 0, 0, SRCINVERT);

    DeleteDC(hdcMem);
    DeleteDC(hdcMem2);
    return hbmMask;
}

playlist::playlist(int _x, int _y, control_type pid) : control()
{
    id = pid;
    sx = _x;
    sy = _y;
}

playlist::~playlist()
{
//
}

bool playlist::mouse_down(WORD px, WORD py)
{
    return false;
}

bool playlist::mouse_up(WORD px, WORD py, bool pforce)
{
    return false;
}

bool playlist::mouse_move(bool prelease, WORD px, WORD py)
{
    return false;
}

bool playlist::mouse_unover(bool pforce)
{
    return false;
}

void playlist::calc_pos()
{

}

void playlist::draw_state(PAINTSTRUCT *pps)
{
    //
}

void playlist::set_parent(control *p_parent)
{
    _setparent(p_parent);
}

panel::panel(int _leftw, int _rightw, int _tiledx, int _tiledw, COLORREF ptrans_color, control_type pid) : control()
{
    id = pid;
    x = 0;
    y = 0;

    panel_rect.left_width     = _leftw;
    panel_rect.right_width    = _rightw;
    panel_rect.tiled_x        = _tiledx;
    panel_rect.tiled_width    = _tiledw;

    is_panel = true;

    use_region = false;
    color_t = ptrans_color;

    if(use_region && !color_t)
        use_region = false;
}

panel::~panel()
{
//
}

bool panel::mouse_down(WORD px, WORD py)
{
    return false;
}

bool panel::mouse_up(WORD px, WORD py, bool pforce)
{
    return false;
}

bool panel::mouse_move(bool prelease, WORD px, WORD py)
{
    return false;
}

bool panel::mouse_unover(bool pforce)
{
    return false;
}

void panel::set_parent(control *p_parent)
{
    _setparent(p_parent);
}

bool panel::loadbitmap_stream(myfile *pfile)
{
    bmp_normal = new bitmap(0, 0);
    if(bmp_normal->LoadFromStream(pfile))
    {
        w = bmp_normal->fWidth;
        h = bmp_normal->fHeight;
        return true;
    }
    else
        return false;
}

void panel::calc_pos()
{

}

void panel::draw_state(PAINTSTRUCT *pps)
{
    if(bmp_normal && bmp_normal->GetHandle())
    {
        HBITMAP oldbmp = (HBITMAP)SelectObject(dc_memory, bmp_normal->GetHandle());

        BitBlt(dc_main, 0, 0, panel_rect.left_width, h, dc_memory, 0, 0, SRCCOPY);
        BitBlt(dc_main, pps->rcPaint.right - panel_rect.right_width, 0, panel_rect.right_width, h, dc_memory, w - panel_rect.right_width, 0, SRCCOPY);

        //window_bmp_blt(bmp_normal->GetHandle(), 0, 0, panel_rect.left_width, h, 0, 0);
        //window_bmp_blt(bmp_normal->GetHandle(), pfs->right - panel_rect.right_width, 0, panel_rect.right_width, h, w - panel_rect.right_width, 0);

        int iSourceCursorPos = panel_rect.tiled_x;
        int iEndCol = pps->rcPaint.right - panel_rect.right_width;
        while(iSourceCursorPos < iEndCol)
        {
            BitBlt(dc_main, iSourceCursorPos, 0, panel_rect.tiled_width, h, dc_memory, panel_rect.tiled_x, 0, SRCCOPY);
            //window_bmp_blt(bmp_normal->GetHandle(), iSourceCursorPos, 0, panel_rect.tiled_width, h, panel_rect.tiled_x, 0);
            iSourceCursorPos += panel_rect.tiled_width;
        }

        SelectObject(dc_memory, oldbmp);
    }
}

bool panel::loadbitmap(mystring *pfilename)
{
    bmp_normal = new bitmap(0, 0);
    if(bmp_normal->LoadFromFile(pfilename))
    {
        w = bmp_normal->fWidth;
        h = bmp_normal->fHeight;
        return true;
    }
    else
        return false;
}

button::button(int _x, int _y, bool _useregion, int _alignH, int _alighV, COLORREF ptrans_color, control_type pid) : control()
{
    id = pid;
    sx = _x;
    sy = _y;

    use_region = _useregion;

    if(_alignH == 1)
        align_horz = align_right;
    if(_alighV == 1)
        align_vert = align_bottom;

    color_t = ptrans_color;

    if(id == btn_playlist || id == info_btn_info)
        use_3state = true;
    else
        use_3state = false;

    if(use_region && !color_t)
        use_region = false;

}

button::~button()
{

}

void button::calc_pos()
{
    if(parent_rect)
        if(parent_new_size)
        {
            if(align_horz == align_left)
                x = sx;
            else if(align_horz == align_right)
                x = ((parent_rect->right - parent_rect->left) - sx) - w;

            if(align_vert == align_top)
                y = sy;
            else if(align_vert == align_bottom)
                y = ((parent_rect->bottom - parent_rect->top) - sx) - h;

            parent_new_size = false;
        }
}

bool button::mouse_down(WORD px, WORD py)
{
    if(!is_clicked)
    {
        window_bmp_blt(bmp_push->GetHandle(), x, y, w, h, 0, 0);
        is_clicked = true;
        return true;
    }
    return false;
}

bool button::mouse_up(WORD px, WORD py, bool pforce)
{
    if(is_clicked || pforce)
    {
        window_bmp_blt(bmp_over->GetHandle(), x, y, w, h, 0, 0);
        is_clicked = false;
        return true;
    }
    return false;
}

bool button::mouse_move(bool prelease, WORD px, WORD py)
{
    if(prelease)
    {
        if(!is_clicked)
        {
            window_bmp_blt(bmp_push->GetHandle(), x, y, w, h, 0, 0);
            is_clicked = true;
            return true;
        }
        return false;
    }
    else if(!is_over)
    {
        window_bmp_blt(bmp_over->GetHandle(), x, y, w, h, 0, 0);
        is_over = true;
        return true;
    }
    return false;
}

bool button::mouse_unover(bool pforce)
{
    is_clicked = false;
    if(is_over || pforce)
    {
        window_bmp_blt(bmp_normal->GetHandle(), x, y, w, h, 0, 0);
        is_over = false;
        return true;
    }
    return false;
}

void button::draw_state(PAINTSTRUCT *pps)
{

}

void button::set_parent(control *p_parent)
{
    _setparent(p_parent);
}

void button::copymainbmp(bitmap *pbmp)
{
    h = pbmp->fHeight;
    w = pbmp->fWidth / 3;

    bmp_normal = new bitmap(w, h);
    bmp_normal->CopyRect(bmp_normal->GetBoundsRect(), pbmp, bmp_normal->GetBoundsRect());

    bmp_over = new bitmap(w, h);
    RECT rec = bmp_normal->GetBoundsRect();
    rec.left = w;
    rec.right = w*2;
    bmp_over->CopyRect(bmp_normal->GetBoundsRect(), pbmp, rec);

    bmp_push = new bitmap(w, h);
    rec = bmp_over->GetBoundsRect();
    rec.left = w*2;
    rec.right = w*3;
    bmp_push->CopyRect(bmp_over->GetBoundsRect(), pbmp, rec);

    calc_pos(); /// !!!!!
}

bool button::loadbitmap_stream(myfile *pfile)
{
    bool ret;
    bitmap bmp1(0, 0);
    if(ret = bmp1.LoadFromStream(pfile))
        copymainbmp(&bmp1);
    return ret;
}

bool button::loadbitmap(mystring *pfilename)
{
    bool ret;
    bitmap bmp1(0, 0);
    if(ret = bmp1.LoadFromFile(pfilename))
        copymainbmp(&bmp1);
    return ret;
}

scroll::scroll(int _x, int _y, bool _ishoriz, bool _useslider, bool _useregion, int _alignH, int _alighV, COLORREF ptrans_color, control_type pid)
{
    id = pid;
    sx = _x;
    sy = _y;

    type = _ishoriz ? horizontal : vertical;
    use_region = _useregion;
    use_slider = _useslider;

    if(_alignH == 1)
        align_horz = align_right;
    if(_alighV == 1)
        align_vert = align_bottom;

    move_mouse = false;
    is_scroll = true;

    fmax = 100;
    fmin = 0;
    fposition = -1;

    color_t = ptrans_color;

    if(use_region && !color_t)
        use_region = false;

    bmp_slider_tr = 0;
    bmp_slider = NULL;
}

scroll::~scroll()
{
    if(bmp_slider_tr)
    {
        DeleteObject(bmp_slider_tr);
        bmp_slider_tr = 0;
    }
    if(bmp_slider)
    {
        delete bmp_slider;
        bmp_slider = NULL;
    }
}

bool scroll::loadbitmap(mystring *pfilename)
{
    bool ret;
    bitmap bmp1(0, 0);
    if(ret = bmp1.LoadFromFile(pfilename))
        copymainbmp(&bmp1);
    return ret;
}

bool scroll::loadbitmap_stream(myfile *pfile)
{
    bool ret;
    bitmap bmp1(0, 0);
    if(ret = bmp1.LoadFromStream(pfile))
        copymainbmp(&bmp1);
    return ret;
}

void scroll::calc_pos()
{
    if(parent_rect)
        if(parent_new_size)
        {
            if(align_horz == align_left)
                x = sx;
            else if(align_horz == align_right)
                x = ((parent_rect->right - parent_rect->left) - sx) - w;

            if(align_vert == align_top)
                y = sy;
            else if(align_vert == align_bottom)
                y = ((parent_rect->bottom - parent_rect->top) - sx) - h;

            parent_new_size = false;
        }
}

bool scroll::mouse_down(WORD px, WORD py)
{
    if(!is_clicked)
    {
        window_bmp_blt(bmp_push->GetHandle(), x, y, w, h, 0, 0);
        set_position((int)((px - x) * (fmax / (float)w)));
        is_clicked = true;
        move_mouse = true;
        return true;
    }
    return false;
}

bool scroll::mouse_up(WORD px, WORD py, bool pforce)
{
    if(is_clicked || pforce)
    {
        window_bmp_blt(bmp_over->GetHandle(), x, y, w, h, 0, 0);
        draw_from_position();
        is_clicked = false;
        move_mouse = false;
        return true;
    }
    return false;
}

bool scroll::mouse_move(bool prelease, WORD px, WORD py)
{
    if(prelease)
    {
        if(is_clicked)
        {
            window_bmp_blt(bmp_push->GetHandle(), x, y, w, h, 0, 0);
            set_position((int)((px - x) * (fmax / (float)w)));
            return true;
        }
        return false;
    }
    else
    {
        window_bmp_blt(bmp_over->GetHandle(), x, y, w, h, 0, 0);
        draw_from_position();
        is_over = true;
        //window_bmp_blt(bmp_over->GetHandle(), x, y, w, h, 0, 0);
        //set_position((int)((px - x) * (fmax / (float)w)));

        return true;
    }
    return false;

    /*if(is_clicked)
    {
        window_bmp_blt(bmp_over->GetHandle(), x, y, w, h, 0, 0);
        set_position((int)((px - x) * (fmax / (float)w)));

        return true;
    }
    return false;*/
}

bool scroll::mouse_unover(bool pforce)
{
    if(!move_mouse)
    {
        is_clicked = false;
        if(is_over || pforce)
        {
            window_bmp_blt(bmp_normal->GetHandle(), x, y, w, h, 0, 0);
            draw_from_position();
            is_over = false;
            return true;
        }
    }
    return false;
}

void scroll::set_parent(control *p_parent)
{
//
}

void scroll::copymainbmp(bitmap *pbmp)
{
    if(!use_slider)
    {
        h = pbmp->fHeight;
        w = pbmp->fWidth / 4;

        calc_pos();

        bmp_normal = new bitmap(w, h);
        bmp_normal->CopyRect(bmp_normal->GetBoundsRect(), pbmp, bmp_normal->GetBoundsRect());

        RECT rec = bmp_normal->GetBoundsRect();
        rec.left = w;
        rec.right = w*2;

        bmp_over = new bitmap(w, h);
        bmp_over->CopyRect(bmp_over->GetBoundsRect(), pbmp, rec);

        bmp_push = new bitmap(w, h);
        rec = bmp_over->GetBoundsRect();
        rec.left = w*2;
        rec.right = w*3;
        bmp_push->CopyRect(bmp_push->GetBoundsRect(), pbmp, rec);

        bmp_slider = new bitmap(w, h);
        rec = bmp_slider->GetBoundsRect();
        rec.left = w*3;
        rec.right = w*4;
        bmp_slider->CopyRect(bmp_slider->GetBoundsRect(), pbmp, rec);

        if(color_t != 0)
        {
            bmp_slider_tr = create_bitmap_mask(bmp_slider->GetHandle());
        }

        set_position(0);
    }
}

int scroll::get_position()
{
    return fposition;
}

void scroll::set_position(int pnewpos)
{
    if(fposition != pnewpos)
    {
        fposition = pnewpos;
        if(fposition > fmax)
            fposition = fmax;
        if(fposition < fmin)
            fposition = fmin;

        if(type == vertical)
        {
            positionwaarde = (int)((float)y + h -
                                   (((float)fposition / (float)fmax) * (float)h));
        }
        else if(type == horizontal)
        {
            positionwaarde = (int)((float)x + (((float)fposition / (float)fmax) * (float)w));
        }
        draw_from_position();
    }
}

void scroll::draw_state(PAINTSTRUCT *pps)
{
    draw_from_position();
}

void scroll::draw_from_position()
{
    if(type == vertical)
    {
        window_bmp_blt_trans(bmp_slider->GetHandle(), bmp_slider_tr, x, positionwaarde, w, h, 0, 0);
    }
    else if(type == horizontal)
    {
        window_bmp_blt_trans(bmp_slider->GetHandle(), bmp_slider_tr, x, y, positionwaarde - x, h, 0, 0);
    }
}

skin_engine::skin_engine(HWND pform, HDC pdc_main, HDC pdc_memory, HINSTANCE pInst, mystring *adir)
{
    main_cur_control = none;
    info_cur_control = none;

    skintype = _notload;

    win_panel_skin = false;

    infoform = NULL;

    skin_load = false;
    show_info_panel = false;
    InitializeCriticalSection(&cs_skin_load_ok);

    min_pl_w = min_pl_h = 0;

    pHeight = 0;
    pWidth = 0;
    skinpath = *adir + _T("\\skin");
    mainform = pform;
    maininst = pInst;

    GetClientRect(mainform, &rect_form_main);

    dc_memory   = pdc_memory;
    dc_main     = pdc_main;

    release_capture = false;
    main_mouse_leave = false;
    info_mouse_leave = false;

    bmp_main            = NULL;
}

skin_engine::~skin_engine()
{
    /*for(std::map<control_type, control*>::iterator it = controls_info.begin() ; it != controls_info.end(); ++it)
    {
        delete it->second;
        it->second = NULL;
    }

    for(std::map<control_type, control*>::iterator it = controls_main.begin() ; it != controls_main.end(); ++it)
    {
        delete it->second;
        it->second = NULL;
    }*/

    for(std::map<control_type, control*>::iterator it = controls.begin() ; it != controls.end(); ++it)
    {
        delete it->second;
        it->second = NULL;
    }


    if(bmp_main)
    {
        delete bmp_main;
        bmp_main = NULL;
    }

    /*if(bmp_dubbuf)
    {
        delete bmp_dubbuf;
        bmp_dubbuf = NULL;
    }*/

    main_cur_control = none;
    info_cur_control = none;

    DeleteCriticalSection(&cs_skin_load_ok);
}

skin_engine::_ControlsName skin_engine::ControlsName =
{
    {"button_play", btn_play},
    {"button_pause", btn_pause},
    {"button_stop", btn_stop},
    {"button_prev", btn_prev},
    {"button_next", btn_next},
    {"button_info", btn_info},
    {"button_conf", btn_config},
    {"button_min", btn_min},
    {"button_exit", btn_exit},
    {"button_info_exit", btn_info_exit},
    {"scroll_volume", sc_vol},
    {"playlist", cnt_playlist},
    {"button_add", btn_add},
    {"button_del", btn_del},
    {"panel_top", panel_pl_top},
};

bool skin_engine::get_config_from_skin(tinyxml2::XMLDocument *pxml)
{
    XMLElement* element = pxml->RootElement();
    if(strcmp(element->Name(), "ver") == 0)
    {
        skin_ver = element->IntText(0);
        if(skin_ver == 0) // not skin ver in file
        {
            return false;
        }
        else if(skin_ver != SKIN_VER) // skin ver in file not support
        {
            return false;
        }
        else if(skin_ver == SKIN_VER)
        {
            auto_mini   = false;
            skintype    = pxml->FirstChildElement("set")->FirstChildElement("switch")->IntText(0) ? _switch : _normal;
            bool use_color;
            skin_transcolor = SkinDecodeColor(pxml->FirstChildElement("color")->FirstChildElement("transparent")->GetText(), &use_color);

            skin_color_set.pl_back = SkinDecodeColor(pxml->FirstChildElement("color")->FirstChildElement("playlist_background")->GetText(), &use_color);
            skin_color_set.use_pl_back = use_color;
            return true;
        }
    }
    else
        return false;
}

bool skin_engine::load_skinfile(wchar_t *pfilename)
{
    EnterCriticalSection(&cs_skin_load_ok);
    bool ret = false;
    skin_file sfile;
    mystring sf = skinpath;
    sf.str.append(_T("\\"));
    sf.str.append(pfilename);
    if(sfile.OpenFile(&sf))
    {
        myfile mfile;
        if(sfile.GetSubFile("config.xml", &mfile))
        {
            tinyxml2::XMLDocument doc;
            if(createconfig_list(&mfile, &doc))
            {
                mfile.close();
                if(get_config_from_skin(&doc))
                {
                    if(sfile.GetSubFile("form_main.bmp", &mfile))
                    {
                        bmp_main = new bitmap(0, 0);
                        if(bmp_main->LoadFromStream(&mfile))
                        {
                            pWidth  = bmp_main->fWidth;
                            pHeight = bmp_main->fHeight;

                            sendappmes(SKIN_NEW_FORM_SIZE, 0);
                            ret = createcontrols(&sfile, &doc);
                            if(ret)
                                skin_load = true;
                        }
                    }
                }
            }
        }
    }

    LeaveCriticalSection(&cs_skin_load_ok);
    return ret;
}

void skin_engine::form_main_pos_changed(WINDOWPOS *ppos)
{
    bool new_size = (ppos->flags & SWP_NOSIZE) ? false : true;
    if(new_size)
    {
        rect_form_main.bottom   = ppos->y + ppos->cy;
        rect_form_main.right    = ppos->x + ppos->cx;
        rect_form_main.left     = ppos->x;
        rect_form_main.top      = ppos->y;

        for(std::map<control_type, control*>::iterator it = controls.begin(); it != controls.end(); ++it)
        {
            if(it->second->place == cp_main)
            {
                it->second->parent_new_size = true;
                it->second->calc_pos();
            }
        }
    }
}

void skin_engine::form_info_pos_changed(WINDOWPOS *ppos)
{
    bool new_size = (ppos->flags & SWP_NOSIZE) ? false : true;
    if(new_size)
    {
        rect_form_info.bottom   = ppos->y + ppos->cy;
        rect_form_info.right    = ppos->x + ppos->cx;
        rect_form_info.left     = ppos->x;
        rect_form_info.top      = ppos->y;

        for(std::map<control_type, control*>::iterator it = controls.begin(); it != controls.end(); ++it)
        {
            if(it->second->place == cp_playlist)
            {
                it->second->parent_new_size = true;
                it->second->calc_pos();
            }
        }
        //if(infoform)
        //    RedrawWindow(infoform, NULL, NULL, RDW_INVALIDATE);
    }
}

bool skin_engine::load_defaultskin()
{
    bool ret = false;
    skin_file sfile;
    EnterCriticalSection(&cs_skin_load_ok);
    if(sfile.OpenResource(maininst, IDR_SKIN_DEFAU1, _T("SKIN_DEFAULT")))
    {
        myfile mfile;
        if(sfile.GetSubFile("config.xml", &mfile))
        {
            tinyxml2::XMLDocument doc;
            if(createconfig_list(&mfile, &doc))
            {
                mfile.close();
                if(get_config_from_skin(&doc))
                {
                    if(sfile.GetSubFile("form_main.bmp", &mfile))
                    {
                        bmp_main = new bitmap(0, 0);
                        if(bmp_main->LoadFromStream(&mfile))
                        {
                            pWidth  = bmp_main->fWidth;
                            pHeight = bmp_main->fHeight;

                            sendappmes(SKIN_NEW_FORM_SIZE, 0);
                            ret = createcontrols(&sfile, &doc);
                            if(ret)
                                skin_load = true;
                        }
                    }
                }
            }
        }
    }
    LeaveCriticalSection(&cs_skin_load_ok);
    return ret;
}

bool skin_engine::createconfig_list(myfile *pfile, tinyxml2::XMLDocument *pxml)
{
    unsigned char* xml_str = reinterpret_cast<unsigned char*>(malloc(pfile->getsize()));
    pfile->read(xml_str, pfile->getsize());
    pxml->Parse((char*)(xml_str), pfile->getsize());
    free(xml_str);
    return !pxml->Error();
}

bool skin_engine::createcontrols_(skin_file *pskinfile, const XMLNode* pchild, HDC *phdc, HDC *pmemhdc, RECT *pparentrect, control_type *pct, const std::string *pstr, control_place pplace, control* parent_cnt, bitmap *parent_bmp)
{
    if(pstr->substr(0, 6) == "button")
    {
        myfile mfile;
        string str;

        if(pchild->FirstChildElement("bitmap"))
        {
            str = string(pchild->FirstChildElement("bitmap")->GetText()) + ".bmp";
        }
        else
        {
            str = *pstr + ".bmp";
        }

        if(pskinfile->GetSubFile(str.c_str(), &mfile))
        {
            button *b;
            b = new button(pchild->FirstChildElement("x")->IntText(1),
                           pchild->FirstChildElement("y")->IntText(1),
                           pchild->FirstChildElement("region")->BoolText(false),
                           pchild->FirstChildElement("align_horz")->IntText(0),
                           pchild->FirstChildElement("align_vert")->IntText(0),
                           skin_transcolor, *pct);

            b->parent_rect = pparentrect;
            b->set_dc(*pmemhdc, *phdc);
            b->place = pplace;
            b->family = control_type2control_type1(*pct);

            if(b->loadbitmap_stream(&mfile))
            {
                controls.insert(std::make_pair(*pct, b));
            }
            else
                return false;
            //delete b; //???
        }
    }
    else if(pstr->substr(0, 6) == "scroll")
    {
        myfile mfile;
        string str = *pstr + ".bmp";
        if(pskinfile->GetSubFile(str.c_str(), &mfile))
        {
            scroll *c;
            c = new scroll(pchild->FirstChildElement("x")->IntText(1),
                           pchild->FirstChildElement("y")->IntText(1),
                           pchild->FirstChildElement("horizontal")->BoolText(true),
                           pchild->FirstChildElement("slider")->BoolText(false),
                           pchild->FirstChildElement("region")->BoolText(false),
                           pchild->FirstChildElement("align_horz")->IntText(0),
                           pchild->FirstChildElement("align_vert")->IntText(0),
                           skin_transcolor, *pct);

            c->parent_rect = pparentrect;
            c->set_dc(*pmemhdc, *phdc);
            c->place = pplace;
            c->family = control_type2control_type1(*pct);

            if(c->loadbitmap_stream(&mfile))
            {
                controls.insert(std::make_pair(*pct, c));
            }
            else
                return false;
        }
    }
    else if(pstr->substr(0, 5) == "panel")
    {
        myfile mfile;
        string str = *pstr + ".bmp";
        if(pskinfile->GetSubFile(str.c_str(), &mfile))
        {
            panel *pnl;
            pnl = new panel(pchild->FirstChildElement("left_corner_width")->IntText(1),
                            pchild->FirstChildElement("right_corner_width")->IntText(1),
                            pchild->FirstChildElement("tiled_x")->IntText(1),
                            pchild->FirstChildElement("tiled_width")->IntText(1),
                            skin_transcolor, *pct);

            pnl->parent_rect = pparentrect;
            pnl->set_dc(*pmemhdc, *phdc);
            pnl->place = pplace;
            pnl->family = control_type2control_type1(*pct);

            if(pnl->loadbitmap_stream(&mfile))
            {
                controls.insert(std::make_pair(*pct, pnl));
            }
            else
                return false;
        }
    }
    else if(pstr->substr(0, 4) == "text")
    {

    }
    else if(*pstr == "playlist")
    {
        playlist *pls;
        pls = new playlist(pchild->FirstChildElement("x")->IntText(1),
                           pchild->FirstChildElement("y")->IntText(1),
                           *pct);

        pls->parent_rect = pparentrect;
        pls->set_dc(*pmemhdc, *phdc);
        pls->place = pplace;
        pls->family = control_type2control_type1(*pct);
        controls.insert(std::make_pair(*pct, pls));
    }
    return true;
}

bool skin_engine::createcontrols(skin_file *pskinfile, tinyxml2::XMLDocument *pxml)
{
    bool ret = false;
    // main panel
    for(const XMLNode* child = pxml->FirstChildElement("main")->FirstChild(); child; child = child->NextSibling())
    {

        if(child->NoChildren())
        {
            /* std::map<std::string, control_type>::iterator it = ControlsName.find((char*)child->Value());
             if(it != ControlsName.end())
                 if(controls.find(it->second) == controls.end())
                 {

                 }*/
        }
        else
        {
            std::map<std::string, control_type>::iterator it = ControlsName.find((char*)child->Value());
            if(it != ControlsName.end())
                if(controls.find(it->second) == controls.end())
                {
                    ret = createcontrols_(pskinfile, child, &dc_main, &dc_memory, &rect_form_main, &it->second, &it->first, cp_main, NULL, bmp_main);
                }
        }
    }

    // info main panel
    if(ret)
    {
        for(const XMLNode* child = pxml->FirstChildElement("info")->FirstChild(); child; child = child->NextSibling())
        {
            if(child->NoChildren())
            {

            }
            else
            {
                std::map<std::string, control_type>::iterator it = ControlsName.find((char*)child->Value());

                if(it != ControlsName.end())
                {
                    if(controls.find(it->second) == controls.end())
                    {

                        ret = createcontrols_(pskinfile, child, &dc_info_main, &dc_info_memory, &rect_form_info, &it->second, &it->first, cp_playlist, NULL, NULL);
                    }
                }
            }
        }
    }

    if(ret)
    {
        std::map<control_type, control*>::iterator cit = controls.find(panel_pl_top);
        if(cit != controls.end())
        {
            min_pl_w = cit->second->w;
            min_pl_h = cit->second->h;
        }
    }
    return ret;
}

int skin_engine::get_min_playlist_w()
{
    return min_pl_w;
}

int skin_engine::get_min_playlist_h()
{
    return min_pl_h;
}

void skin_engine::sendappmes(WPARAM wParam, LPARAM lParam)
{
    PostMessage(mainform, SKIN_APP_MES, wParam, lParam);
}

void skin_engine::paintInfo(PAINTSTRUCT *pps)
{
    EnterCriticalSection(&cs_skin_load_ok);
    if(skin_load && show_info_panel)
    {
        for(std::map<control_type, control*>::iterator it = controls.begin(); it != controls.end(); ++it)
        {
            if(it->second->place == cp_playlist)
            {
                it->second->draw_state(pps);
            }
        }

        if(skin_color_set.use_pl_back)
        {
            panel *pnl = NULL;
            std::map<control_type, control*>::iterator it = controls.find(panel_pl_top);
            if(it != controls.end())
                pnl = reinterpret_cast<panel*>(it->second);
            else
                pnl = NULL;

            RECT ir;
            if(pnl)
                ir.top      = pnl->h;
            else
                ir.top = 0;
            ir.left     = pps->rcPaint.left;
            ir.right    = pps->rcPaint.right;
            ir.bottom   = pps->rcPaint.bottom + ir.top;

            SetBkColor(dc_info_main, skin_color_set.pl_back);
            ExtTextOutW(dc_info_main, 0, 0, ETO_OPAQUE, &ir, NULL, 0, NULL);
        }

    }
    LeaveCriticalSection(&cs_skin_load_ok);
}

void skin_engine::paintMain(RECT *pfs, PAINTSTRUCT *pps)
{
    EnterCriticalSection(&cs_skin_load_ok);
    if(skin_load)
        if(bmp_main->GetHandle())
        {
            window_bmp_blt2(bmp_main->GetHandle(), pps->rcPaint.left,
                            pps->rcPaint.top, pps->rcPaint.right,
                            pps->rcPaint.bottom, pps->rcPaint.left,
                            pps->rcPaint.top);

            for(std::map<control_type, control*>::iterator it = controls.begin(); it != controls.end(); ++it)
            {
                if(it->second->place == cp_main)
                {
                    it->second->draw_state(pps);
                }
            }
        }
    LeaveCriticalSection(&cs_skin_load_ok);
}

void skin_engine::SetInfoHandle(HWND pinfoform)
{
    infoform        = pinfoform;
    if(infoform)
    {
        GetClientRect(infoform, &rect_form_info);
        dc_info_main    = GetDC(infoform);
        dc_info_memory  = CreateCompatibleDC(dc_info_main);
        for(std::map<control_type, control*>::iterator it = controls.begin(); it != controls.end(); ++it)
        {
            if(it->second->place == cp_playlist)
            {
                it->second->set_dc(dc_info_memory, dc_info_main);
            }
        }
        show_info_panel = true;
    }
}

void skin_engine::FreeInfoHandle(HWND pinfoform)
{
    DeleteDC(dc_info_memory);
    ReleaseDC(pinfoform, dc_info_main);
    infoform = NULL;
}

bool skin_engine::main_controlAtPos(WORD px, WORD py)
{
    for(std::map<control_type, control*>::iterator it = controls.begin(); it != controls.end(); ++it)
    {
        if(it->second->touche(px, py) && (it->second->place == cp_main))
        {
           return true;
        }
    }
    return false;
}

bool skin_engine::main_mousedown(WORD px, WORD py, WPARAM wParam)
{
    if(main_cur_control != none)
    {
        if(controls[main_cur_control]->mouse_down(px, py))
        {
            if(!release_capture)
            {
                SetFocus(mainform);
                SetCapture(mainform);
                release_capture = true;
            }
            if(controls[main_cur_control]->is_scroll)
                sendappmes(SKIN_SCROLL_NEW_POS, LPARAM(controls[main_cur_control]->id));
            return true;
        }
    }
    return false;
}

bool skin_engine::main_mouseup(WORD px, WORD py, WPARAM wParam)
{
    if(main_cur_control != none)
    {
        if(controls[main_cur_control]->mouse_up(px, py, release_capture))
        {
            if(release_capture)
            {
                ReleaseCapture();
                release_capture = false;
            }
            if(controls[main_cur_control]->id != sc_vol)
                sendappmes_control_mouseup(&controls[main_cur_control]->id);
            return true;
        }
    }
    else if(release_capture)
    {
        ReleaseCapture();
        release_capture = false;
    }
    return false;
}

void skin_engine::main_mouseleave()
{
    main_cur_control = none;
    main_mouse_leave = true;
    for(std::map<control_type, control*>::iterator it = controls.begin(); it != controls.end(); ++it)
    {
        if((it->second->is_over) && (it->second->place == cp_main))
        {
            it->second->mouse_unover(false);
            break;
        }
    }
}

bool skin_engine::main_mousemove(WORD px, WORD py, WPARAM wParam)
{
    control_type old_ct = main_cur_control;
    main_mouse_leave = false;

    for(std::map<control_type, control*>::iterator it = controls.begin(); it != controls.end(); ++it)
    {
        if(it->second->touche(px, py) && (it->second->place == cp_main))
        {
            main_cur_control = it->first;
            break;
        }
        else
            main_cur_control = none;
    }

    if(old_ct != none && old_ct != main_cur_control)
    {
        controls[old_ct]->mouse_unover(release_capture);
    }

    if(main_cur_control != none)
    {
        controls[main_cur_control]->mouse_move(release_capture, px, py);
        return true;
    }
    else
        return false;
}

bool skin_engine::info_mousemove(WORD px, WORD py, WPARAM wParam)
{
    control_type old_ct = info_cur_control;
    info_mouse_leave = false;

    for(std::map<control_type, control*>::iterator it = controls.begin(); it != controls.end(); ++it)
    {
        if(it->second->touche(px, py) && (it->second->place == cp_playlist))
        {
            info_cur_control = it->first;
            break;
        }
        else
            info_cur_control = none;
    }

    if(old_ct != none && old_ct != info_cur_control)
    {
        controls[old_ct]->mouse_unover(release_capture);
    }

    if(info_cur_control != none)
    {
        controls[info_cur_control]->mouse_move(release_capture, px, py);
        return true;
    }
    else
        return false;
}

void skin_engine::info_mouseleave()
{
    info_cur_control = none;
    info_mouse_leave = true;
    for(std::map<control_type, control*>::iterator it = controls.begin(); it != controls.end(); ++it)
    {
        if((it->second->is_over) && (it->second->place == cp_playlist))
        {
            it->second->mouse_unover(false);
            break;
        }
    }
}

bool skin_engine::info_mousedown(WORD px, WORD py, WPARAM wParam)
{
    if(info_cur_control != none)
    {
        if(controls[info_cur_control]->mouse_down(px, py))
        {
            if(!release_capture)
            {
                SetFocus(mainform);
                SetCapture(mainform);
                release_capture = true;
            }
            if(controls[info_cur_control]->is_scroll)
                sendappmes(SKIN_SCROLL_NEW_POS, LPARAM(controls[info_cur_control]->id));
            return true;
        }
    }
    return false;
}

bool skin_engine::info_mouseup(WORD px, WORD py, WPARAM wParam)
{
    if(info_cur_control != none)
    {
        if(controls[info_cur_control]->mouse_up(px, py, release_capture))
        {
            if(release_capture)
            {
                ReleaseCapture();
                release_capture = false;
            }
            if(controls[info_cur_control]->id != sc_vol)
                sendappmes_control_mouseup(&controls[info_cur_control]->id);
            return true;
        }
    }
    else if(release_capture)
    {
        ReleaseCapture();
        release_capture = false;
    }
    return false;
}

void skin_engine::set_sc_volume(DWORD pvol)
{
    scroll *s = static_cast<scroll*>(controls[sc_vol]);
    if(s)
        s->set_position(pvol);
}

DWORD skin_engine::get_sc_volume()
{
    scroll *s = static_cast<scroll*>(controls[sc_vol]);
    if(s)
        return s->get_position();
    else
        return 0;
}
