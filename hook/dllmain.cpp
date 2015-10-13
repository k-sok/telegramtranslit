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

//standard
#include <Windows.h>

//specific
#include "TelegramTranslitTranslator.h"

using namespace TelegramTranslit;

// forward declarations
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);

// global variables
#pragma data_seg("Shared")
HHOOK hkKey = NULL;
#pragma data_seg()

#pragma comment(linker,"/section:Shared,rws")

static HINSTANCE hInstHookDll = NULL;
static Translator translator;
static HWND myHWND = NULL;


BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        hInstHookDll = static_cast<HINSTANCE>(hModule);
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) void SetHook(HWND hwnd)
{
    myHWND = hwnd;
    const DWORD myThreadId = GetWindowThreadProcessId(myHWND, NULL);
    if (hkKey == NULL)
        hkKey = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, hInstHookDll, myThreadId);
}

extern "C" __declspec(dllexport) void RemoveHook()
{
    if (hkKey != NULL)
        UnhookWindowsHookEx(hkKey);
    hkKey = NULL;
}

void sendBackspace(const HWND hwnd)
{
    SetForegroundWindow(hwnd);
    INPUT ip;
    ZeroMemory(&ip, sizeof(ip));
    ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = VK_BACK;
    ip.ki.wScan = 0;
    ip.ki.dwFlags = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;
    SendInput(1, &ip, sizeof(ip));
    ip.ki.dwFlags |= KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(ip));
}

void sendUnicodeChar(const HWND hwnd, wchar_t c)
{
    SetForegroundWindow(hwnd);
    INPUT ip;
    ZeroMemory(&ip, sizeof(ip));
    ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = 0;
    ip.ki.wScan = static_cast<WORD>(c);
    ip.ki.dwFlags = KEYEVENTF_UNICODE;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;
    SendInput(1, &ip, sizeof(ip));
    ip.ki.dwFlags = KEYEVENTF_UNICODE|KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(ip));
}

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    static int generatedCharsCount = 0;
    static int generatedBackspaceCount = 0;
    if (nCode >= 0 && nCode == HC_ACTION)
    {
        MSG * const msg = reinterpret_cast<MSG*>(lParam);
        if (msg->message == WM_CHAR && msg->wParam < 256)
        {   
            bool doTranslate = true;
            if (generatedCharsCount > 0)
            {
                msg->wParam = 1;
                generatedCharsCount--;
                doTranslate = false;
            }
                
            if (generatedBackspaceCount > 0 && msg->wParam == static_cast<WPARAM>('\b'))
            {
                generatedBackspaceCount--;
                doTranslate = false;
            }

            if (doTranslate)
            {
                Translator::NextAction nextAction = 
                    translator.next(static_cast<char>(msg->wParam));

                if (nextAction.actionType != Translator::ActionType::NoTranslation)
                {
                    generatedCharsCount = 1;
                    if (nextAction.actionType == Translator::ActionType::Replace)
                    {
                        generatedBackspaceCount = 2;
                        sendBackspace(myHWND);
                    }

                    msg->wParam = 1;
                    sendUnicodeChar(myHWND, nextAction.characater);
                    return 1;
                }
            }   
        }       
    }   
    return CallNextHookEx(hkKey, nCode, wParam, lParam);
}
