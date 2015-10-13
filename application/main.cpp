// **********************************************************************************
// The MIT License(MIT)
//
// Copyright(c) 2015 Konstantin Sokolov <konstantin.sokolov.mail@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// **********************************************************************************

// A lot of code in this file originates from here:
// http://everything2.com/title/Win32+system+tray+icon

#include <windows.h>
#include "resource.h"


// constants
const LPSTR HelpAbout = TEXT("TelegramTranslit\n"
                             "by Konstantin Sokolov <konstantin.sokolov.mail@gmail.com>\n\n"
                             "Use CTRL+SHIFT+T to toggle transliteration.\n\n"
                             "Key mapping as on www.translit.ru.");
const LPSTR MyClassName = TEXT("TelegramTranslitWndClass");
const LPSTR Title = TEXT("TelegramTranslit");
const UINT VkHotkey = 0x54; //T
const LPSTR HookDllName = TEXT("hook.dll");

// types
typedef void(*SetHookFuncPtr)(HWND);
typedef void(*RemoveHookFuncPtr)(void);

enum {
    //  Tray icon crap
    ID_TRAYICON = 1,

    APPWM_TRAYICON = WM_APP,
    APPWM_NOP = WM_APP + 1,

    //  Our commands
    ID_ABOUT = 2000,
    ID_EXIT,
    ID_HOTKEY_TOGGLE
};

// global variables
//  Here we keep track of whether we're showing a message box or not.
static bool isModalState = false;
static SetHookFuncPtr SetHook = nullptr;
static RemoveHookFuncPtr RemoveHook = nullptr;
static bool isTranslitActive = false;
static HWND telegramWindow = NULL;

// Prototypes
void AddTrayIcon(HWND hWnd, UINT uID, UINT uCallbackMsg, UINT uIcon, LPSTR pszToolTip);
void RemoveTrayIcon(HWND hWnd, UINT uID);
void ModifyTrayIcon(HWND hWnd, UINT uID, UINT uIcon, LPSTR pszToolTip);
HICON LoadSmallIcon(HINSTANCE hInstance, UINT uID);
BOOL ShowPopupMenu(HWND hWnd, POINT *curpos, int wDefaultItem);
void OnInitMenuPopup(HWND hWnd, HMENU hMenu, UINT uID);
BOOL OnCommand(HWND hWnd, WORD wID, HWND hCtl);
void OnTrayIconMouseMove(HWND hWnd);
void OnTrayIconRBtnUp(HWND hWnd);
void OnTrayIconLBtnDblClick(HWND hWnd);
void OnClose(HWND hWnd);
void RegisterMainWndClass(HINSTANCE hInstance);
void UnregisterMainWndClass(HINSTANCE hInstance);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE prev, LPSTR cmdline, int show)
{
    //  Detect previous instance, and bail if there is one.  
    HWND hPrev = NULL;
    if (hPrev = FindWindow(MyClassName, Title))
        SendMessage( hPrev, WM_CLOSE, 0, 0 );

    telegramWindow = FindWindow(NULL, TEXT("Telegram"));
    if (!telegramWindow)
    {
        MessageBox(NULL, "Cannot find Telegram. Exiting...!", Title,
            MB_ICONINFORMATION | MB_OK | MB_TOPMOST);
        return 1;
    }

    //  We have to have a window, even though we never show it.  This is 
    //  because the tray icon uses window messages to send notifications to 
    //  its owner.  Starting with Windows 2000, you can make some kind of 
    //  "message target" window that just has a message queue and nothing
    //  much else, but we'll be backwardly compatible here.
    RegisterMainWndClass(hInst);

    const HWND hWnd = 
        CreateWindow(MyClassName, Title, 0, 0, 0, 100, 100, NULL, NULL, hInst, NULL);

    if (!hWnd) 
    {
        MessageBox(NULL, "Ack! I can't create the window!", Title,
            MB_ICONERROR | MB_OK | MB_TOPMOST);
        return 1;
    }

    const HMODULE hDll = LoadLibrary(HookDllName);
    SetHook = reinterpret_cast<SetHookFuncPtr>(GetProcAddress(hDll, "SetHook"));
    RemoveHook = reinterpret_cast<RemoveHookFuncPtr>(GetProcAddress(hDll, "RemoveHook"));
    SetHook(telegramWindow);
    isTranslitActive = true;

    RegisterHotKey(hWnd, ID_HOTKEY_TOGGLE, MOD_CONTROL | MOD_SHIFT, VkHotkey);

    MSG  msg;
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterMainWndClass(hInst);

    return msg.wParam;
}

void AddTrayIcon(HWND hWnd, UINT uID, UINT uCallbackMsg, UINT uIcon, LPSTR pszToolTip)
{
    NOTIFYICONDATA  nid;

    memset(&nid, 0, sizeof(nid));

    nid.cbSize = sizeof(nid);
    nid.hWnd = hWnd;
    nid.uID = uID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = uCallbackMsg;
    nid.hIcon = LoadSmallIcon(GetModuleHandle(NULL), uIcon);
    strcpy_s(nid.szTip, 128, pszToolTip);
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void ModifyTrayIcon(HWND hWnd, UINT uID, UINT uIcon, LPSTR pszToolTip)
{
    NOTIFYICONDATA  nid;

    memset(&nid, 0, sizeof(nid));

    nid.cbSize = sizeof(nid);
    nid.hWnd = hWnd;
    nid.uID = uID;

    if(uIcon != (UINT)-1) {
        nid.hIcon = LoadSmallIcon(GetModuleHandle(NULL), uIcon);
        nid.uFlags |= NIF_ICON;
    }

    if(pszToolTip) {
        strcpy_s(nid.szTip, 128, pszToolTip);
        nid.uFlags |= NIF_TIP;
    }

    if(uIcon != (UINT)-1 || pszToolTip)
        Shell_NotifyIcon(NIM_MODIFY, &nid);
}


void RemoveTrayIcon(HWND hWnd, UINT uID)
{
    NOTIFYICONDATA  nid;

    memset(&nid, 0, sizeof(nid));

    nid.cbSize = sizeof(nid);
    nid.hWnd = hWnd;
    nid.uID = uID;

    Shell_NotifyIcon(NIM_DELETE, &nid);
}

static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) 
    {
        case WM_CREATE:
            AddTrayIcon(hWnd, ID_TRAYICON, APPWM_TRAYICON, IDI_TRAY_ICON_ON, Title);
            return 0;

        case APPWM_NOP:
            //  There's a long comment in OnTrayIconRBtnUp() which explains 
            //  what we're doing here.
            return 0;

            //  This is the message which brings tidings of mouse events involving 
            //  our tray icon.  We defined it ourselves.  See AddTrayIcon() for 
            //  details of how we told Windows about it.
        case APPWM_TRAYICON:
            SetForegroundWindow(hWnd);

            switch (lParam) 
            {
                case WM_MOUSEMOVE:
                    OnTrayIconMouseMove(hWnd);
                    return 0;

                case WM_RBUTTONUP:
                    //  There's a long comment in OnTrayIconRBtnUp() which 
                    //  explains what we're doing here.
                    OnTrayIconRBtnUp(hWnd);
                    return 0;

                case WM_LBUTTONDBLCLK:
                    OnTrayIconLBtnDblClick(hWnd);
                    return 0;
            }
        return 0;

    case WM_COMMAND:
        return OnCommand(hWnd, LOWORD(wParam), (HWND)lParam);

    case WM_INITMENUPOPUP:
        OnInitMenuPopup(hWnd, (HMENU)wParam, lParam);
        return 0;

    case WM_HOTKEY:
        if (wParam == ID_HOTKEY_TOGGLE)
        {
            if (isTranslitActive)
                RemoveHook();
            else
                SetHook(telegramWindow);

            isTranslitActive = !isTranslitActive;
            const UINT uIcon = isTranslitActive ? IDI_TRAY_ICON_ON : IDI_TRAY_ICON_OFF;
            ModifyTrayIcon(hWnd, ID_TRAYICON, uIcon, Title);
            return 0;
        }

    case WM_CLOSE:
        OnClose(hWnd);
        return DefWindowProc(hWnd, uMsg, wParam, lParam);

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

void OnClose(HWND hWnd)
{
    RemoveTrayIcon(hWnd, ID_TRAYICON);
    RemoveHook();
    PostQuitMessage(0);
}

BOOL ShowPopupMenu(HWND hWnd, POINT *curpos, int wDefaultItem)
{
    HMENU   hPop = NULL;
    int     i = 0;
    WORD    cmd;
    POINT   pt;

    if (isModalState)
        return FALSE;

    hPop = CreatePopupMenu();

    if (!curpos) {
        GetCursorPos(&pt);
        curpos = &pt;
    }

    InsertMenu(hPop, i++, MF_BYPOSITION | MF_STRING, ID_ABOUT, "About...");
    InsertMenu(hPop, i++, MF_BYPOSITION | MF_STRING, ID_EXIT, "Exit");

    SetMenuDefaultItem(hPop, ID_ABOUT, FALSE);

    SetFocus(hWnd);

    SendMessage(hWnd, WM_INITMENUPOPUP, (WPARAM)hPop, 0);

    cmd = TrackPopupMenu(hPop, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
                         curpos->x, curpos->y, 0, hWnd, NULL);

    SendMessage(hWnd, WM_COMMAND, cmd, 0);

    DestroyMenu(hPop);

    return cmd;
}


BOOL OnCommand(HWND hWnd, WORD wID, HWND hCtl)
{
    if (isModalState)
        return 1;

    //  Have a look at the command and act accordingly
    switch(wID) 
    {
        case ID_ABOUT:
            isModalState = true;
            MessageBox(hWnd, HelpAbout, Title, MB_ICONINFORMATION | MB_OK);
            isModalState = false;
            return 0;

        case ID_EXIT:
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            return 0;
        default:
            return 1;
    }   

}


//  When the mouse pointer drifts over the tray icon, a "tooltip" will be 
//  displayed.  Before that happens, we get notified about the movement, so we 
//  get a chance to set the "tooltip" text to be something useful.  
void OnTrayIconMouseMove(HWND hWnd)
{
    //  stub
}


//  Right-click on tray icon displays menu.
void OnTrayIconRBtnUp(HWND hWnd)
{
    /*
    This SetForegroundWindow() and PostMessage( , APPWM_NOP, , ) are
    recommended in some MSDN sample code; apparently there's a bug in most if
    not all versions of Windows: Tray icon menus don't vanish properly when
    cancelled unless you provide special coddling.

    In MSDN, see: "PRB: Menus for Notification Icons Don't Work Correctly",
    Q135788:

    mk:@ivt:kb/Source/win32sdk/q135788.htm

    Example code:

    mk:@ivt:pdref/good/code/graphics/gdi/setdisp/c3447_7lbt.htm

    Both of these pseudo-URL's are from the July 1998 MSDN.  Those geniuses
    have since completely re-broken MSDN at least once, so the pseudo-URL's
    are useless with more recent MSDN's.

    In the April 2000 MSDN, you can search titles for "Menus for
    Notification Icons"; "Don't" in the title has been changed to "Do Not",
    and searching for either complete title doesn't work anyway.  Good old
    MSDN.
    */

    SetForegroundWindow(hWnd);

    ShowPopupMenu(hWnd, NULL, -1);

    PostMessage(hWnd, APPWM_NOP, 0, 0);
}


void OnTrayIconLBtnDblClick(HWND hWnd)
{
    SendMessage(hWnd, WM_COMMAND, ID_ABOUT, 0);
}


void OnInitMenuPopup(HWND hWnd, HMENU hPop, UINT uID)
{
    //  stub
}


void RegisterMainWndClass(HINSTANCE hInstance)
{
    WNDCLASSEX wclx;
    memset(&wclx, 0, sizeof(wclx));

    wclx.cbSize = sizeof(wclx);
    wclx.style = 0;
    wclx.lpfnWndProc = &WindowProc;
    wclx.cbClsExtra = 0;
    wclx.cbWndExtra = 0;
    wclx.hInstance = hInstance;
    wclx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wclx.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wclx.lpszMenuName = NULL;
    wclx.lpszClassName = MyClassName;

    RegisterClassEx(&wclx);
}

void UnregisterMainWndClass(HINSTANCE hInstance)
{
    UnregisterClass(MyClassName, hInstance);
}


HICON LoadSmallIcon(HINSTANCE hInstance, UINT uID)
{
    return (HICON)LoadImage(hInstance, MAKEINTRESOURCE(uID), IMAGE_ICON,
        16, 16, 0);
}
