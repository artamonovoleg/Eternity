#pragma once

#include <iostream>

/* FOREGROUND */
// These codes set the actual text to the specified color
#define RESETTEXT  "\x1B[0m" // Set all colors back to normal.
#define FOREBLK  "\x1B[30m" // Black
#define FORERED  "\x1B[31m" // Red
#define FOREGRN  "\x1B[32m" // Green
#define FOREYEL  "\x1B[33m" // Yellow
#define FOREBLU  "\x1B[34m" // Blue
#define FOREMAG  "\x1B[35m" // Magenta
#define FORECYN  "\x1B[36m" // Cyan
#define FOREWHT  "\x1B[37m" // White

// These will set the text color and then set it back to normal afterwards.
#define BLK(x) FOREBLK x RESETTEXT
#define RED(x) FORERED x RESETTEXT
#define GRN(x) FOREGRN x RESETTEXT
#define YEL(x) FOREYEL x RESETTEXT
#define BLU(x) FOREBLU x RESETTEXT
#define MAG(x) FOREMAG x RESETTEXT
#define CYN(x) FORECYN x RESETTEXT
#define WHT(x) FOREWHT x RESETTEXT

namespace Eternity
{
    class Logger
    {
        private:
            template<typename... Args>
            static void Print(Args... args)
            {
                ((std::cout << args << " "), ...);
                std::cout << "\n";
            }
        public:
            template<typename... Args>
            static void Info(Args... args)
            {
                std::cout << "[ Info  ] ";
                Print(args...);
            }

            template<typename... Args>
            static void Trace(Args... args)
            {
                std::cout << "[ Trace ] ";
                Print(args...);
            }

            template<typename... Args>
            static void Warn(Args... args)
            {
                std::cout << "[ " << YEL("Warn") << "  ] ";
                Print(args...);
            }

            template<typename... Args>
            static void Error(Args... args)
            {
                std::cout << "[ " << RED("Error") << " ] ";
                Print(args...);
            }
    };
} // namespace Eternity