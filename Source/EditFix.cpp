/**
  *  Fix for text controls
  *
  *  Licensed under The GNU Lesser General Public License
  *  Redistributions of files must retain the above copyright notice.
  *
  *  @filesource
  *  @copyright    Copyright (c) 2005-2008 Sijawusz Pur Rahnama
  *  @copyright    Copyright (c) 2003-2005 Kamil 'Ol�rin' Figiela
  *  @link         svn://konnekt.info/kaway2/ kAway2 plugin SVN Repo
  *  @version      $Revision$
  *  @modifiedby   $LastChangedBy$
  *  @lastmodified $Date$
  *  @license      http://creativecommons.org/licenses/LGPL/2.1/
  */

#include "EditFix.h"

/**
 * Usage:
 * - include it & put it after CreateWindow("edit", ...)
 * - set actual window proc via SetProp function like this:
 *     SetProp(hwnd, "oldWndProc", (HANDLE) SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) EditFix));
 */

LRESULT CALLBACK EditFix(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
  switch (iMsg) {
    case WM_CHAR:
      HWND parent = GetParent(hWnd);

      bool controlKey = (bool) HIBYTE(GetKeyState(VK_CONTROL));
      bool altKey = (bool) HIBYTE(GetKeyState(VK_MENU));
      bool shiftKey = (bool) HIBYTE(GetKeyState(VK_SHIFT));

      if (!controlKey || altKey) {
        break;
      }

      if (HIBYTE(GetKeyState(VK_RETURN))) {
        SendMessage(parent, WM_COMMAND, (WPARAM) 1, 0);
      }

      if (HIBYTE(GetKeyState(VkKeyScan('a')))) {
        SendMessage(hWnd, EM_SETSEL, (WPARAM) 0, (LPARAM) -1);
        return 0;
      }

      if (HIBYTE(GetKeyState(VK_BACK))) {
        int length = SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0) + 1;
        char * body_tmp = new char[length];

        GetWindowText(hWnd, body_tmp, length);
        std::string temp = body_tmp, rest;

        int selectionStart, selectionEnd;
        SendMessage(hWnd, EM_GETSEL, (WPARAM) &selectionStart, (LPARAM) &selectionEnd);

        if (selectionStart != selectionEnd) {
          SendMessage(hWnd, EM_REPLACESEL, true, (LPARAM) "");
          return 0;
        }

        rest = temp.substr(selectionStart);
        temp = temp.substr(0, selectionStart);

        int pos = temp.find_last_of(" ", temp.length() - 2);
        int pos2 = temp.find_last_of("\r\n", temp.length() - 1); 
        int pos3 = temp.find_last_of(".", temp.length() - 2);
        // int pos4 = temp.find_last_of(",", temp.length() - 2);

        if (pos < pos2) pos = pos2;
        if (pos < pos3) pos = pos3;
        // if(pos < pos4) pos = pos4;

        if (pos == -1) pos = 0;
        selectionStart = selectionStart - (temp.length() - pos) + 1;

        temp = temp.substr(0, pos + ((pos == pos2) ? -1 : ((pos == 0) ? 0 : 1)));
        temp = temp + rest;
        SetWindowText(hWnd, temp.c_str());

        SendMessage(hWnd, EM_SETSEL, (WPARAM) selectionStart, (LPARAM) selectionStart);
        SendMessage(hWnd, EM_SCROLLCARET, (WPARAM) 0, (LPARAM) 0);

        delete [] body_tmp;
        return 0;
      }
      break;
  }
  return CallWindowProc((WNDPROC) GetProp(hWnd, "oldWndProc"), hWnd, iMsg, wParam, lParam);
}