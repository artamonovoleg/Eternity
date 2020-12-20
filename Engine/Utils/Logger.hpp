//
// Created by artamonovoleg on 20.12.2020.
//

#pragma once

#include <memory>
#include <iostream>
#include <utility>

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

namespace Eternity
{
    class Logger
    {
        private:
            Logger()    = default;
            ~Logger()   = default;
            template <typename Arg, typename... Args>
            static void Print(std::ostream& out, Arg&& arg, Args&&... args)
            {
                out << std::forward<Arg>(arg);
                ((out << ' ' << std::forward<Args>(args)), ...);
                out << "\n";
            }
        public:
            template <typename... Args>
            static void Info(Args&&... args)
            {
                std::cout << WHITE << "[  INFO  ]  ";
                Print(std::cout, args...);
            }

            template <typename... Args>
            static void Warn(Args&&... args)
            {
                std::cout << YELLOW << "[  WARN  ]  ";
                Print(std::cout, args...);
            }

            template <typename... Args>
            static void Error(Args&&... args)
            {
                std::cout << RED << "[  ERROR ]  ";
                Print(std::cout, args...);
            }

            template <typename... Args>
            static void Performance(Args&&... args)
            {
                std::cout << BLUE << "[ PERFORMANCE ]  ";
                Print(std::cout, args...);
            }

            template <typename... Args>
            static void Trace(Args&&... args)
            {
                std::cout << BOLDBLUE << "[  Trace  ]  " << MAGENTA;
                Print(std::cout, args...);
            }
    };
}