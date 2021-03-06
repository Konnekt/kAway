/**
  *  kAway2 Away Window class
  *
  *  Licensed under The GNU Lesser General Public License
  *  Redistributions of files must retain the above copyright notice.
  *
  *  @filesource
  *  @copyright    Copyright (c) 2005-2008 Sijawusz Pur Rahnama
  *  @copyright    Copyright (c) 2004 Kuba 'nix' Niegowski
  *  @copyright    Copyright (c) 2003-2004 Kamil 'Ol�rin' Figiela
  *  @link         svn://konnekt.info/kaway2/ kAway2 plugin SVN Repo
  *  @version      $Revision$
  *  @modifiedby   $LastChangedBy$
  *  @lastmodified $Date$
  *  @license      http://creativecommons.org/licenses/LGPL/2.1/
  */

#include "stdafx.h"

#include "AwayWnd.h"
#include "Controller.h"

// dlg ctrl ID
#define STATUS_OK           0x2000
#define STATUS_EDIT         0x2001
#define STATUS_WNDDESC      0x2002
#define STATUS_CHANGE       0x2003
#define STATUS_CHANGE_INFO  0x2004
#define STATUS_EDIT_INFO    0x2005
#define MUTE                0x2006

namespace kAway2 {
  AwayWnd::AwayWnd() {
    WNDCLASSEX awayWnd;
    ZeroMemory(&awayWnd, sizeof(WNDCLASSEX));
    awayWnd.cbSize = sizeof(awayWnd);
    awayWnd.style = CS_HREDRAW | CS_VREDRAW;
    awayWnd.lpfnWndProc = &AwayWnd::wndProc;
    awayWnd.cbClsExtra = 0;
    awayWnd.cbWndExtra = 0;
    awayWnd.hInstance = Ctrl->hInst();
    awayWnd.hCursor = LoadCursor(NULL, IDC_ARROW);
    awayWnd.lpszMenuName = NULL;
    awayWnd.lpszClassName = "kAway2AwayWnd";
    awayWnd.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
    awayWnd.hIcon = LoadIcon(Ctrl->hDll(), MAKEINTRESOURCE(179));
    awayWnd.hIconSm = LoadIcon(Ctrl->hDll(), MAKEINTRESOURCE(179));
    RegisterClassEx(&awayWnd);
  }

  AwayWnd::~AwayWnd() {
    UnregisterClass("kAway2AwayWnd", Ctrl->hInst());
  }

  void AwayWnd::show() {
    if (this->haveInstance(net)) {
      SetActiveWindow((HWND) this->getInstance(net));
    } else {
      HWND hWnd = CreateWindowEx(NULL, "kAway2AwayWnd", "Podaj pow�d nieobecno�ci", WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
        (GetSystemMetrics(SM_CXSCREEN) / 2) - ((300 - (GetSystemMetrics(SM_CXFIXEDFRAME) * 2)) / 2),
        (GetSystemMetrics(SM_CYSCREEN) / 2) - ((100 - (GetSystemMetrics(SM_CYFIXEDFRAME) * 2) + GetSystemMetrics(SM_CYCAPTION)) / 2),
        300 + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,220 + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),
        NULL, NULL, Ctrl->hInst(), (void*) net);
    }
  }

  LRESULT CALLBACK AwayWnd::wndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
    Controller* controller = Singleton<Controller>::getInstance();

    switch (iMsg) {
      case WM_CREATE: {
        SendMessage(hWnd, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) ICMessage(IMI_ICONGET, ico::logoSmall, IML_16));
        SendMessage(hWnd, WM_SETICON, (WPARAM) ICON_SMALL, (LPARAM) ICMessage(IMI_ICONGET, ico::logoSmall, IML_16));

        LPCREATESTRUCT pCreate = (LPCREATESTRUCT) lParam;
        int net = (int) pCreate->lpCreateParams;
        AwayWnd::sWndData *data = new AwayWnd::sWndData(net);
        SetWindowLong(hWnd, GWL_USERDATA, (LONG) data);

        controller->wnd->addInstance(net, hWnd);

        HFONT font = CreateFont(-11, 0, 0, 0, 0, 0, 0, 0, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, 
          CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Tahoma");
        SendMessage(hWnd, WM_SETFONT, (WPARAM) font, true);

        HWND hwndTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
          WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
          CW_USEDEFAULT, CW_USEDEFAULT,
          CW_USEDEFAULT, CW_USEDEFAULT,
          hWnd, NULL, Ctrl->hInst(), NULL);
        SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        RECT rect;
        GetClientRect(hWnd, &rect);
        TOOLINFO ti;
        ZeroMemory(&ti, sizeof(TOOLINFO));
        ti.cbSize = sizeof(TOOLINFO);
        ti.uFlags = TTF_SUBCLASS;       
        ti.hinst = Ctrl->hInst();
        ti.uId = 0;
        ti.rect.left = rect.left;
        ti.rect.top = rect.top;
        ti.rect.right = rect.right;
        ti.rect.bottom = rect.bottom;

        AwayWnd::tStats stats;
        // stats.push_back( AwayWnd::sStatus( ST_ONLINE, "Dost�pny", data->hImlOnline ) );
        // stats.push_back( AwayWnd::sStatus( ST_CHAT, "Pogadam", data->hImlChat ) );
        stats.push_back( AwayWnd::sStatus( ST_AWAY, "Zaraz wracam", data->hImlAway ) );
        stats.push_back( AwayWnd::sStatus( ST_NA, "Nieosi�galny", data->hImlNa ) );
        stats.push_back( AwayWnd::sStatus( ST_DND, "Nie przeszkadza�", data->hImlDnd ) );
        stats.push_back( AwayWnd::sStatus( ST_HIDDEN, "Ukryty", data->hImlInv ) );
        // stats.push_back( AwayWnd::sStatus( ST_OFFLINE, "Niedost�pny", data->hImlOffline ) );

        int count = stats.size();
        int width = count * 40;
        int x = 5 + int(double(300 - width) / 2);

        for (AwayWnd::tStats::iterator it = stats.begin(); it != stats.end(); it++) {
          HWND hWndTmp = CreateWindow("button", "", WS_TABSTOP | WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_ICON,
            x, 30, 40, 20, hWnd, (HMENU) it->id, Ctrl->hInst(), NULL);
          x += 40;
          ti.hwnd = hWndTmp;
          ti.lpszText = (LPSTR) it->name;
          SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM) &ti);
          controller->wnd->prepareButtonImage(it->img, hWnd, net, it->id);
        }
        // int chkSt = IMessage(IM_GET_STATUS, net);
        CheckDlgButton(hWnd, Config::get(cfg::wnd::onEnableSt).to_i(), BST_CHECKED);

        // pole combo - wyb�r opisu
        HWND hWndCombo = CreateWindow("combobox", "", WS_TABSTOP | WS_CHILD | WS_VISIBLE | CBS_AUTOHSCROLL | WS_EX_CONTROLPARENT | 
          WS_EX_NOPARENTNOTIFY | CBS_DROPDOWNLIST, 13, 58, 274, 100, hWnd, (HMENU) STATUS_EDIT, Ctrl->hInst(), NULL);
        SendMessage(hWndCombo, WM_SETFONT, (WPARAM) font, true);
        // SetProp(edit, "oldWndProc", (HANDLE) SetWindowLongPtr(GetDlgItem(hWndCombo, 0x3e9), GWLP_WNDPROC, (LONG_PTR) EditFix));

        // przycisk OK
        HWND hWndTmp = CreateWindowEx(WS_EX_CONTROLPARENT, "button", "OK", BS_DEFPUSHBUTTON | BS_TEXT | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 
          212, 190, 75, 25, hWnd, (HMENU) STATUS_OK, Ctrl->hInst(), NULL);
        SendMessage(hWndTmp, WM_SETFONT, (WPARAM) font, true);

        // napis
        hWndTmp = CreateWindow("static", "Podaj przyczyn� swojej nieobecno�ci", WS_CHILD | WS_VISIBLE | SS_CENTER, 
          13, 8, 274, 15, hWnd, (HMENU) STATUS_WNDDESC, Ctrl->hInst(), NULL);
        SendMessage(hWndTmp, WM_SETFONT, (WPARAM) font, true);

        // checkbox - zmie� status
        hWndTmp = CreateWindow("button", "status", WS_TABSTOP | WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
          13, 193, 50, 20, hWnd, (HMENU) STATUS_CHANGE, Ctrl->hInst(), NULL);      
        SendMessage(hWndTmp, WM_SETFONT, (WPARAM) font, true);

        ti.hwnd = hWndTmp;
        ti.lpszText = "Zmie� status";
        SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM) &ti);
        CheckDlgButton(hWnd, STATUS_CHANGE, Config::get(cfg::wnd::changeOnEnable).to_i() ? BST_CHECKED : 0);

        // checkbox - zmie� opis
        hWndTmp = CreateWindow("button", "opis", WS_TABSTOP | WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
          73, 193, 40, 20, hWnd, (HMENU) STATUS_CHANGE_INFO, Ctrl->hInst(), NULL);      
        SendMessage(hWndTmp, WM_SETFONT, (WPARAM) font, true);

        ti.hwnd = hWndTmp;
        ti.lpszText = "Zmie� opis";
        SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM) &ti);
        CheckDlgButton(hWnd, STATUS_CHANGE_INFO, Config::get(cfg::wnd::changeInfoOnEnable).to_i() ? BST_CHECKED : 0);

        // checkbox - wycisz
        hWndTmp = CreateWindow("button", "wycisz", WS_TABSTOP | WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
          123, 193, 50, 20, hWnd, (HMENU) MUTE, Ctrl->hInst(), NULL);      
        SendMessage(hWndTmp, WM_SETFONT, (WPARAM) font, true);

        ti.hwnd = hWndTmp;
        ti.lpszText = "Wycisz wszystkie d�wi�ki";
        SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM) &ti);
        CheckDlgButton(hWnd, MUTE, Config::get(cfg::wnd::muteOnEnable).to_i() ? BST_CHECKED : 0);

        // odczytujemy liste mru
        MRU::tItems list = controller->mruList->get();

        // wype�niamy combobox
        for (MRU::tItems::iterator it = list.begin(); it != list.end(); it++) {
          SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM) (*it).a_str());
        }

        HWND edit = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", list[0].a_str(), WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_VSCROLL | 
          ES_MULTILINE | ES_WANTRETURN, 13, 88, 274, 100, hWnd, (HMENU) STATUS_EDIT_INFO, Ctrl->hInst(), NULL);
        SendMessage(edit, WM_SETFONT, (WPARAM) font, true);
        SetProp(edit, "oldWndProc", (HANDLE) SetWindowLongPtr(edit, GWLP_WNDPROC, (LONG_PTR) EditFix));

        SetFocus(edit);
        SendMessage(hWndCombo, CB_SETCURSEL, 0, 0);
        break;
      }

      case WM_DESTROY: {
        AwayWnd::sWndData *data = (AwayWnd::sWndData *) GetWindowLong(hWnd, GWL_USERDATA);
        controller->wnd->removeInstance(data->net);

        delete data;
        return 0;
      }

      case WM_COMMAND: {
        switch(LOWORD(wParam)) {
          case 1:
          case STATUS_OK: {
            AwayWnd::sWndData *data = (AwayWnd::sWndData *) GetWindowLong(hWnd, GWL_USERDATA);
            int st, len = SendMessage(GetDlgItem(hWnd, STATUS_EDIT_INFO), WM_GETTEXTLENGTH, 0, 0) + 1;
            char * msg = new char[len];
            
            GetWindowText(GetDlgItem(hWnd, STATUS_EDIT_INFO), msg, len);
            controller->mruList->append(msg);

            if (IsDlgButtonChecked(hWnd, ST_ONLINE)) st = ST_ONLINE;
            if (IsDlgButtonChecked(hWnd, ST_CHAT)) st = ST_CHAT;
            if (IsDlgButtonChecked(hWnd, ST_AWAY)) st = ST_AWAY;
            if (IsDlgButtonChecked(hWnd, ST_NA)) st = ST_NA;
            if (IsDlgButtonChecked(hWnd, ST_DND)) st = ST_DND;
            if (IsDlgButtonChecked(hWnd, ST_HIDDEN)) st = ST_HIDDEN;
            if (IsDlgButtonChecked(hWnd, ST_OFFLINE)) st = ST_OFFLINE;

            if (st) {
              Config::get(cfg::wnd::onEnableSt).set(st);
            }
            Config::get(cfg::wnd::changeOnEnable).set((IsDlgButtonChecked(hWnd, STATUS_CHANGE) == BST_CHECKED) ? 1 : 0);
            Config::get(cfg::wnd::changeInfoOnEnable).set((IsDlgButtonChecked(hWnd, STATUS_CHANGE_INFO) == BST_CHECKED) ? 1 : 0);
            Config::get(cfg::wnd::muteOnEnable).set((IsDlgButtonChecked(hWnd, MUTE) == BST_CHECKED) ? 1 : 0);

            controller->fromWnd(true);
            controller->enable(msg);

            delete [] msg;
            DestroyWindow(hWnd);
            break;
          }

          case 2:
          case SC_CLOSE: {
            DestroyWindow(hWnd);
            break;
          }

          default: {
            if (HIWORD(wParam) == CBN_SELCHANGE) {
              int len = SendMessage(GetDlgItem(hWnd, STATUS_EDIT), WM_GETTEXTLENGTH, 0, 0) + 1;
              char * msg = new char[len];

              GetWindowText(GetDlgItem(hWnd, STATUS_EDIT), msg, len);
              SetWindowText(GetDlgItem(hWnd, STATUS_EDIT_INFO), msg);
              delete [] msg;

              return 1;
            }
            break;
          }
        }
        break;
      }
    }
    return DefWindowProc(hWnd, iMsg, wParam, lParam);
  }

  void AwayWnd::prepareButtonImage(HIMAGELIST &hIml, HWND hWnd, int net, int status) {
    hWnd = GetDlgItem(hWnd, status);

    if (ICMessage(IMC_ISWINXP)) {
      hIml = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 7, 0);
      HICON hIco = (HICON) Ctrl->ICMessage(IMI_ICONGET, UIIcon(IT_STATUS, net, status, 0), IML_ICO2);

      if (ImageList_ReplaceIcon(hIml, -1, hIco) == -1) { // normal
        hIco = (HICON) Ctrl->ICMessage(IMI_ICONGET, UIIcon(IT_STATUS, 0, status, 0), IML_16);
        ImageList_ReplaceIcon(hIml, -1, hIco);
      }

      ImageList_ReplaceIcon(hIml, -1, hIco); // hover
      ImageList_ReplaceIcon(hIml, -1, hIco); // pressed
      ImageList_ReplaceIcon(hIml, -1, hIco); // disabled
      ImageList_ReplaceIcon(hIml, -1, hIco); // focused
      ImageList_ReplaceIcon(hIml, -1, hIco); // focus + hover
      ImageList_ReplaceIcon(hIml, -1, hIco); // pressed + ??
      // DestroyIcon(hIco); // wedlug Konnekt SDK to jest kopia do usuniecia - naprawde nie jest to kopia

      BUTTON_IMAGELIST bil;
      ZeroMemory(&bil, sizeof(BUTTON_IMAGELIST));
      bil.himl = hIml;
      bil.margin.left = 0;
      bil.margin.top = 0;
      bil.margin.right = 0;
      bil.margin.bottom = 0;
      bil.uAlign = BUTTON_IMAGELIST_ALIGN_LEFT;
      Button_SetImageList(hWnd, &bil);
    } else {
      SendMessage(hWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM) Ctrl->ICMessage(IMI_ICONGET, UIIcon(IT_STATUS, net, status, 0), IML_ICO2));
    }
  }
}