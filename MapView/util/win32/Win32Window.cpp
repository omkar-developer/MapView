//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "win32/Win32Window.h"

Key VirtualKeyCodeToKey(WPARAM key, LPARAM flags)
{
    switch (key)
    {
        // Check the scancode to distinguish between left and right shift
      case VK_SHIFT:
        {
            static unsigned int lShift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
            unsigned int scancode = static_cast<unsigned int>((flags & (0xFF << 16)) >> 16);
            return scancode == lShift ? KEY_LSHIFT : KEY_RSHIFT;
        }

        // Check the "extended" flag to distinguish between left and right alt
      case VK_MENU:       return (HIWORD(flags) & KF_EXTENDED) ? KEY_RALT : KEY_LALT;

        // Check the "extended" flag to distinguish between left and right control
      case VK_CONTROL:    return (HIWORD(flags) & KF_EXTENDED) ? KEY_RCONTROL : KEY_LCONTROL;

        // Other keys are reported properly
      case VK_LWIN:       return KEY_LSYSTEM;
      case VK_RWIN:       return KEY_RSYSTEM;
      case VK_APPS:       return KEY_MENU;
      case VK_OEM_1:      return KEY_SEMICOLON;
      case VK_OEM_2:      return KEY_SLASH;
      case VK_OEM_PLUS:   return KEY_EQUAL;
      case VK_OEM_MINUS:  return KEY_DASH;
      case VK_OEM_4:      return KEY_LBRACKET;
      case VK_OEM_6:      return KEY_RBRACKET;
      case VK_OEM_COMMA:  return KEY_COMMA;
      case VK_OEM_PERIOD: return KEY_PERIOD;
      case VK_OEM_7:      return KEY_QUOTE;
      case VK_OEM_5:      return KEY_BACKSLASH;
      case VK_OEM_3:      return KEY_TILDE;
      case VK_ESCAPE:     return KEY_ESCAPE;
      case VK_SPACE:      return KEY_SPACE;
      case VK_RETURN:     return KEY_RETURN;
      case VK_BACK:       return KEY_BACK;
      case VK_TAB:        return KEY_TAB;
      case VK_PRIOR:      return KEY_PAGEUP;
      case VK_NEXT:       return KEY_PAGEDOWN;
      case VK_END:        return KEY_END;
      case VK_HOME:       return KEY_HOME;
      case VK_INSERT:     return KEY_INSERT;
      case VK_DELETE:     return KEY_DELETE;
      case VK_ADD:        return KEY_ADD;
      case VK_SUBTRACT:   return KEY_SUBTRACT;
      case VK_MULTIPLY:   return KEY_MULTIPLY;
      case VK_DIVIDE:     return KEY_DIVIDE;
      case VK_PAUSE:      return KEY_PAUSE;
      case VK_F1:         return KEY_F1;
      case VK_F2:         return KEY_F2;
      case VK_F3:         return KEY_F3;
      case VK_F4:         return KEY_F4;
      case VK_F5:         return KEY_F5;
      case VK_F6:         return KEY_F6;
      case VK_F7:         return KEY_F7;
      case VK_F8:         return KEY_F8;
      case VK_F9:         return KEY_F9;
      case VK_F10:        return KEY_F10;
      case VK_F11:        return KEY_F11;
      case VK_F12:        return KEY_F12;
      case VK_F13:        return KEY_F13;
      case VK_F14:        return KEY_F14;
      case VK_F15:        return KEY_F15;
      case VK_LEFT:       return KEY_LEFT;
      case VK_RIGHT:      return KEY_RIGHT;
      case VK_UP:         return KEY_UP;
      case VK_DOWN:       return KEY_DOWN;
      case VK_NUMPAD0:    return KEY_NUMPAD0;
      case VK_NUMPAD1:    return KEY_NUMPAD1;
      case VK_NUMPAD2:    return KEY_NUMPAD2;
      case VK_NUMPAD3:    return KEY_NUMPAD3;
      case VK_NUMPAD4:    return KEY_NUMPAD4;
      case VK_NUMPAD5:    return KEY_NUMPAD5;
      case VK_NUMPAD6:    return KEY_NUMPAD6;
      case VK_NUMPAD7:    return KEY_NUMPAD7;
      case VK_NUMPAD8:    return KEY_NUMPAD8;
      case VK_NUMPAD9:    return KEY_NUMPAD9;
      case 'A':           return KEY_A;
      case 'Z':           return KEY_Z;
      case 'E':           return KEY_E;
      case 'R':           return KEY_R;
      case 'T':           return KEY_T;
      case 'Y':           return KEY_Y;
      case 'U':           return KEY_U;
      case 'I':           return KEY_I;
      case 'O':           return KEY_O;
      case 'P':           return KEY_P;
      case 'Q':           return KEY_Q;
      case 'S':           return KEY_S;
      case 'D':           return KEY_D;
      case 'F':           return KEY_F;
      case 'G':           return KEY_G;
      case 'H':           return KEY_H;
      case 'J':           return KEY_J;
      case 'K':           return KEY_K;
      case 'L':           return KEY_L;
      case 'M':           return KEY_M;
      case 'W':           return KEY_W;
      case 'X':           return KEY_X;
      case 'C':           return KEY_C;
      case 'V':           return KEY_V;
      case 'B':           return KEY_B;
      case 'N':           return KEY_N;
      case '0':           return KEY_NUM0;
      case '1':           return KEY_NUM1;
      case '2':           return KEY_NUM2;
      case '3':           return KEY_NUM3;
      case '4':           return KEY_NUM4;
      case '5':           return KEY_NUM5;
      case '6':           return KEY_NUM6;
      case '7':           return KEY_NUM7;
      case '8':           return KEY_NUM8;
      case '9':           return KEY_NUM9;
    }

    return Key(0);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
      case WM_NCCREATE:
        {
            LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT)lParam;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreateStruct->lpCreateParams);
            return DefWindowProcA(hWnd, message, wParam, lParam);
        }
    }

    OSWindow *window = (OSWindow*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (window)
    {
        switch (message)
        {
          case WM_DESTROY:
          case WM_CLOSE:
            {
                Event event;
                event.Type = Event::EVENT_CLOSED;
                window->pushEvent(event);
                break;
            }

          case WM_MOVE:
            {
                RECT winRect;
                GetClientRect(hWnd, &winRect);

                POINT topLeft;
                topLeft.x = winRect.left;
                topLeft.y = winRect.top;
                ClientToScreen(hWnd, &topLeft);

                Event event;
                event.Type        = Event::EVENT_MOVED;
                event.Move.X      = topLeft.x;
                event.Move.Y      = topLeft.y;
                window->pushEvent(event);

                break;
            }

          case WM_SIZE:
            {
                RECT winRect;
                GetClientRect(hWnd, &winRect);

                POINT topLeft;
                topLeft.x = winRect.left;
                topLeft.y = winRect.top;
                ClientToScreen(hWnd, &topLeft);

                POINT botRight;
                botRight.x = winRect.right;
                botRight.y = winRect.bottom;
                ClientToScreen(hWnd, &botRight);

                Event event;
                event.Type        = Event::EVENT_RESIZED;
                event.Size.Width  = botRight.x - topLeft.x;
                event.Size.Height = botRight.y - topLeft.y;
                window->pushEvent(event);

                break;
            }

          case WM_SETFOCUS:
            {
                Event event;
                event.Type = Event::EVENT_GAINED_FOCUS;
                window->pushEvent(event);
                break;
            }

          case WM_KILLFOCUS:
            {
                Event event;
                event.Type = Event::EVENT_LOST_FOCUS;
                window->pushEvent(event);
                break;
            }

          case WM_KEYDOWN:
          case WM_SYSKEYDOWN:
          case WM_KEYUP:
          case WM_SYSKEYUP:
            {
                bool down = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN);

                Event event;
                event.Type        = down ? Event::EVENT_KEY_PRESSED : Event::EVENT_KEY_RELEASED;
                event.Key.Alt     = HIWORD(GetAsyncKeyState(VK_MENU))    != 0;
                event.Key.Control = HIWORD(GetAsyncKeyState(VK_CONTROL)) != 0;
                event.Key.Shift   = HIWORD(GetAsyncKeyState(VK_SHIFT))   != 0;
                event.Key.System  = HIWORD(GetAsyncKeyState(VK_LWIN)) || HIWORD(GetAsyncKeyState(VK_RWIN));
                event.Key.Code    = VirtualKeyCodeToKey(wParam, lParam);
                window->pushEvent(event);

                break;
            }

          case WM_MOUSEWHEEL:
            {
                Event event;
                event.Type = Event::EVENT_MOUSE_WHEEL_MOVED;
                event.MouseWheel.Delta = static_cast<short>(HIWORD(wParam)) / 120;
                window->pushEvent(event);
                break;
            }

          case WM_LBUTTONDOWN:
          case WM_LBUTTONDBLCLK:
            {
                Event event;
                event.Type               = Event::EVENT_MOUSE_BUTTON_PRESSED;
                event.MouseButton.Button = MOUSEBUTTON_LEFT;
                event.MouseButton.X      = static_cast<short>(LOWORD(lParam));
                event.MouseButton.Y      = static_cast<short>(HIWORD(lParam));
                window->pushEvent(event);
                break;
            }

          case WM_LBUTTONUP:
            {
                Event event;
                event.Type               = Event::EVENT_MOUSE_BUTTON_RELEASED;
                event.MouseButton.Button = MOUSEBUTTON_LEFT;
                event.MouseButton.X      = static_cast<short>(LOWORD(lParam));
                event.MouseButton.Y      = static_cast<short>(HIWORD(lParam));
                window->pushEvent(event);
                break;
            }

          case WM_RBUTTONDOWN:
          case WM_RBUTTONDBLCLK:
            {
                Event event;
                event.Type               = Event::EVENT_MOUSE_BUTTON_PRESSED;
                event.MouseButton.Button = MOUSEBUTTON_RIGHT;
                event.MouseButton.X      = static_cast<short>(LOWORD(lParam));
                event.MouseButton.Y      = static_cast<short>(HIWORD(lParam));
                window->pushEvent(event);
                break;
            }

            // Mouse right button up event
          case WM_RBUTTONUP:
            {
                Event event;
                event.Type               = Event::EVENT_MOUSE_BUTTON_RELEASED;
                event.MouseButton.Button = MOUSEBUTTON_RIGHT;
                event.MouseButton.X      = static_cast<short>(LOWORD(lParam));
                event.MouseButton.Y      = static_cast<short>(HIWORD(lParam));
                window->pushEvent(event);
                break;
            }

            // Mouse wheel button down event
          case WM_MBUTTONDOWN:
          case WM_MBUTTONDBLCLK:
            {
                Event event;
                event.Type               = Event::EVENT_MOUSE_BUTTON_PRESSED;
                event.MouseButton.Button = MOUSEBUTTON_MIDDLE;
                event.MouseButton.X      = static_cast<short>(LOWORD(lParam));
                event.MouseButton.Y      = static_cast<short>(HIWORD(lParam));
                window->pushEvent(event);
                break;
            }

            // Mouse wheel button up event
          case WM_MBUTTONUP:
            {
                Event event;
                event.Type               = Event::EVENT_MOUSE_BUTTON_RELEASED;
                event.MouseButton.Button = MOUSEBUTTON_MIDDLE;
                event.MouseButton.X      = static_cast<short>(LOWORD(lParam));
                event.MouseButton.Y      = static_cast<short>(HIWORD(lParam));
                window->pushEvent(event);
                break;
            }

            // Mouse X button down event
          case WM_XBUTTONDOWN:
          case WM_XBUTTONDBLCLK:
            {
                Event event;
                event.Type               = Event::EVENT_MOUSE_BUTTON_PRESSED;
                event.MouseButton.Button = (HIWORD(wParam) == XBUTTON1) ? MOUSEBUTTON_BUTTON4 : MOUSEBUTTON_BUTTON5;
                event.MouseButton.X      = static_cast<short>(LOWORD(lParam));
                event.MouseButton.Y      = static_cast<short>(HIWORD(lParam));
                window->pushEvent(event);
                break;
            }

            // Mouse X button up event
          case WM_XBUTTONUP:
            {
                Event event;
                event.Type               = Event::EVENT_MOUSE_BUTTON_RELEASED;
                event.MouseButton.Button = (HIWORD(wParam) == XBUTTON1) ? MOUSEBUTTON_BUTTON4 : MOUSEBUTTON_BUTTON5;
                event.MouseButton.X      = static_cast<short>(LOWORD(lParam));
                event.MouseButton.Y      = static_cast<short>(HIWORD(lParam));
                window->pushEvent(event);
                break;
            }

          case WM_MOUSEMOVE:
            {
                int mouseX = static_cast<short>(LOWORD(lParam));
                int mouseY = static_cast<short>(HIWORD(lParam));

                Event event;
                event.Type        = Event::EVENT_MOUSE_MOVED;
                event.MouseMove.X = mouseX;
                event.MouseMove.Y = mouseY;
                window->pushEvent(event);
                break;
            }

          case WM_MOUSELEAVE:
            {
                Event event;
                event.Type = Event::EVENT_MOUSE_LEFT;
                window->pushEvent(event);
                break;
            }
        }

    }
    return DefWindowProcA(hWnd, message, wParam, lParam);
}

Win32Window::Win32Window()
    : mNativeWindow(0),
      mParentWindow(0),
      mNativeDisplay(0)
{
}

Win32Window::~Win32Window()
{
    destroy();
}

bool Win32Window::initialize(const std::string &name, size_t width, size_t height)
{
    destroy();

    mParentClassName = name;
    mChildClassName = name + "Child";

    // Work around compile error from not defining "UNICODE" while Chromium does
    const LPSTR idcArrow = MAKEINTRESOURCEA(32512);

    WNDCLASSEXA parentWindowClass = { 0 };
    parentWindowClass.cbSize = sizeof(WNDCLASSEXA);
    parentWindowClass.style = 0;
    parentWindowClass.lpfnWndProc = WndProc;
    parentWindowClass.cbClsExtra = 0;
    parentWindowClass.cbWndExtra = 0;
    parentWindowClass.hInstance = GetModuleHandle(NULL);
    parentWindowClass.hIcon = NULL;
    parentWindowClass.hCursor = LoadCursorA(NULL, idcArrow);
    parentWindowClass.hbrBackground = 0;
    parentWindowClass.lpszMenuName = NULL;
    parentWindowClass.lpszClassName = mParentClassName.c_str();
    if (!RegisterClassExA(&parentWindowClass))
    {
        return false;
    }

    WNDCLASSEXA childWindowClass = { 0 };
    childWindowClass.cbSize = sizeof(WNDCLASSEXA);
    childWindowClass.style = CS_OWNDC;
    childWindowClass.lpfnWndProc = WndProc;
    childWindowClass.cbClsExtra = 0;
    childWindowClass.cbWndExtra = 0;
    childWindowClass.hInstance = GetModuleHandle(NULL);
    childWindowClass.hIcon = NULL;
    childWindowClass.hCursor = LoadCursorA(NULL, idcArrow);
    childWindowClass.hbrBackground = 0;
    childWindowClass.lpszMenuName = NULL;
    childWindowClass.lpszClassName = mChildClassName.c_str();
    if (!RegisterClassExA(&childWindowClass))
    {
        return false;
    }

    DWORD parentStyle = WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;
    DWORD parentExtendedStyle = WS_EX_APPWINDOW;

    RECT sizeRect = { 0, 0, width, height };
    AdjustWindowRectEx(&sizeRect, parentStyle, FALSE, parentExtendedStyle);

    mParentWindow = CreateWindowExA(parentExtendedStyle, mParentClassName.c_str(), name.c_str(), parentStyle, CW_USEDEFAULT, CW_USEDEFAULT,
                                    sizeRect.right - sizeRect.left, sizeRect.bottom - sizeRect.top, NULL, NULL,
                                    GetModuleHandle(NULL), this);

    mNativeWindow = CreateWindowExA(0, mChildClassName.c_str(), name.c_str(), WS_CHILD, 0, 0, width, height,
                                    mParentWindow, NULL, GetModuleHandle(NULL), this);

    mNativeDisplay = GetDC(mNativeWindow);
    if (!mNativeDisplay)
    {
        destroy();
        return false;
    }

    return true;
}

void Win32Window::destroy()
{
    if (mNativeDisplay)
    {
        ReleaseDC(mNativeWindow, mNativeDisplay);
        mNativeDisplay = 0;
    }

    if (mNativeWindow)
    {
        DestroyWindow(mNativeWindow);
        mNativeWindow = 0;
    }

    if (mParentWindow)
    {
        DestroyWindow(mParentWindow);
        mParentWindow = 0;
    }

    UnregisterClassA(mParentClassName.c_str(), NULL);
    UnregisterClassA(mChildClassName.c_str(), NULL);
}

EGLNativeWindowType Win32Window::getNativeWindow() const
{
    return mNativeWindow;
}

EGLNativeDisplayType Win32Window::getNativeDisplay() const
{
    return mNativeDisplay;
}

void Win32Window::messageLoop()
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void Win32Window::setMousePosition(int x, int y)
{
    RECT winRect;
    GetClientRect(mNativeWindow, &winRect);

    POINT topLeft;
    topLeft.x = winRect.left;
    topLeft.y = winRect.top;
    ClientToScreen(mNativeWindow, &topLeft);

    SetCursorPos(topLeft.x + x, topLeft.y + y);
}

OSWindow *CreateOSWindow()
{
    return new Win32Window();
}

bool Win32Window::resize(int width, int height)
{
    if (width == mWidth && height == mHeight)
    {
        return true;
    }

    RECT windowRect;
    if (!GetWindowRect(mParentWindow, &windowRect))
    {
        return false;
    }

    RECT clientRect;
    if (!GetClientRect(mParentWindow, &clientRect))
    {
        return false;
    }

    LONG diffX = (windowRect.right - windowRect.left) - clientRect.right;
    LONG diffY = (windowRect.bottom - windowRect.top) - clientRect.bottom;
    if (!MoveWindow(mParentWindow, windowRect.left, windowRect.top, width + diffX, height + diffY, FALSE))
    {
        return false;
    }

    if (!MoveWindow(mNativeWindow, 0, 0, width, height, FALSE))
    {
        return false;
    }

    return true;
}

void Win32Window::setVisible(bool isVisible)
{
    int flag = (isVisible ? SW_SHOW : SW_HIDE);

    ShowWindow(mParentWindow, flag);
    ShowWindow(mNativeWindow, flag);
}

void Win32Window::pushEvent(Event event)
{
    OSWindow::pushEvent(event);

    switch (event.Type)
    {
      case Event::EVENT_RESIZED:
        MoveWindow(mNativeWindow, 0, 0, mWidth, mHeight, FALSE);
        break;
    }
}
