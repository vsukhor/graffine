#ifndef GRAFFINE_UTILS_LOGGER_H
#define GRAFFINE_UTILS_LOGGER_H

#include "graffine/utils/msgr.h"
#include "graffine/utils/colorcodes.h"

#include <ostream>  // std::cerr
#include <cstdlib>  // std::exit

namespace graffine {

inline thread_local graffine::utils::Msgr* msgr {};  ///< Output message processor.


/// Prints a string to output streams of the Msgr object.
/// The printed string is a formatted aggregation of \p args ending with an
/// optional  \p endline .
template<bool endline=true,
         typename ...Args>
static void jot(Args... args)
{
    if (msgr)
        msgr->print<endline>(args...);
    else
        std::cerr << "Error: msgr is NULL" << std::endl;
}

template<typename ...Args>
static void abort(Args... args)
{
    if (msgr)
        msgr->print(args...);
    else
        std::cerr << "Error: msgr is NULL" << std::endl;

    std::exit(EXIT_FAILURE);
}

template<typename ...Args>
static void warn(Args... args)
{
    jot(graffine::utils::colorcodes::RED, "WARNING: ",
        graffine::utils::colorcodes::RESET, args...);
}

template<typename ...Args>
static void warn_if(const bool condition,
                    Args... args)
{
    if (condition)
        warn(args...);
}

template<typename ...Args>
static void abort_if(const bool condition,
                     Args... args)
{
    if (condition)
        abort(args...);
}

template<typename ...Args>
std::string to_string(Args&& ...args)
{
    const char* separator = "";
    std::ostringstream os;
    (((os << separator << args), separator = ""), ...);

    return os.str();
}

inline auto colored_as(const char* c, const std::string& s) -> std::string
{
    return std::string(c) + s + graffine::utils::colorcodes::RESET;
}

}  // namespace graffine

#endif  // GRAFFINE_UTILS_LOGGER_H