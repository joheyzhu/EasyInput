#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <graphics.h>
#include <string>
#include <time.h>

using namespace std;

#pragma comment(lib, "user32.lib")
void EnableDPIAware() {
    typedef BOOL(WINAPI* SETPROCESSDPIAWARENESSCONTEXTPROC)(HANDLE);
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        SETPROCESSDPIAWARENESSCONTEXTPROC pSet = (SETPROCESSDPIAWARENESSCONTEXTPROC)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
        if (pSet) pSet((HANDLE)-4);
    }
}

wstring g_content = L"";
wstring g_tempMsg = L"";
int g_msgTimer = 0;
bool g_isTopMost = false;
IMAGE g_bg;

void SendUnicodeString(const wstring& str) {
    for (size_t i = 0; i < str.length(); i++) {
        wchar_t ch = str[i];
        if (ch == L'\n' || ch == L'\r') {
            if (ch == L'\r' && i + 1 < str.length() && str[i + 1] == L'\n') i++;
            INPUT input = { 0 };
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = VK_RETURN;
            SendInput(1, &input, sizeof(INPUT));
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));
        }
        else {
            INPUT input = { 0 };
            input.type = INPUT_KEYBOARD;
            input.ki.dwFlags = KEYEVENTF_UNICODE;
            input.ki.wScan = ch;
            SendInput(1, &input, sizeof(INPUT));
            input.ki.dwFlags |= KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));
        }
        Sleep(3);
    }
}

wstring GetClipboardText() {
    if (!OpenClipboard(NULL)) return L"";
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData == NULL) { CloseClipboard(); return L""; }
    wchar_t* pText = static_cast<wchar_t*>(GlobalLock(hData));
    wstring text = (pText ? pText : L"");
    GlobalUnlock(hData);
    CloseClipboard();
    return text;
}

void DrawUI(int countdown = 0) {
    if (g_bg.getwidth() > 0) {
        putimage(0, 0, &g_bg);
    }
    else {
        setbkcolor(RGB(30, 30, 40));
        cleardevice();
    }

    setbkmode(TRANSPARENT);

    setfillcolor(WHITE);
    fillroundrect(50, 60, 910, 420, 20, 20);

    RECT textRect = { 80, 90, 880, 390 };

    if (countdown > 0) {
        settextcolor(RGB(220, 20, 60));
        settextstyle(40, 0, L"微软雅黑", 0, 0, FW_BOLD, false, false, false);
        wstring tip = L"准备开始输入...\n请将光标点入目标位置\n剩余时间: " + to_wstring(countdown) + L" 秒";
        drawtext(tip.c_str(), &textRect, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
    }
    else if (!g_tempMsg.empty()) {
        settextcolor(RGB(0, 150, 50));
        settextstyle(35, 0, L"微软雅黑", 0, 0, FW_BOLD, false, false, false);
        drawtext(g_tempMsg.c_str(), &textRect, DT_CENTER | DT_VCENTER);
    }
    else {
        settextcolor(BLACK);
        settextstyle(40, 0, L"微软雅黑");
        if (g_content.empty()) {
            drawtext(L"内容为空\n点击此处粘贴剪贴板文本", &textRect, DT_CENTER | DT_VCENTER);
        }
        else {
            wstring preview = (g_content.length() > 500) ? g_content.substr(0, 500) + L"..." : g_content;
            drawtext(preview.c_str(), &textRect, DT_LEFT | DT_WORDBREAK);
        }
    }

    settextstyle(30, 0, L"微软雅黑", 0, 0, FW_SEMIBOLD, false, false, false);

    setfillcolor(g_isTopMost ? RGB(52, 199, 89) : WHITE);
    fillroundrect(80, 480, 280, 550, 15, 15);
    settextcolor(g_isTopMost ? WHITE : BLACK);
    RECT rb1 = { 80, 480, 280, 550 };
    drawtext(L"窗口置顶", &rb1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    setfillcolor(WHITE);
    fillroundrect(380, 480, 580, 550, 15, 15);
    settextcolor(BLACK);
    RECT rb2 = { 380, 480, 580, 550 };
    drawtext(L"清空重置", &rb2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    setfillcolor(RGB(0, 122, 255));
    fillroundrect(680, 480, 880, 550, 15, 15);
    settextcolor(WHITE);
    RECT rb3 = { 680, 480, 880, 550 };
    drawtext(L"开始模拟输入", &rb3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    settextcolor(WHITE);
    settextstyle(28, 0, L"Arial", 0, 0, FW_BOLD, false, false, false);
    outtextxy(400, 15, L"EasyInput v0.4.2");

    FlushBatchDraw();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    EnableDPIAware();

    HWND hwnd = initgraph(960, 600);
    SetWindowText(hwnd, L"EasyInput v0.4.2");
    loadimage(&g_bg, L"background.jpg", 960, 600);

    BeginBatchDraw();
    ExMessage m;

    while (true) {
        if (!g_tempMsg.empty()) {
            if (clock() - g_msgTimer > 1000) g_tempMsg = L"";
        }

        DrawUI();

        if (peekmessage(&m, EX_MOUSE)) {
            if (m.message == WM_LBUTTONDOWN) {
                if (m.x >= 50 && m.x <= 910 && m.y >= 60 && m.y <= 420) {
                    wstring temp = GetClipboardText();
                    if (!temp.empty()) {
                        g_content = temp;
                        g_tempMsg = L"内容已成功粘贴！";
                        g_msgTimer = clock();
                    }
                }
                else if (m.x >= 80 && m.x <= 280 && m.y >= 480 && m.y <= 550) {
                    g_isTopMost = !g_isTopMost;
                    SetWindowPos(hwnd, g_isTopMost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                }
                else if (m.x >= 380 && m.x <= 580 && m.y >= 480 && m.y <= 550) {
                    g_content = L"";
                    g_tempMsg = L"已重置内容";
                    g_msgTimer = clock();
                }
                else if (m.x >= 680 && m.x <= 880 && m.y >= 480 && m.y <= 550) {
                    if (!g_content.empty()) {
                        for (int i = 5; i > 0; i--) {
                            DrawUI(i);
                            Sleep(1000);
                        }
                        DrawUI();
                        SendUnicodeString(g_content);
                        g_tempMsg = L" 输入任务已完成！";
                        g_msgTimer = clock();
                    }
                }
            }
        }
        Sleep(20);
    }

    EndBatchDraw();
    closegraph();
    return 0;
}