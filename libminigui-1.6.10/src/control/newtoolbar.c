/*
** $Id: newtoolbar.c 6976 2006-08-17 08:17:35Z xwyan $
**
** newtoolbar.c: the new-toolbar control module.
**
** This control is a substitute of old ToolBar control.
** New application should use the NewToolBar control
** instead of ToolBar control.
**
** Copyright (C) 2003 ~ 2006 Feynman Software.
**
** All rights reserved by Feynman Software.
**
** Current maintainer: Wang Xiaogang
**
** Create date: 2003/04/24
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "ctrl/ctrlhelper.h"
#include "ctrl/newtoolbar.h"
#include "cliprect.h"
#include "internals.h"
#include "ctrlclass.h"

#ifdef _CTRL_NEWTOOLBAR

#include "ctrlmisc.h"
#include "newtoolbar_impl.h"

static int NewToolbarCtrlProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam);


BOOL RegisterNewToolbarControl (void)
{
    WNDCLASS WndClass;

    WndClass.spClassName = CTRL_NEWTOOLBAR;
    WndClass.dwStyle     = WS_NONE;
    WndClass.dwExStyle   = WS_EX_NONE;
    WndClass.hCursor     = GetSystemCursor (IDC_ARROW);
    WndClass.iBkColor    = GetWindowElementColor (BKC_CONTROL_DEF);
    WndClass.WinProc     = NewToolbarCtrlProc;

    return AddNewControlClass (&WndClass) == ERR_OK;
}



static void line_a2b (HDC hdc, POINT *a, POINT *b, int color)
{
    SetPenColor (hdc, color);
    MoveTo (hdc, a->x, a->y);
    LineTo (hdc, b->x, b->y);
}



static void draw_left_right_line_vert (HDC hdc, RECT *rc)
{
    POINT       pta, ptb;
#ifdef _FLAT_WINDOW_STYLE    
    pta.x = 0; pta.y = 0;
    ptb.x = 0; ptb.y = rc->bottom;
    line_a2b (hdc, &pta, &ptb, COLOR_black);

    pta.x = rc->right - 1 ; pta.y = 0;
    ptb.x = rc->right - 1; ptb.y = rc->bottom;
    line_a2b (hdc, &pta, &ptb, COLOR_black);

    /* draw two separator lines */
    pta.y = 2; pta.x = MARGIN_V_HORZ;
    ptb.y = pta.y; ptb.x = rc->right - MARGIN_V_HORZ - 1;
    line_a2b (hdc, &pta, &ptb, COLOR_black);

    pta.y = 5; pta.x = MARGIN_V_HORZ;
    ptb.y = pta.y; ptb.x = rc->right - MARGIN_V_HORZ - 1;
    line_a2b (hdc, &pta, &ptb, COLOR_black);
#else 
    rc->bottom -= 1;
    rc->right -= 1;

    pta.x = 0; pta.y = 0;
    ptb.x = 0; ptb.y = rc->bottom;
    line_a2b (hdc, &pta, &ptb, COLOR_darkgray);
    pta.x += 1; ptb.x = pta.x;
    line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);

    pta.x = rc->right - 1 ; pta.y = 0;
    ptb.x = rc->right - 1 ; ptb.y = rc->bottom;
    line_a2b (hdc, &pta, &ptb, COLOR_darkgray);
    pta.x += 1; ptb.x = pta.x;
    line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);

    /* draw two separator lines */
    pta.y = 2; pta.x = MARGIN_V_HORZ;
    ptb.y = pta.y; ptb.x = rc->right - MARGIN_V_HORZ - 1;
    line_a2b (hdc, &pta, &ptb, COLOR_darkgray);
    pta.y += 1; ptb.y = pta.y;
    line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);

    pta.y = 5; pta.x = MARGIN_V_HORZ;
    ptb.y = pta.y; ptb.x = rc->right - MARGIN_V_HORZ - 1;
    line_a2b (hdc, &pta, &ptb, COLOR_darkgray);
    pta.y += 1; ptb.y = pta.y;
    line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);
#endif
}

static int get_max_right_in_col (NTBITEM *item, NTBCTRLDATA *ntbar)
{
    NTBITEM *it;
    int maxr = 0;
    
    it = item;
    
    if (!(ntbar->style & NTBS_WITHTEXT)) {
        return it->rc_item.right;
    }    
       
    while (it && ((it->flags & NTBIF_TYPEMASK) != NTBIF_NEWLINE)) {
        
        if (it->rc_text.right > maxr) {
            if ((it->flags & NTBIF_TYPEMASK) != NTBIF_SEPARATOR 
                && (it->flags & NTBIF_TYPEMASK) != NTBIF_NEWLINE) {
                maxr = it->rc_text.right;
            }
        }
        it = it->prev;
    }

    it = item;
    while (it && ((it->flags & NTBIF_TYPEMASK) != NTBIF_NEWLINE)) {
        
        if (it->rc_text.right > maxr) {
            if ((it->flags & NTBIF_TYPEMASK) != NTBIF_SEPARATOR 
                && (it->flags & NTBIF_TYPEMASK) != NTBIF_NEWLINE) {
                maxr = it->rc_text.right;
            }
        }
        it = it->next;
    }
    return maxr;
}


static void draw_separator_vert (HDC hdc, int l, int t, NTBCTRLDATA *ntbar, NTBITEM *it)
{
    POINT pta, ptb;
#ifdef _FLAT_WINDOW_STYLE
    pta.x = l;
    pta.y = t + 2;
    ptb.y = pta.y;
    ptb.x = get_max_right_in_col (it, ntbar) - MARGIN_V_HORZ * 2;
    line_a2b (hdc, &pta, &ptb, COLOR_black);
#else
    pta.x = l;
    pta.y = t + 1;
    ptb.x = get_max_right_in_col (it, ntbar) - MARGIN_V_HORZ * 2;
    ///printf ("get_max_right_in_col (it, ntbar) == %d \n", get_max_right_in_col (it, ntbar));
    ptb.y = pta.y;
    line_a2b (hdc, &pta, &ptb, COLOR_darkgray);    

    pta.x = l;
    pta.y += 1;
    ptb.x = get_max_right_in_col (it, ntbar) - MARGIN_V_HORZ * 2;
    ptb.y = pta.y;
    line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);
#endif
}



static void draw_checked_item_vert (HDC hdc, PNTBCTRLDATA ntb_data, 
                 NTBITEM *item, int l, int t, int w, int h)
{
    if (ntb_data->nr_cols >= 3) {
        FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image,
                               w * 2, h * item->bmp_cell);
    } else {
        if (ntb_data->nr_cols >=2 ) {
            FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image, 
                                   w, h * item->bmp_cell);
        } else {
            POINT pta, ptb;
            int r, b;
            
            FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image,                     
                                   0, h * item->bmp_cell);
            r = l + w ;
            b = t + h ;
            
            pta.x = l; pta.y = t;
            ptb.x = l; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_black);
            pta.x = l; pta.y = t;
            ptb.x = r; ptb.y = t;
            line_a2b (hdc, &pta, &ptb, COLOR_black);
            
            pta.x = r; pta.y = t;
            ptb.x = r; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_black);
            pta.x = l; pta.y = b;
            ptb.x = r; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_black);
        }
    }
}



static void draw_normal_item_vert (HDC hdc, PNTBCTRLDATA ntb_data, 
                NTBITEM *item, int l, int t, int w, int h)
{
    if (item == ntb_data->sel_item) {
        if (ntb_data->btn_down && ntb_data->nr_cols >= 3) {
            FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image,
                                   w * 2, h * item->bmp_cell);
        } else {
            if (!ntb_data->btn_down && ntb_data->nr_cols >= 2)
                FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image,
                                       w, h * item->bmp_cell);
            else
                FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image,
                                       0, h * item->bmp_cell);
        }
    } else {
        FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image,
                               0, h * item->bmp_cell);
    }
}



static void draw_disabled_item_vert (HDC hdc, PNTBCTRLDATA ntb_data, 
                NTBITEM *item, int l, int t, int w, int h)
{
    if (ntb_data->nr_cols == 4) {
        FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image, 
                               w * 3, h * item->bmp_cell);
    } else {
        FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image, 
                               0, h * item->bmp_cell);
    }
}



static void draw_item_state_vert (HDC hdc, PNTBCTRLDATA ntb_data, 
                NTBITEM *item, int l, int t, int w, int h, int r, int b)
{
    if ((ntb_data->style & NTBS_WITHTEXT) && !(ntb_data->style & NTBS_TEXTRIGHT)) {
        r = l + w;
        b = t + h;
    } else {
        r--; b--;
    }
    
    if (!(item->flags & NTBIF_DISABLED) && item == ntb_data->sel_item) {
        POINT pta, ptb;
        if (ntb_data->btn_down) {
            pta.x = l; pta.y = t;
            ptb.x = l; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_darkgray);
            pta.x = l; pta.y = t;
            ptb.x = r; ptb.y = t;
            line_a2b (hdc, &pta, &ptb, COLOR_darkgray);
            
            pta.x = r; pta.y = t;
            ptb.x = r; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);
            pta.x = l; pta.y = b;
            ptb.x = r; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);
        }
        else {
            pta.x = l; pta.y = t;
            ptb.x = l; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);
            pta.x = l; pta.y = t;
            ptb.x = r; ptb.y = t;
            line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);
            
            pta.x = r; pta.y = t;
            ptb.x = r; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_darkgray);
            pta.x = l; pta.y = b;
            ptb.x = r; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_darkgray);
        }
    }
}



static void draw_item_text_vert (HWND hwnd, HDC hdc, PNTBCTRLDATA ntb_data, NTBITEM *item)
{
    DWORD format = DT_SINGLELINE | DT_VCENTER;
    
    if (ntb_data->style & NTBS_TEXTRIGHT)
        format |= DT_LEFT;
    else
        format |= DT_CENTER;

    SetBkMode (hdc, BM_TRANSPARENT);
    SetBkColor (hdc, GetWindowBkColor (hwnd));
    SetTextColor (hdc, GetWindowElementColorEx (hwnd, FGC_CONTROL_NORMAL));
    DrawText (hdc, item->text, -1, &item->rc_text, format);
            
}



static void draw_tool_bar_vert (HWND hwnd, HDC hdc, PNTBCTRLDATA ntb_data)
{
    int l, t, r, b, w, h;
    RECT rc;
    NTBITEM *item;

    GetClientRect (hwnd, &rc);
   
    draw_left_right_line_vert (hdc, &rc);

    item = ntb_data->head;

    while (item) {
        
        if ((item->flags & NTBIF_TYPEMASK) == NTBIF_NEWLINE) {
            /*
             * if new line just skip it :)
             */
            if ((ntb_data->style & NTBS_MULTLINE) == FALSE) {
                break;
            }
            item = item->next;
        }
        
        l = item->rc_item.left;
        t = item->rc_item.top;
        r = item->rc_item.right;
        b = item->rc_item.bottom;
        w = ntb_data->w_cell;
        h = ntb_data->h_cell;
                
        if ((item->flags & NTBIF_TYPEMASK) == NTBIF_SEPARATOR) { 
            /* 
             * draw separator 
             */
            if (ntb_data->style & NTBS_DRAWSEPARATOR) {
                draw_separator_vert (hdc, l, t, ntb_data, item);
            }
            item = item->next;
            continue;
        } 

        if (item->flags & NTBIF_DISABLED) { 
            /*
             * draw disabled item 
             */
            draw_disabled_item_vert (hdc, ntb_data, item, l, t, w, h);
        
        } else {
            
            if (item->flags & NTBIF_CHECKED) { 
                /*
                 * draw checked item
                 */
                draw_checked_item_vert (hdc, ntb_data, item, l, t, w, h);
            
            } else {
                /*
                 * draw normal item
                 */
                draw_normal_item_vert (hdc, ntb_data, item, l, t, w, h);
            }
        }

        if (ntb_data->style & NTBS_DRAWSTATES) {
            /*
             * draw state (like a box)
             */
            draw_item_state_vert (hdc, ntb_data, item, l, t, w, h, r, b);
        }

        if (ntb_data->style & NTBS_WITHTEXT) {
            /*
             * draw item's text 
             */
            draw_item_text_vert (hwnd, hdc, ntb_data, item);
        }

        item = item->next;
    }
}



static void draw_top_bottom_line_horz (HDC hdc, RECT *rc)
{
    POINT       pta, ptb;
    
#ifdef _FLAT_WINDOW_STYLE
    pta.x = 0; pta.y = 0;
    ptb.x = rc->right; ptb.y = pta.y;
    line_a2b (hdc, &pta, &ptb, COLOR_black);

    pta.x = 0; pta.y = rc->bottom - 1;
    ptb.x = rc->right; ptb.y = pta.y;
    line_a2b (hdc, &pta, &ptb, COLOR_black);

    /* draw two separator lines */
    pta.x = 2; pta.y = MARGIN_VERT;
    ptb.x = pta.x; ptb.y = rc->bottom - MARGIN_VERT - 1;
    line_a2b (hdc, &pta, &ptb, COLOR_black);

    pta.x = 5; pta.y = MARGIN_VERT;
    ptb.x = pta.x; ptb.y = rc->bottom - MARGIN_VERT - 1;
    line_a2b (hdc, &pta, &ptb, COLOR_black);
#else
    rc->bottom -= 1;
    rc->right -= 1;

    pta.x = 0; pta.y = 0;
    ptb.x = rc->right; ptb.y = pta.y;
    line_a2b (hdc, &pta, &ptb, COLOR_darkgray);
    pta.y += 1; ptb.y = pta.y;
    line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);

    pta.x = 0; pta.y = rc->bottom - 1;
    ptb.x = rc->right; ptb.y = pta.y;
    line_a2b (hdc, &pta, &ptb, COLOR_darkgray);
    pta.y += 1; ptb.y = pta.y;
    line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);

    /* draw two separator lines */
    pta.x = 2; pta.y = MARGIN_VERT;
    ptb.x = pta.x; ptb.y = rc->bottom - MARGIN_VERT - 1;
    line_a2b (hdc, &pta, &ptb, COLOR_darkgray);
    pta.x += 1; ptb.x = pta.x;
    line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);

    pta.x = 5; pta.y = MARGIN_VERT;
    ptb.x = pta.x; ptb.y = rc->bottom - MARGIN_VERT - 1;
    line_a2b (hdc, &pta, &ptb, COLOR_darkgray);
    pta.x += 1; ptb.x = pta.x;
    line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);
#endif /* _FLAT_WINDOW_STYLE */
}



static void draw_separator_horz (HDC hdc, int l, int t, NTBITEM *it)
{
    POINT pta, ptb;
#ifdef _FLAT_WINDOW_STYLE
    pta.x = l + 2;
    pta.y = t;
    ptb.x = pta.x;
    ptb.y = it->rc_item.bottom;
    line_a2b (hdc, &pta, &ptb, COLOR_black);
#else
    pta.x = l + 1;
    pta.y = t;
    ptb.x = pta.x;
    ptb.y = it->rc_item.bottom;
    line_a2b (hdc, &pta, &ptb, COLOR_darkgray);
    
    pta.x += 1;
    pta.y = t;
    ptb.x = pta.x;
    ptb.y = it->rc_item.bottom - MARGIN_VERT;
    line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);
#endif
}



static void draw_disabled_item_horz (HDC hdc, PNTBCTRLDATA ntb_data, 
                                     NTBITEM *item, int l, int t, int w, int h)
{
    if (ntb_data->nr_cols == 4) {
        FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image, 
                               w * 3, h * item->bmp_cell);
    } else {
        FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image, 
                               0, h * item->bmp_cell);
    }
}



static void draw_checked_item_horz (HDC hdc, PNTBCTRLDATA ntb_data, 
                                    NTBITEM *item, int l, int t, int w, int h)
{
    if (ntb_data->nr_cols >= 3) {
        FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image,
                               w * 2, h * item->bmp_cell);
    } else {
        if (ntb_data->nr_cols >=2 ) {
            FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image, 
                                   w, h * item->bmp_cell);
        } else {
            int r, b;
            POINT pta, ptb;

            FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image, 
                                   0, h * item->bmp_cell);
            r = l + w ;
            b = t + h ;
            pta.x = l; pta.y = t;
            ptb.x = l; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_black);
            pta.x = l; pta.y = t;
            ptb.x = r; ptb.y = t;
            line_a2b (hdc, &pta, &ptb, COLOR_black);
            
            pta.x = r; pta.y = t;
            ptb.x = r; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_black);
            pta.x = l; pta.y = b;
            ptb.x = r; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_black);
        }
    }
    
}



static void draw_normal_item_horz (HDC hdc, PNTBCTRLDATA ntb_data, 
                                   NTBITEM *item, int l, int t, int w, int h)
{
    if (item == ntb_data->sel_item) {
        if (ntb_data->btn_down && ntb_data->nr_cols >= 3) {
            FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image,
                                   w * 2, h * item->bmp_cell);
        } else {
            if (!ntb_data->btn_down && ntb_data->nr_cols >= 2)
                FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image,
                                       w, h * item->bmp_cell);
            else {
                FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image,
                                       0, h * item->bmp_cell);
            }
        }
    } else {
        FillBoxWithBitmapPart (hdc, l, t, w, h, 0, 0, ntb_data->image,
                               0, h * item->bmp_cell);
    }
}



static void draw_item_state_horz (HDC hdc, PNTBCTRLDATA ntb_data, 
                NTBITEM *item, int l, int t, int w, int h, int r, int b)
{
    POINT pta, ptb;

    if ((ntb_data->style & NTBS_WITHTEXT) && !(ntb_data->style & NTBS_TEXTRIGHT)) {
        r = l + w;
        b = t + h;
    } else {
        r--; b--;
    }
    
    if (!(item->flags & NTBIF_DISABLED) && item == ntb_data->sel_item) {
        if (ntb_data->btn_down) {
            pta.x = l; pta.y = t;
            ptb.x = l; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_darkgray);
            pta.x = l; pta.y = t;
            ptb.x = r; ptb.y = t;
            line_a2b (hdc, &pta, &ptb, COLOR_darkgray);

            pta.x = r; pta.y = t;
            ptb.x = r; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);
            pta.x = l; pta.y = b;
            ptb.x = r; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);
        } else {
            pta.x = l; pta.y = t;
            ptb.x = l; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);
            pta.x = l; pta.y = t;
            ptb.x = r; ptb.y = t;
            line_a2b (hdc, &pta, &ptb, COLOR_lightwhite);
            
            pta.x = r; pta.y = t;
            ptb.x = r; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_darkgray);
            pta.x = l; pta.y = b;
            ptb.x = r; ptb.y = b;
            line_a2b (hdc, &pta, &ptb, COLOR_darkgray);
        }
    }
}



static void draw_item_text_horz (HWND hwnd, HDC hdc, PNTBCTRLDATA ntb_data, NTBITEM *item)
{
    DWORD format = DT_SINGLELINE | DT_VCENTER;
    
    if (ntb_data->style & NTBS_TEXTRIGHT) {
        format |= DT_LEFT;
    } else {
        format |= DT_CENTER;
    }
    SetBkMode (hdc, BM_TRANSPARENT);
    SetBkColor (hdc, GetWindowBkColor (hwnd));
    SetTextColor (hdc, GetWindowElementColorEx (hwnd, FGC_CONTROL_NORMAL));
    DrawText (hdc, item->text, -1, &item->rc_text, format);
}



static void draw_tool_bar_horz (HWND hwnd, HDC hdc, PNTBCTRLDATA ntb_data)
{
    int         l, t, r, b, w, h;
    RECT        rc;
    
    NTBITEM*    item = NULL;
    
    GetClientRect (hwnd, &rc);
    draw_top_bottom_line_horz (hdc, &rc);
    item = ntb_data->head;
    
    while (item) {
        
        if ((item->flags & NTBIF_TYPEMASK) == NTBIF_NEWLINE) {
            /*
             * if new line just skip it :)
             */
            if ((ntb_data->style & NTBS_MULTLINE) == FALSE) {
                break;
            }
            item = item->next;
        }
        
        l = item->rc_item.left;
        t = item->rc_item.top;
        r = item->rc_item.right;
        b = item->rc_item.bottom;
        w = ntb_data->w_cell;
        h = ntb_data->h_cell;
        
        if ((item->flags & NTBIF_TYPEMASK) == NTBIF_SEPARATOR) {
            /* 
             * draw separator 
             */
            if (ntb_data->style & NTBS_DRAWSEPARATOR) {
                draw_separator_horz (hdc, l, t, item);
            }
            item = item->next;
            continue;
        }

        if (item->flags & NTBIF_DISABLED) {
            /*
             * draw disalbed item 
             */
            draw_disabled_item_horz (hdc, ntb_data, item, l, t, w, h);

        } else {
            
            if (item->flags & NTBIF_CHECKED) {
                /* 
                 * draw checked item 
                 */
                draw_checked_item_horz (hdc, ntb_data, item, l, t, w, h);
            
            } else {
                /*
                 * draw normal item 
                 */
                draw_normal_item_horz (hdc, ntb_data, item, l, t, w, h);
            }
        } 
        if (ntb_data->style & NTBS_DRAWSTATES) {
            /* 
             * draw item's state
             */
            draw_item_state_horz (hdc, ntb_data, item, l, t, w, h, r, b);
        }

        if (ntb_data->style & NTBS_WITHTEXT) {
            /*
             * draw item's text
             */
            draw_item_text_horz (hwnd, hdc, ntb_data, item);
        }

        item = item->next;
    } 
}



static NTBITEM *get_item_by_pos (PNTBCTRLDATA ntbar, int posx, int posy)
{
    NTBITEM* item;

    item = ntbar->head;
    while (item) {
        if (!(item->flags & NTBIF_DISABLED) && PtInRect (&item->rc_item, posx, posy)) {
            return item;
        }
        item = item->next;
    }
    return NULL;
}



static int get_last_col_max_right (NTBCTRLDATA *ntbar)
{
    NTBITEM *it;
    int maxr = 0;
    
    it = ntbar->tail->prev;
    while (it && ((it->flags & NTBIF_TYPEMASK) != NTBIF_NEWLINE)) {
        if (it->rc_item.right > maxr) { 
            maxr = it->rc_item.right; 
        }
        it = it->prev;
    }
    
    return maxr;
}


static void append_new_item (HWND hwnd, NTBCTRLDATA* ntbar, NTBITEM* new_item)
{
    HDC hdc;

    hdc = GetClientDC (hwnd);
    if (!(ntbar->style & NTBS_VERTICAL)) {
        
        //horizontal
        if (ntbar->tail == NULL) {
            
            new_item->rc_item.top = MARGIN_VERT;
            new_item->rc_item.left = MARGIN_HORZ;
        
        } else {
            
            if ((new_item->flags & NTBIF_TYPEMASK ) == NTBIF_NEWLINE) {
                new_item->rc_item.top = ntbar->tail->rc_item.bottom + GAP_ITEM_ITEM_VERT;
                new_item->rc_item.left = MARGIN_HORZ;
                new_item->rc_item.bottom = new_item->rc_item.top;
                new_item->rc_item.right = new_item->rc_item.left;
                goto  end;
            
            } else {
            
                new_item->rc_item.top = ntbar->tail->rc_item.top;
                if ((ntbar->tail->flags & NTBIF_TYPEMASK) == NTBIF_NEWLINE)
                    new_item->rc_item.left = MARGIN_HORZ;
                else
                    new_item->rc_item.left = ntbar->tail->rc_item.right + GAP_ITEM_ITEM_HORZ;
            
            }
        }
        
        
        switch (new_item->flags & NTBIF_TYPEMASK) {
        case NTBIF_PUSHBUTTON:
        case NTBIF_CHECKBUTTON:
        case NTBIF_HOTSPOTBUTTON:
            if (ntbar->style & NTBS_WITHTEXT) {
                if (ntbar->style & NTBS_TEXTRIGHT) {

                    SIZE sz_text;
                    GetTextExtent (hdc, new_item->text, -1, &sz_text);

                    new_item->rc_item.bottom 
                            = new_item->rc_item.top + ntbar->h_cell + 1;

                    new_item->rc_text.left = new_item->rc_item.left + ntbar->w_cell + GAP_BMP_TEXT_HORZ;
                    new_item->rc_text.right = new_item->rc_text.left + sz_text.cx;
                    new_item->rc_text.top = new_item->rc_item.top;
                    new_item->rc_text.bottom = new_item->rc_item.bottom;

                    new_item->rc_item.right = new_item->rc_text.right + GAP_BMP_TEXT_HORZ + 1;
                } else {
                    new_item->rc_item.right = new_item->rc_item.left + ntbar->w_cell + 1;

                    new_item->rc_text.left = new_item->rc_item.left;
                    new_item->rc_text.right = new_item->rc_item.right;
                    new_item->rc_text.top 
                            = new_item->rc_item.top + ntbar->h_cell + GAP_BMP_TEXT_VERT;
                    new_item->rc_text.bottom = new_item->rc_text.top + GetFontHeight (hdc);

                    new_item->rc_item.bottom = new_item->rc_text.bottom + 1;
                }
            } else {
                new_item->rc_item.right = new_item->rc_item.left + ntbar->w_cell + 1;
                new_item->rc_item.bottom = new_item->rc_item.top + ntbar->h_cell + 1;
            }
            break;
            
        case NTBIF_NEWLINE:
        case NTBIF_SEPARATOR:
        default:
            if (ntbar->style & NTBS_DRAWSEPARATOR) {
                new_item->rc_item.right = new_item->rc_item.left + WIDTH_SEPARATOR;
            } else {
                new_item->rc_item.right = new_item->rc_item.left + WIDTH_SEPARATOR * 2;
            }
            new_item->rc_item.bottom = new_item->rc_item.top + ntbar->h_cell;
            break;
        }
    }
    else
    {
        // vertical 
        
        if (ntbar->tail == NULL) {
            /*
             * It is the first item of the bar! 
             */
            new_item->rc_item.top = MARGIN_V_VERT;
            new_item->rc_item.left = MARGIN_V_HORZ;
        } else {
            
            if ((new_item->flags & NTBIF_TYPEMASK ) == NTBIF_NEWLINE) {
                new_item->rc_item.left = ntbar->tail->rc_item.right + GAP_ITEM_ITEM_HORZ;                    
                new_item->rc_item.top = MARGIN_V_VERT;
                new_item->rc_item.bottom = new_item->rc_item.top;
                new_item->rc_item.right = new_item->rc_item.left;
                goto  end;
            } else {
                if ((ntbar->tail->flags & NTBIF_TYPEMASK) == NTBIF_NEWLINE) {
                    new_item->rc_item.top = MARGIN_V_VERT; 
                    new_item->rc_item.left = get_last_col_max_right (ntbar) + GAP_ITEM_ITEM_HORZ; /**/
                } else {
                    new_item->rc_item.top = ntbar->tail->rc_item.bottom + GAP_ITEM_ITEM_VERT;
                    new_item->rc_item.left = ntbar->tail->rc_item.left; 
                }
                //new_item->rc_item.left = ntbar->tail->rc_item.left;
                
            }
        }
        switch (new_item->flags & NTBIF_TYPEMASK) {
        case NTBIF_PUSHBUTTON:
        case NTBIF_CHECKBUTTON:
        case NTBIF_HOTSPOTBUTTON:
            if (ntbar->style & NTBS_WITHTEXT) {
                if (ntbar->style & NTBS_TEXTRIGHT) {

                    SIZE sz_text;
                    GetTextExtent (hdc, new_item->text, -1, &sz_text);
                    
                    new_item->rc_item.bottom 
                            = new_item->rc_item.top + ntbar->h_cell + 1;

                    new_item->rc_text.left = new_item->rc_item.left + ntbar->w_cell + GAP_BMP_TEXT_HORZ;
                    new_item->rc_text.right = new_item->rc_text.left + sz_text.cx;
                    new_item->rc_text.top = new_item->rc_item.top;
                    new_item->rc_text.bottom = new_item->rc_item.bottom;

                    new_item->rc_item.right = new_item->rc_text.right + GAP_BMP_TEXT_HORZ + 1;
                } else {
                    new_item->rc_item.right = new_item->rc_item.left + ntbar->w_cell + 1;

                    new_item->rc_text.left = new_item->rc_item.left;
                    new_item->rc_text.right = new_item->rc_item.right;
                    new_item->rc_text.top 
                            = new_item->rc_item.top + ntbar->h_cell + GAP_BMP_TEXT_VERT;
                    new_item->rc_text.bottom = new_item->rc_text.top + GetFontHeight (hdc);

                    new_item->rc_item.bottom = new_item->rc_text.bottom + 1;
                }
            }
            else {
                new_item->rc_item.right = new_item->rc_item.left + ntbar->w_cell + 1;
                new_item->rc_item.bottom = new_item->rc_item.top + ntbar->h_cell + 1;
            }
            break;
                        
        case NTBIF_SEPARATOR:
        default:
            if (ntbar->style & NTBS_DRAWSEPARATOR) {
                new_item->rc_item.bottom = new_item->rc_item.top + WIDTH_SEPARATOR;
            } else {
                new_item->rc_item.bottom = new_item->rc_item.top + WIDTH_SEPARATOR * 2;
            }
            new_item->rc_item.right = new_item->rc_item.left + ntbar->w_cell;
            break;
        }
    }

end:
    ReleaseDC (hdc);

    new_item->next = NULL;
    if (ntbar->tail == NULL) {
        ntbar->head = ntbar->tail = new_item;
        new_item->prev = NULL;
    }
    else {
        ntbar->tail->next = new_item;
        new_item->prev = ntbar->tail;
        ntbar->tail = new_item;
    }
    ntbar->nr_items++;
}



static void recalc_items (HWND hwnd, NTBCTRLDATA* ntb_data)
{
    HDC hdc;
    PNTBITEM item, prev = NULL;

    hdc = GetClientDC (hwnd);

    item = ntb_data->head;

    if(!( ntb_data->style & NTBS_VERTICAL ))
    {
        while (item) {
            //horizontal
            
            if (prev == NULL )
            {
                item->rc_item.top = MARGIN_VERT;
                item->rc_item.left = MARGIN_HORZ;
            }
            else if ( (item->flags & NTBIF_TYPEMASK ) == NTBIF_NEWLINE )
            {
                item->rc_item.top =  prev->rc_item.bottom + GAP_ITEM_ITEM_VERT;
                item->rc_item.left = MARGIN_HORZ;
                item->rc_item.bottom = item->rc_item.top;
                item->rc_item.right = item->rc_item.left;
                goto  end;
            }
            else
            {
                if ( (prev->flags & NTBIF_TYPEMASK) == NTBIF_NEWLINE )
                    item->rc_item.left = MARGIN_HORZ;
                else
                    item->rc_item.left = prev->rc_item.right + GAP_ITEM_ITEM_HORZ;
                item->rc_item.top =  prev->rc_item.top;
            }


            switch (item->flags & NTBIF_TYPEMASK) {
            case NTBIF_PUSHBUTTON:
            case NTBIF_CHECKBUTTON:
            case NTBIF_HOTSPOTBUTTON:
            if (ntb_data->style & NTBS_WITHTEXT) {
                if (ntb_data->style & NTBS_TEXTRIGHT) {

                    SIZE sz_text;
                    GetTextExtent (hdc, item->text, -1, &sz_text);

                    item->rc_item.bottom 
                            = item->rc_item.top + ntb_data->h_cell + 1;

                    item->rc_text.left = item->rc_item.left + ntb_data->w_cell + GAP_BMP_TEXT_HORZ + 1;
                    item->rc_text.right = item->rc_text.left + sz_text.cx;
                    item->rc_text.top = item->rc_item.top;
                    item->rc_text.bottom = item->rc_item.bottom;

                    item->rc_item.right = item->rc_text.right + GAP_BMP_TEXT_HORZ + 1;
                }
                else {
                    item->rc_item.right = item->rc_item.left + ntb_data->w_cell + 1;

                    item->rc_text.left = item->rc_item.left;
                    item->rc_text.right = item->rc_item.right;
                    item->rc_text.top 
                            = item->rc_item.top + ntb_data->h_cell + GAP_BMP_TEXT_VERT;
                    item->rc_text.bottom = item->rc_text.top + GetFontHeight (hdc);

                    item->rc_item.bottom = item->rc_text.bottom + 1;
                }
            }
            else {
                item->rc_item.right = item->rc_item.left + ntb_data->w_cell + 1;
                item->rc_item.bottom = item->rc_item.top + ntb_data->h_cell + 1;
            }
            break;
                        
            case NTBIF_SEPARATOR:
            default:
                if (ntb_data->style & NTBS_DRAWSEPARATOR)
                    item->rc_item.right = item->rc_item.left + WIDTH_SEPARATOR;
                else
                    item->rc_item.right = item->rc_item.left + WIDTH_SEPARATOR * 2;
                item->rc_item.bottom = item->rc_item.top + ntb_data->h_cell;
                break;
            }

            prev = item;
            item = item->next;
        }
    }
    else
    {
     //vertical
        while (item) {
            if (prev == NULL )
            {
                item->rc_item.top = MARGIN_V_VERT;
                item->rc_item.left = MARGIN_V_HORZ;
            }
            else if ( (item->flags & NTBIF_TYPEMASK ) == NTBIF_NEWLINE )
            {
                item->rc_item.left = prev->rc_item.right + GAP_ITEM_ITEM_HORZ;
                item->rc_item.top = MARGIN_V_VERT;
                item->rc_item.bottom = item->rc_item.top;
                item->rc_item.right = item->rc_item.left;
                goto  end;
            }
            else
            {
                if ( (prev->flags & NTBIF_TYPEMASK) == NTBIF_NEWLINE )
                    item->rc_item.top = MARGIN_V_VERT; 
                else
                    item->rc_item.top = prev->rc_item.bottom + GAP_ITEM_ITEM_VERT;
                item->rc_item.left = prev->rc_item.left;
            }

            switch (item->flags & NTBIF_TYPEMASK) {
            case NTBIF_PUSHBUTTON:
            case NTBIF_CHECKBUTTON:
            case NTBIF_HOTSPOTBUTTON:
            if (ntb_data->style & NTBS_WITHTEXT) {
                if (ntb_data->style & NTBS_TEXTRIGHT) {

                    SIZE sz_text;
                    GetTextExtent (hdc, item->text, -1, &sz_text);

                    item->rc_item.bottom 
                            = item->rc_item.top + ntb_data->h_cell + 1;

                    item->rc_text.left = item->rc_item.left + ntb_data->w_cell + GAP_BMP_TEXT_HORZ + 1;
                    item->rc_text.right = item->rc_text.left + sz_text.cx;
                    item->rc_text.top = item->rc_item.top;
                    item->rc_text.bottom = item->rc_item.bottom;

                    item->rc_item.right = item->rc_text.right + GAP_BMP_TEXT_HORZ + 1;
                }
                else {
                    item->rc_item.right = item->rc_item.left + ntb_data->w_cell + 1;

                    item->rc_text.left = item->rc_item.left;
                    item->rc_text.right = item->rc_item.right;
                    item->rc_text.top 
                            = item->rc_item.top + ntb_data->h_cell + GAP_BMP_TEXT_VERT;
                    item->rc_text.bottom = item->rc_text.top + GetFontHeight (hdc);

                    item->rc_item.bottom = item->rc_text.bottom + 1;
                }
            }
            else {
                item->rc_item.right = item->rc_item.left + ntb_data->w_cell + 1;
                item->rc_item.bottom = item->rc_item.top + ntb_data->h_cell + 1;
            }
            break;
                        
            case NTBIF_SEPARATOR:
            default:
                if (ntb_data->style & NTBS_DRAWSEPARATOR)
                    item->rc_item.bottom = item->rc_item.top + WIDTH_SEPARATOR;
                else
                    item->rc_item.bottom = item->rc_item.top + WIDTH_SEPARATOR * 2;
                item->rc_item.right = item->rc_item.left + ntb_data->w_cell;
                break;
            }

            prev = item;
            item = item->next;
        }
    }
end:

    ReleaseDC (hdc);
}


/*
 * 
 * helper functions for NewToolbarCtrlProc () 
 * 
 */

static void bar_size_changing_horz (HWND hwnd, PNTBCTRLDATA ntb_data, const RECT *expect, RECT *result)
{
    result->left = expect->left;
    result->top = expect->top;
    result->right = expect->right;
    if (ntb_data->style & NTBS_MULTLINE) {
        result->bottom = expect->bottom;
    } else {
        if (ntb_data->style & NTBS_WITHTEXT) {
            result->bottom = expect->top + ntb_data->h_cell + MARGIN_VERT * 2;
            if (!(ntb_data->style & NTBS_TEXTRIGHT))
                result->bottom += GetWindowFont (hwnd)->size + GAP_BMP_TEXT_VERT;
        } else {
            result->bottom = expect->top + ntb_data->h_cell + MARGIN_VERT * 2;
        }
    }
}

static void bar_size_changing_vert (PNTBCTRLDATA ntb_data, const RECT *expect, RECT *result)
{
    result->left = expect->left;
    result->top = expect->top;
    result->bottom = expect->bottom;
    if (ntb_data->style & NTBS_WITHTEXT || ntb_data->style & NTBS_MULTLINE)  {
        result->right = expect->right;
    } else {
        result->right = expect->left + ntb_data->w_cell + MARGIN_V_HORZ * 2;
    }
}


static int bar_add_item (HWND hwnd, PNTBCTRLDATA ntb_data, NTBITEMINFO* item_info)
{
    NTBITEM* new_item;
    int text_len;
    if (!(new_item = (NTBITEM*)calloc (1, sizeof (NTBITEM))))
        return NTB_ERR;

    new_item->flags = item_info->flags;
    new_item->id = item_info->id;
    new_item->bmp_cell = item_info->bmp_cell;
    
    if (item_info->text == NULL) {
        new_item->text[0] = '\0';
    } else {
        text_len = strlen (item_info->text);
        strncpy (new_item->text, item_info->text, NTB_TEXT_LEN);
        
        if (text_len > NTB_TEXT_LEN + 1) {
            int i, fit[2 * NTB_TEXT_LEN + 1], pos[2 * NTB_TEXT_LEN + 1], dx[2 * NTB_TEXT_LEN + 1];
            char buff [2 * NTB_TEXT_LEN + 1];
            SIZE size;
            HDC hdc = GetClientDC (hwnd);
            
            memset (fit, 0, sizeof (fit));
            memset (pos, 0, sizeof (pos));
            memset (dx, 0, sizeof (dx));
            
            if (text_len > 2 * NTB_TEXT_LEN) {
                memcpy (buff, item_info->text, NTB_TEXT_LEN * 2);
                buff[2 * NTB_TEXT_LEN] = 0;
            } else {
                memcpy (buff, item_info->text, text_len);
                buff[text_len] = 0;
            }
            GetTextExtentPoint (hdc, buff, strlen (buff), 0xfffffff, fit, pos, dx, &size);
            ReleaseDC (hdc);
            for (i = 0; pos[i] <= NTB_TEXT_LEN; i++) 
                ;
            
            memcpy (new_item->text, buff, pos[i-1]);

            new_item->text[pos[i-1]] = '\0';
            
        }
    }

    if (item_info->tip) {
        strncpy (new_item->tip, item_info->tip, NTB_TIP_LEN);
    } else {
        new_item->tip [0] = '\0';
    }
    
    new_item->rc_hotspot = item_info->rc_hotspot;
    new_item->hotspot_proc = item_info->hotspot_proc;
    new_item->add_data = item_info->add_data;
    
    append_new_item (hwnd, ntb_data, new_item);
    InvalidateRect (hwnd, &new_item->rc_item, TRUE);
    return 0;
}

static int NewToolbarCtrlProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    PCONTROL myself;
    PNTBCTRLDATA ntb_data;

    myself   = Control (hwnd);
    ntb_data = (PNTBCTRLDATA) myself->dwAddData2;

    switch (message) {
        
    case MSG_CREATE: {
        NTBINFO* data = (NTBINFO*)myself->dwAddData;
        
        ntb_data = (NTBCTRLDATA*) calloc (1, sizeof (NTBCTRLDATA));
        if (ntb_data == NULL)
            return -1;
        
        ntb_data->head = NULL;
        ntb_data->style = myself->dwStyle;
        
        ntb_data->image = data->image;
        ntb_data->nr_cells = data->nr_cells;
        ntb_data->nr_cols = data->nr_cols;
        if (data->nr_cols == 0)
            ntb_data->nr_cols = 4;
        
        ntb_data->w_cell = data->w_cell;
        if (data->w_cell == 0)
            ntb_data->w_cell = data->image->bmWidth / ntb_data->nr_cols;
        
        ntb_data->h_cell = data->h_cell;
        if (data->h_cell == 0)
            ntb_data->h_cell = data->image->bmHeight / ntb_data->nr_cells;
        
        ntb_data->nr_items = 0;
        ntb_data->sel_item = NULL;
        
        myself->dwAddData2 = (DWORD)ntb_data;
        break;
    }


      
    case MSG_DESTROY: {
        NTBITEM *item, *tmp;
        
        item = ntb_data->head;
        while (item) {
            tmp = item->next;
            free (item);
            item = tmp;
        }
        
        free (ntb_data);
        break;
    }


        
    case MSG_FONTCHANGED: {
          RECT rc;
          
          recalc_items (hwnd, ntb_data);
          GetWindowRect (hwnd, &rc);
          MoveWindow (hwnd, rc.left, rc.top, RECTW(rc), RECTH(rc), TRUE);
          break;
    }
        
        
        
    case MSG_NCPAINT:
        return 0;

    

    case MSG_SIZECHANGING: 
        
        if (!(ntb_data->style & NTBS_VERTICAL)) {
            /*
             * size changing , style == HORZ
             */
            bar_size_changing_horz (hwnd, ntb_data, (const RECT *)(wParam), (RECT*)(lParam));
        
        } else {
            /*
             * style == Vertical ;
             */
            bar_size_changing_vert (ntb_data, (const RECT *)(wParam), (RECT*)(lParam));
        }
        return 0;
        
    
    
    case NTBM_ADDITEM:
        
        return bar_add_item (hwnd, ntb_data, (NTBITEMINFO*)lParam);
      
        
      
    case NTBM_GETITEM: {

        int id = wParam;
        PNTBITEMINFO item_info = (PNTBITEMINFO) lParam;
        NTBITEM* item = NULL;
        
        if (!item_info)
            return NTB_ERR;
        
        item = ntb_data->head;
        
        while (item) {
            if (id == item->id) {
                if (item_info->which & MTB_WHICH_FLAGS)
                      item_info->flags = item->flags;
                
                if (item_info->which & MTB_WHICH_ID)
                    item_info->id = item->id;
                
                if (item_info->which & MTB_WHICH_CELL)
                    item_info->bmp_cell = item->bmp_cell;
                
                if (item_info->which & MTB_WHICH_HOTSPOT) {
                    item_info->hotspot_proc = item->hotspot_proc;
                    item_info->rc_hotspot = item->rc_hotspot;
                }
                
                if (item_info->which & MTB_WHICH_ADDDATA)
                    item_info->add_data = item->add_data;
                
                if (item_info->which & MTB_WHICH_TEXT)
                    strncpy (item_info->text, item->text, NTB_TEXT_LEN);
                
                if (item_info->which & MTB_WHICH_TIP)
                    strncpy (item_info->tip, item->tip, NTB_TIP_LEN);
                
                return NTB_OKAY;
            }
            item = item->next;
        }
        
        return NTB_ERR;
    }
        
    case NTBM_SETITEM:
      {
          int id = wParam;
          PNTBITEMINFO item_info = (PNTBITEMINFO) lParam;
          NTBITEM* item = NULL;
            
            if (!item_info) {
                return NTB_ERR;
            }
            item = ntb_data->head;
            while (item) {
                if (id == item->id) {

                    if (item_info->which & MTB_WHICH_FLAGS) {
                        item->flags = item_info->flags;
                        InvalidateRect (hwnd, &item->rc_item, TRUE);
                    }

                    if (item_info->which & MTB_WHICH_ID)
                        item->id = item_info->id;

                    if (item_info->which & MTB_WHICH_CELL) {
                        item->bmp_cell = item_info->bmp_cell;
                        InvalidateRect (hwnd, &item->rc_item, TRUE);
                    }

                    if (item_info->which & MTB_WHICH_HOTSPOT) {
                        item->hotspot_proc = item_info->hotspot_proc;
                        item->rc_hotspot = item_info->rc_hotspot;
                    }

                    if (item_info->which & MTB_WHICH_ADDDATA)
                        item->add_data = item_info->add_data;

                    if (item_info->which & MTB_WHICH_TEXT) {
                        strncpy (item->text, item_info->text, NTB_TEXT_LEN);
                        if (ntb_data->style & NTBS_WITHTEXT) {
                            if (ntb_data->style & NTBS_TEXTRIGHT) {
                                recalc_items (hwnd, ntb_data);
                                InvalidateRect (hwnd, NULL, TRUE);
                            }
                            else {
                                InvalidateRect (hwnd, &item->rc_text, TRUE);
                            }
                        }
                    }

                    if (item_info->which & MTB_WHICH_TIP)
                        strncpy (item->tip, item_info->tip, NTB_TIP_LEN);

                    return NTB_OKAY;
                }
                item = item->next;
            }

            return NTB_ERR;
        }

        case NTBM_ENABLEITEM:
        {
            int id = wParam;
            BOOL enable = lParam;
            PNTBITEM item = NULL;

            item = ntb_data->head;
            while (item) {
                if (id == item->id) {
                    if (enable && (item->flags & NTBIF_DISABLED)) {
                        item->flags &= ~NTBIF_DISABLED;
                        InvalidateRect (hwnd, &item->rc_item, TRUE);
                    }
                    else if (!enable && !(item->flags & NTBIF_DISABLED)) {
                        item->flags |= NTBIF_DISABLED;
                        InvalidateRect (hwnd, &item->rc_item, TRUE);
                    }

                    return NTB_OKAY;
                }

                item = item->next;
            }

            return NTB_ERR;
        }
        
        case NTBM_SETBITMAP:
        {
            NTBINFO *ntb_newdata = NULL;

            ntb_newdata = (NTBINFO *) lParam;
            if (!ntb_newdata)
                return NTB_ERR;

            ntb_data->image = ntb_newdata->image;
            ntb_data->nr_cells =ntb_newdata->nr_cells;
            ntb_data->nr_cols = ntb_newdata->nr_cols;
            if (ntb_newdata->nr_cols == 0)
                ntb_data->nr_cols = 4;

            ntb_data->w_cell = ntb_newdata->w_cell;
            if (ntb_data->w_cell == 0)
                ntb_data->w_cell = ntb_newdata->image->bmWidth / ntb_data->nr_cols;

            ntb_data->h_cell = ntb_newdata->h_cell;
            if (ntb_newdata->h_cell == 0)
                ntb_data->h_cell = ntb_newdata->image->bmHeight / ntb_data->nr_cells;
            
            InvalidateRect(hwnd, NULL, TRUE);
            break;
            //return NTB_OKAY;
        }


        case MSG_PAINT: {
            HDC hdc = BeginPaint (hwnd);
            
            if (!(ntb_data->style & NTBS_VERTICAL)) {
                draw_tool_bar_horz (hwnd, hdc, ntb_data);
            } else {
                draw_tool_bar_vert (hwnd, hdc, ntb_data);
            }
            EndPaint (hwnd, hdc);
            return 0;
        }

        case MSG_MOUSEMOVEIN:
        {
            if (!wParam && ntb_data->sel_item) {
                InvalidateRect (hwnd, &ntb_data->sel_item->rc_item, TRUE);
                ntb_data->sel_item = NULL;
            }
            break;
        }

        case MSG_MOUSEMOVE: {
 
            PNTBITEM item;
            int x = LOSWORD(lParam), y = HISWORD(lParam);
            
            if (GetCapture () == hwnd) {
                break;
            }
            item = get_item_by_pos (ntb_data, x, y);
            if (item) {
                if (ntb_data->sel_item != item) {
                    if (ntb_data->sel_item) {
                        InvalidateRect (hwnd, &ntb_data->sel_item->rc_item, TRUE);
                    }
                    ntb_data->sel_item = item;
                    InvalidateRect (hwnd, &ntb_data->sel_item->rc_item, TRUE);
                }
                break;
            } else {
                if (ntb_data->sel_item) {
                    InvalidateRect (hwnd, &ntb_data->sel_item->rc_item, TRUE);
                    ntb_data->sel_item = NULL;
                }
            }
            break;
        }

 
        case MSG_LBUTTONDOWN: {
            int posx, posy;
            NTBITEM* item;

            posx = LOSWORD (lParam);
            posy = HISWORD (lParam);

            if ((item = get_item_by_pos (ntb_data, posx, posy)) == NULL)
                break;
                
            if (GetCapture () == hwnd)
                break;
            SetCapture (hwnd);

            ntb_data->sel_item = item;
            ntb_data->btn_down = TRUE;
            InvalidateRect (hwnd, &item->rc_item, TRUE);
            break;
        }
            

        case MSG_LBUTTONUP: {
            int sx, sy, x, y;
            PNTBITEM item;

            sx = x = LOSWORD(lParam);
            sy = y = HISWORD(lParam);

            ntb_data->btn_down = FALSE;

            if (GetCapture() != hwnd)
                break;
            ReleaseCapture ();

            ScreenToClient (hwnd, &x, &y);

            if ((item = get_item_by_pos (ntb_data, x, y)) 
                            && item == ntb_data->sel_item) {
                if ((item->flags & NTBIF_TYPEMASK) == NTBIF_HOTSPOTBUTTON 
                                && item->hotspot_proc) {

                    RECT rc_hotspot = item->rc_hotspot;
                    OffsetRect (&rc_hotspot, item->rc_item.left, item->rc_item.top);

                    if (PtInRect (&rc_hotspot, x, y)) {
                        RECT rc_item = item->rc_item;
                            
                        ClientToScreen (hwnd, &rc_item.left, &rc_item.top);
                        ClientToScreen (hwnd, &rc_item.right, &rc_item.bottom);

                        item->hotspot_proc (hwnd, item->id, &rc_item, sx, sy);
                        InvalidateRect (hwnd, &item->rc_item, TRUE);
                        break;
                    }
                }

                if ( ( item->flags&NTBIF_TYPEMASK ) == NTBIF_CHECKBUTTON ) {
                    if ( item->flags & NTBIF_CHECKED )
                        item->flags &= ~NTBIF_CHECKED;
                    else
                        item->flags |= NTBIF_CHECKED;
                }
                NotifyParent (hwnd, myself->id, item->id);

                InvalidateRect (hwnd, &item->rc_item, TRUE);
            }
            else if (ntb_data->sel_item) {
                InvalidateRect (hwnd, &ntb_data->sel_item->rc_item, TRUE);
                ntb_data->sel_item = NULL;
            }
            break;
        }

        case MSG_RBUTTONDOWN:
        {
            int posx, posy;
            NTBITEM* item;

            posx = LOSWORD (lParam);
            posy = HISWORD (lParam);


            if ((item = get_item_by_pos (ntb_data, posx, posy)) == NULL)
                break;

            if (GetCapture () == hwnd)
                break;
            SetCapture (hwnd);

            ntb_data->sel_item = item;
            ntb_data->btn_down = TRUE;
            InvalidateRect (hwnd, &item->rc_item, TRUE);
        }
        break;

        case MSG_RBUTTONUP:
        {
            int sx, sy, x, y;
            PNTBITEM item;

            sx = x = LOSWORD(lParam);
            sy = y = HISWORD(lParam);

            ntb_data->btn_down = FALSE;

            if (GetCapture() != hwnd)
                break;
            ReleaseCapture ();

            ScreenToClient (hwnd, &x, &y);

            if ((item = get_item_by_pos (ntb_data, x, y)) 
                            && item == ntb_data->sel_item) {
                if ((item->flags & NTBIF_TYPEMASK) == NTBIF_HOTSPOTBUTTON 
                                && item->hotspot_proc) {

                    RECT rc_hotspot = item->rc_hotspot;
                    OffsetRect (&rc_hotspot, item->rc_item.left, item->rc_item.top);

                    if (PtInRect (&rc_hotspot, x, y)) {
                        RECT rc_item = item->rc_item;
                            
                        ClientToScreen (hwnd, &rc_item.left, &rc_item.top);
                        ClientToScreen (hwnd, &rc_item.right, &rc_item.bottom);

                        item->hotspot_proc (hwnd, item->id, &rc_item, sx, sy);
                        InvalidateRect (hwnd, &item->rc_item, TRUE);
                        break;
                    }
                }

                NotifyParent (hwnd, myself->id, item->id | 0x8000);

                InvalidateRect (hwnd, &item->rc_item, TRUE);
            }
            else if (ntb_data->sel_item) {
                InvalidateRect (hwnd, &ntb_data->sel_item->rc_item, TRUE);
                ntb_data->sel_item = NULL;
            }
            break;
        }
        
        default:
            break;
    }

    return DefaultControlProc (hwnd, message, wParam, lParam);
}

#endif /* _CTRL_NEWTOOLBAR */

