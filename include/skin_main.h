#ifndef SKIN_MAIN_H_INCLUDED
#define SKIN_MAIN_H_INCLUDED

#include "common_dop.h"
#include "mystring.h"
#include "myfile.h"
#include "skin_utils.h"
#include "utils.h"
#include "skin_file.h"

#include "utils_str.h"
#include "tinyxml2.h"

using namespace tinyxml2;

#define SKIN_VER                1

#define SKIN_APP_MES            MES_CREATE_APP + 1
#define SKIN_BUTTON_MOUSE_UP    MES_CREATE_APP + 2
#define SKIN_SCROLL_NEW_POS     MES_CREATE_APP + 3
#define SKIN_NEW_FORM_SIZE      MES_CREATE_APP + 10

struct PanelRect
{
    LONG left_width;
    LONG right_width;
    LONG tiled_x;
    LONG tiled_width;
};

struct SkinColor
{
    bool use_pl_back;
    COLORREF pl_back;
};

enum bitmap_type
{
    DIB,
    DDB
};

enum skin_type
{
    _notload,
    _normal,
    _switch
};

enum scroll_orientation
{
    vertical,
    horizontal
};

enum control_align_horiz
{
    align_left = 0,
    align_right,
};

enum control_align_vert
{
    align_top = 0,
    align_bottom
};

enum control_place
{
    cp_main,
    cp_playlist
};

enum control_type_1
{
    ct_error = 0,
    ct_button,
    ct_panel,
    ct_playlist,
    ct_scroll
};

enum control_type
{
    none = 0,
    btn_play,
    btn_pause,
    btn_stop,
    btn_prev,
    btn_next,
    btn_rec,
    btn_info,
    btn_config,
    btn_min,
    btn_exit,
    sc_vol,
    // info panel
    info_btn_info,
    btn_info_exit,

    cnt_playlist,

    btn_playlist,
    btn_add,
    btn_del,

    panel_pl_top,
    panel_pl_bottom,
};

class bitmap
{
public:
    bitmap(int w, int h);
    virtual ~bitmap();
    void Clear();
    bool Empty();
    HBITMAP GetHandle();
    HBITMAP ReleaseHandle();
    void Dormant();
    void draw(HDC DC, int x, int y);
    void* GetScanLine(int y);
    COLORREF GetDIBPixels(int x, int y);
    HRGN ToRegion(COLORREF cTransparentColor = 0, COLORREF cTolerance = 0x101010);
    void CopyRect(const RECT dstrect, bitmap *srcbmp, const RECT srcrect);

    RECT GetBoundsRect() const;

    bool LoadFromFile(mystring *afile);
    bool LoadFromStream(myfile *astream);

    bitmap_type GetType();

    int fWidth, fHeight;
    HBITMAP fHandle;

private:

    bitmap_type fhandletype;

    BITMAPINFO *fDIBHeader;
    void *fDIBBits;
    int fDIBSize;

    bool fDIBAutoFree;

    unsigned short fNewPixelFormat;

    int fScanLineSize;

    myfile file;

    void ClearData();
    int CalcScanLineSize(BITMAPINFOHEADER *header);
    int GetScanLineSize();
    bool read_bitmap(myfile *afile, unsigned long apos);

//protected:
};

class control
{
public:
    control()
    {
        id = none;
        x = y = w = h = sx = sy = 0;
        parent = NULL;
        place = cp_main;
        use_region = false;
        is_over    = false;
        is_clicked = false;
        align_horz  = align_left;
        align_vert  = align_top;
        bmp_normal = NULL;
        bmp_over = NULL;
        bmp_push = NULL;
        bmp_3state = NULL;
        dc_memory = 0;
        dc_main = 0;
        region = 0;
        is_scroll   = false;
        is_text     = false;
        is_panel    = false;
        parent_new_size = true;

        parent_rect = NULL;

        tag = 0;
    }
    virtual ~control()
    {
        //mystring str;
        //mymes(str.Format(_T("%i"), (int)id));
        if(bmp_normal)
        {
            delete bmp_normal;
            bmp_normal = NULL;
        }
        if(bmp_over)
        {
            delete bmp_over;
            bmp_over = NULL;
        }
        if(bmp_push)
        {
            delete bmp_push;
            bmp_push = NULL;
        }
        if(bmp_3state)
        {
            delete bmp_3state;
            bmp_3state = NULL;
        }
        if(region)
        {
            DeleteObject(region);
            region = 0;
        }
    }

    bool touche(WORD px, WORD py);
    RECT getboundsrect() const;

    bool window_bmp_blt(HBITMAP SrcBmp, WORD hdc_x, WORD hdc_y, WORD hdc_w, WORD hdc_h, WORD srcx, WORD srcy);
    bool window_bmp_blt_trans(HBITMAP SrcBmp, HBITMAP TrBmp, WORD hdc_x, WORD hdc_y, WORD hdc_w, WORD hdc_h, WORD srcx, WORD srcy);

    virtual void calc_pos() = 0;
    virtual void draw_state(PAINTSTRUCT *pps) = 0;

    virtual bool mouse_down(WORD px, WORD py) = 0;
    virtual bool mouse_up(WORD px, WORD py, bool pforce) = 0;
    virtual bool mouse_move(bool prelease, WORD px, WORD py) = 0;
    virtual bool mouse_unover(bool pforce) = 0;

    virtual void set_parent(control *p_parent) = 0;

    HBITMAP create_bitmap_mask(HBITMAP pSrcBitmap);

    void set_dc(HDC pdc_memory, HDC pdc_main);

    control_type id;
    control_place place;
    control_type_1 family;

    DWORD tag;

    bool is_over;
    bool is_clicked;
    bool is_scroll;

    bool is_panel;
    bool is_text;

    int x, y, w, h;
    int sx, sy;

    control_align_horiz align_horz;
    control_align_vert align_vert;

    RECT *parent_rect;
    bool parent_new_size;

protected:

    void _setparent(control *p_parent);

    HDC dc_memory;
    HDC dc_main;
    HRGN region;
    //bitmap *bmp_buf;

    control *parent;
    bitmap *bmp_normal;
    bitmap *bmp_over, *bmp_push, *bmp_3state;

    COLORREF color_t;

    bool use_region;

};

class playlist: public control
{
public:
    playlist(int _x, int _y, control_type pid);
    virtual ~playlist();

    void calc_pos();
    void draw_state(PAINTSTRUCT *pps);

    bool mouse_down(WORD px, WORD py);
    bool mouse_up(WORD px, WORD py, bool pforce);
    bool mouse_move(bool prelease, WORD px, WORD py);
    bool mouse_unover(bool pforce);

    void set_parent(control *p_parent);
private:

};

class panel: public control
{
public:
    panel(int _leftw, int _rightw, int _tiledx, int _tiledw, COLORREF ptrans_color, control_type pid);
    virtual ~panel();
    bool loadbitmap(mystring *pfilename);
    bool loadbitmap_stream(myfile *pfile);

    void calc_pos();
    void draw_state(PAINTSTRUCT *pps);

    bool mouse_down(WORD px, WORD py);
    bool mouse_up(WORD px, WORD py, bool pforce);
    bool mouse_move(bool prelease, WORD px, WORD py);
    bool mouse_unover(bool pforce);

    void set_parent(control *p_parent);

private:
    PanelRect panel_rect;
};

class button: public control
{
public:
    button(int _x, int _y, bool _useregion, int _alignH, int _alighV, COLORREF ptrans_color, control_type pid);
    virtual ~button();
    bool loadbitmap(mystring *pfilename);
    bool loadbitmap_stream(myfile *pfile);

    void calc_pos();
    void draw_state(PAINTSTRUCT *pps);

    bool mouse_down(WORD px, WORD py);
    bool mouse_up(WORD px, WORD py, bool pforce);
    bool mouse_move(bool prelease, WORD px, WORD py);
    bool mouse_unover(bool pforce);

    void set_parent(control *p_parent);

private:
    void copymainbmp(bitmap *pbmp);
    bool use_3state;
};

class scroll: public control
{
public:
    scroll(int _x, int _y, bool _ishoriz, bool _useslider, bool _useregion, int _alignH, int _alighV, COLORREF ptrans_color, control_type pid);
    virtual ~scroll();
    bool loadbitmap(mystring *pfilename);
    bool loadbitmap_stream(myfile *pfile);

    void calc_pos();
    void draw_state(PAINTSTRUCT *pps);

    bool mouse_down(WORD px, WORD py);
    bool mouse_up(WORD px, WORD py, bool pforce);
    bool mouse_move(bool prelease, WORD px, WORD py);
    bool mouse_unover(bool pforce);

    void set_position(int pnewpos);
    int get_position();

    void set_parent(control *p_parent);

    scroll_orientation type;
    int fmax, fmin;
private:
    void copymainbmp(bitmap *pbmp);
    bool use_slider;

    bool move_mouse;

    int fposition;
    int positionwaarde;

    void draw_from_position();

    HBITMAP bmp_slider_tr;
    bitmap *bmp_slider;
};

#define SE_CONTROLS_COUNT 13

class skin_engine
{
    //static const char* _ControlsName[SE_CONTROLS_COUNT][SE_CONTROLS_COUNT];
    typedef map<std::string, control_type> _ControlsName;
    static _ControlsName ControlsName;
public:
    skin_engine(HWND pform, HDC pdc_main, HDC pdc_memory, HINSTANCE pInst, mystring *adir);
    ~skin_engine();

    bool load_defaultskin();
    bool load_skinfile(wchar_t *pfilename);

    bool createconfig_list(myfile *pfile, tinyxml2::XMLDocument *pxml);
    bool createcontrols_(skin_file *pskinfile, const XMLNode* pchild, HDC *phdc, HDC *pmemhdc, RECT *pparentrect, control_type *pct, const std::string *pstr, control_place pplace, control* parent_cnt, bitmap *parent_bmp);
    bool createcontrols(skin_file *pskinfile, tinyxml2::XMLDocument *pxml);

    void paintMain(RECT *pfs, PAINTSTRUCT *pps);
    void paintInfo(PAINTSTRUCT *pps);

    bool main_mousemove(WORD px, WORD py, WPARAM wParam);
    void main_mouseleave();
    bool main_mousedown(WORD px, WORD py, WPARAM wParam);
    bool main_mouseup(WORD px, WORD py, WPARAM wParam);

    bool main_controlAtPos(WORD px, WORD py);

    bool info_mousemove(WORD px, WORD py, WPARAM wParam);
    void info_mouseleave();
    bool info_mousedown(WORD px, WORD py, WPARAM wParam);
    bool info_mouseup(WORD px, WORD py, WPARAM wParam);

    void SetInfoHandle(HWND pinfoform);
    void FreeInfoHandle(HWND pinfoform);

    void sendappmes(WPARAM wParam, LPARAM lParam);

    void set_sc_volume(DWORD pvol);
    DWORD get_sc_volume();

    int get_min_playlist_w();
    int get_min_playlist_h();

    void form_main_pos_changed(WINDOWPOS *ppos);
    void form_info_pos_changed(WINDOWPOS *ppos);

    unsigned long pHeight, pWidth;

private:
    HINSTANCE maininst;

    HWND mainform;
    HWND infoform;

    HDC dc_memory;
    HDC dc_main;

    HDC dc_info_memory;
    HDC dc_info_main;

    mystring skinpath;

    skin_type skintype;
    COLORREF skin_transcolor;

    CRITICAL_SECTION cs_skin_load_ok;

    control_type main_cur_control;
    control_type info_cur_control;

    //bitmap *bmp_dubbuf;
    bitmap *bmp_main;

    bool auto_mini;
    bool release_capture;

    bool main_mouse_leave;
    bool info_mouse_leave;

    bool win_panel_skin;

    bool skin_load;
    bool show_info_panel;

    RECT rect_form_main;
    RECT rect_form_info;

    SkinColor skin_color_set;

    int skin_ver;
    int min_pl_w, min_pl_h;

    //map<control_type, control*> controls_main;
    //map<control_type, control*> controls_info;
    map<control_type, control*> controls;

    bool get_config_from_skin(tinyxml2::XMLDocument *pxml); // map<string, string> *pconflist

    bool window_bmp_blt2(HBITMAP SrcBmp, int srcx, int srcy, int srcw, int srch, int dstx, int dsty)
    {
        if(srcw && srch)
        {
            HBITMAP h = (HBITMAP)SelectObject(dc_memory, SrcBmp);
            int retval = BitBlt(dc_main, srcx, srcy, srcw, srch,
                                dc_memory, dstx, dsty,
                                SRCCOPY);
            SelectObject(dc_memory, h);
            return retval ? true : false;
        }
        return false;
    }

    void sendappmes_control_mouseup(control_type *pct)
    {
        sendappmes(SKIN_BUTTON_MOUSE_UP, LPARAM(*pct));
    }

    control_type_1 control_type2control_type1(control_type pct)
    {
        switch(pct)
        {
        case none:
            return ct_error;
            break;
        case btn_play:
        case btn_pause:
        case btn_stop:
        case btn_prev:
        case btn_next:
        case btn_rec:
        case btn_info:
        case btn_config:
        case btn_min:
        case btn_exit:
        case btn_info_exit:
        case btn_playlist:
        case btn_add:
        case btn_del:
        case info_btn_info:
            return ct_button;
            break;
        case sc_vol:
            return ct_scroll;
            break;
        case cnt_playlist:
            return ct_playlist;
            break;
        case panel_pl_top:
        case panel_pl_bottom:
            return ct_panel;
            break;
        default:
            return ct_error;
        }
    }

    /*void GetIntParam(string *pstr, char* delim, std::vector<int>* results)
    {
        unsigned int cutAt;
        while((cutAt = pstr->find_first_of(delim)) != string::npos)
        {
            if(cutAt > 0)
            {
                results->push_back(atoi(pstr->substr(0,cutAt).c_str()));
            }
            *pstr = pstr->substr(cutAt+1);
        }
        if(pstr->length() > 0)
        {
            results->push_back(atoi(pstr->c_str()));
        }
    }

    void GetStrParam(string *pstr, char* delim, std::vector<string>* results)
    {
        unsigned int cutAt;
        while((cutAt = pstr->find_first_of(delim)) != string::npos)
        {
            if(cutAt > 0)
            {
                results->push_back(pstr->substr(0,cutAt));
            }
            *pstr = pstr->substr(cutAt+1);
        }
        if(pstr->length() > 0)
        {
            results->push_back(*pstr);
        }
    }*/

};


#endif // SKIN_MAIN_H_INCLUDED
