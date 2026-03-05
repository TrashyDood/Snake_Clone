#include <iostream>
#include <windows.h>

#ifndef CONSOLE_RENDERER
#define CONSOLE_RENDERER

/*  These are color flags. These can be used to give color to the terminal
    Combine them using | 
    FOREGROUND_RED, FOREGROUND_GREEN, FOREGROUND_BLUE, FOREGROUND_INTENSITY
    BACKGROUND_RED, BACKGROUND_GREEN, BACKGROUND_BLUE, BACKGROUND_INTENSITY
    For more information check usage in fillBuffer.
*/

#define DEFAULT_WIDTH  120
#define DEFAULT_HEIGHT 30

typedef unsigned short Color;
inline constexpr COORD defaultCoord = { 0, 0 };

inline int screen_width  = 0;
inline int screen_height = 0;
inline const int buffer_size   = DEFAULT_WIDTH * DEFAULT_HEIGHT;

inline int active_handle = 0;

inline char* char_buffer = nullptr;
inline Color* color_buffer = nullptr;

inline void GetWindowSize(int& pWidth, int& pHeight)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    pWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    pHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

inline void fillBuffer(char c, char colors = 0) {

    for (int i = 0; i < screen_width * screen_height; i++) { 
        char_buffer[i] = c; 
        color_buffer[i] = colors;
    }
}

inline void clearBuffer() { fillBuffer(' ', 0); }

inline void setupCustomConsole() {

    if (active_handle == 0) {

        active_handle = 1;
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

        DWORD dwMode = 0;
        GetConsoleMode(h, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING; 
        //dwMode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
        SetConsoleMode(h, dwMode);

        std::cout << "\033[?25l"; // secret command sent to windows...
        std::cout << "\033[?1049h"; // even more secret...

        char_buffer = new char[buffer_size];
        color_buffer = new unsigned short[buffer_size];

        clearBuffer();

    } else { std::cerr << "Warning - Called setupCustomConsole twice in a row" << std::endl; }
}

inline void deleteCustomConsole() {

    if (active_handle) {

        active_handle = 0;
        FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
        std::cout << "\033[?1049l";
        std::cout << "\033[?25h";
        delete char_buffer;
        delete color_buffer;

    } else { std::cerr << "Warning - Called clearCustomConsole twice in a row" << std::endl; }
}

inline void renderBuffers(COORD pCoord = defaultCoord) {

    if (active_handle) {

        GetWindowSize(screen_width, screen_height);

        DWORD written = 0;
        WriteConsoleOutputCharacterA(GetStdHandle(STD_OUTPUT_HANDLE), char_buffer, buffer_size, pCoord, &written);
        WriteConsoleOutputAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_buffer, buffer_size, pCoord, &written);

    } else { std::cerr << "Warning - Trying to render an uninitialized buffer - Call setupCustomConsole First!" << std::endl; }
}

inline void DrawChar(char pChar, unsigned int pIndex, Color pColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
{
    if (pIndex > buffer_size - 1)
        return;

    if (active_handle)
    {
        char_buffer[pIndex] = pChar;
        color_buffer[pIndex] = pColor;
    }
    else { std::cerr << "Warning - Accessing uninitialized buffer!" << std::endl; }
}

inline void DrawCharCoords(char pChar, unsigned int pX, unsigned int pY, Color pColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
{ 
    DrawChar(pChar, pX + pY * screen_width, pColor);
}

#pragma region input
inline int getIfBasicKeyIsCurrentlyDown(char key) {

    int pressed = GetAsyncKeyState(key) & 0x8000;
    return pressed;
}

// If you want other keys you can look up their code on the interwebs.
inline int getIfUpKeyIsCurrentlyDown() { return GetAsyncKeyState(VK_UP) & 0x8000; }
inline int getIfDownKeyIsCurrentlyDown() { return GetAsyncKeyState(VK_DOWN) & 0x8000; }
inline int getIfLeftKeyIsCurrentlyDown() { return GetAsyncKeyState(VK_LEFT) & 0x8000; }
inline int getIfRightKeyIsCurrentlyDown() { return GetAsyncKeyState(VK_RIGHT) & 0x8000; }
inline int getIfSpaceKeyIsCurrentlyDown() { return GetAsyncKeyState(VK_SPACE) & 0x8000; }
inline int getIfEnterKeyIsCurrentlyDown() { return GetAsyncKeyState(VK_RETURN) & 0x8000; }
inline int getIfBackKeyIsCurrentlyDown() { return GetAsyncKeyState(VK_BACK) & 0x8000; }
inline int getIfEscKeyIsCurrentlyDown() { return GetAsyncKeyState(VK_ESCAPE) & 0x8000; }
#pragma endregion
#endif