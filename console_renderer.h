#include <iostream>
#include <windows.h>

#ifndef CONSOLE_RENDERER
#define CONSOLE_RENDERER

/*
    This is the console renderer you will use for week 2 & 3. 

    To use, simply add this file in the same folder as your other file(s), 
    then include it in the .cpp file(s) that need to use its functions. This is done with #include "console_renderer.h".
    That just copy pastes all this code into that cpp file.

    Do not take this as a example of good code.

    It is merely created as a tool & as a learning exercise for you.

    Feel free to modify it.    

    Remember - When including this file, all this will simply be pasted in at the line #include "console_renderer.h".
    That means you will get naming conflicts if you use identical function / variable names in your file(s).

*/

/*  These are color flags. These can be used to give color to the terminal
    Combine them using | 
    FOREGROUND_RED, FOREGROUND_GREEN, FOREGROUND_BLUE, FOREGROUND_INTENSITY
    BACKGROUND_RED, BACKGROUND_GREEN, BACKGROUND_BLUE, BACKGROUND_INTENSITY
    For more information check usage in fillBuffer.
*/

/* The internal coordinate systems assumes 0,0 is the top left corner */ 

/* 
    These define how large the program assumes the terminal is.
    120x30 is the default size of the windows terminal.
    Things will get messed up if you change the size of the terminal (or buffer) while running.
    It is possible to update this program to deal with a terminal that changes size.    
    Reading text input using std::cin >> will cause undefined behaviour while the custom console is running. */

#define DEFAULT_WIDTH  120
#define DEFAULT_HEIGHT 30

inline constexpr COORD defCoord = { 0, 0 };
inline const int screen_width  = DEFAULT_WIDTH;
inline const int screen_height = DEFAULT_HEIGHT;
inline const int buffer_size   = DEFAULT_WIDTH * DEFAULT_HEIGHT;

inline int active_handle = 0;

inline char*           screen_buffer = nullptr;
inline unsigned short* colors_buffer = nullptr;

inline void fillBuffer(char c, char colors) {

    for (int i = 0; i < screen_width * screen_height; i++) { 
        screen_buffer[i] = c; 
        colors_buffer[i] = colors;
    }
}

inline void fillBuffer(char c) { fillBuffer(c,   FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); }
inline void clearBuffer()      { fillBuffer(' ', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); }

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

        screen_buffer = new char[buffer_size];
        colors_buffer = new unsigned short[buffer_size];

        clearBuffer();

    } else { std::cerr << "Warning - Called setupCustomConsole twice in a row" << std::endl; }
}

inline void deleteCustomConsole() {

    if (active_handle) {

        active_handle = 0;
        FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
        std::cout << "\033[?1049l";
        std::cout << "\033[?25h";
        delete screen_buffer;
        delete colors_buffer;

    } else { std::cerr << "Warning - Called clearCustomConsole twice in a row" << std::endl; }
}

// Renders both buffers. Should probably be called renderBuffers...
inline void renderBuffers(COORD pCoord = defCoord) {

    if (active_handle) {

        DWORD written = 0;
        WriteConsoleOutputCharacterA(GetStdHandle(STD_OUTPUT_HANDLE), screen_buffer, buffer_size, pCoord, &written);
        WriteConsoleOutputAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colors_buffer, buffer_size, pCoord, &written);

    } else { std::cerr << "Warning - Trying to render an uninitialized buffer - Call setupCustomConsole First!" << std::endl; }
}

inline void drawTile(unsigned int pIndex, char pSymbol, unsigned short pColors = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE) {
    if (active_handle)
    {
        screen_buffer[pIndex] = pSymbol;
        colors_buffer[pIndex] = pColors;
    }
    else { std::cerr << "Warning - Trying to modify an uninitialized buffer - Call setupCustomConsole First!" << std::endl; }
}

// Memory safe if you input invalid coordinates.
inline void drawTile(unsigned int pX, unsigned int pY, char pSymbol, unsigned short pColors = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE) {
    drawTile(pX + pY * screen_width, pSymbol, pColors);
}

// NOT MEMORY SAFE, this function can overflow the buffer...
inline void drawString(int x, int y, std::string s, unsigned short colors) {

    if (active_handle) {

        for (int i = 0; i < s.length(); i++) {

            screen_buffer[y * screen_width + x + i] = s[i];
            colors_buffer[y * screen_width + x + i] = colors;
        }

    } else { std::cerr << "Warning - Trying to modify an uninitialized buffer - Call setupCustomConsole First!" << std::endl; }

}

inline void drawString(int x, int y, std::string s) { drawString(x, y, s, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); }

#pragma region input
// This works roughly for A-Z and 0-9
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