/**
 * \file logger.h
 * \brief Free functions writing messages through the library's message logger.
 * \details graffine prints its reports through the Msgr object pointed to by
 * \ref graffine::msgr. The client sets the pointer, separately in each
 * thread that reports, e.g.
 * \code
 * graffine::utils::Msgr logger {&std::cout, &logFile, 6};
 * graffine::msgr = &logger;
 * \endcode
 */

#ifndef GRAFFINE_UTILS_LOGGER_H
#define GRAFFINE_UTILS_LOGGER_H

#include "graffine/utils/msgr.h"
#include "graffine/utils/colorcodes.h"

#include <ostream>  // std::cerr

namespace graffine {

/// Message logger used by the library in the current thread; null until
/// set by the client.
inline thread_local graffine::utils::Msgr* msgr {};


/** Prints the concatenation of \p args through \ref msgr.
 * \details If \ref msgr is not set, prints an error note to std::cerr
 * instead.
 * \tparam endline End the output with a line break.
 * \param args Parts of the message.
 */
template<bool endline=true,
         typename ...Args>
static void jot(Args... args)
{
    if (msgr)
        msgr->print<endline>(args...);
    else
        std::cerr << "Error: msgr is NULL" << std::endl;
}

/** Prints the concatenation of \p args as a warning, i.e. after a red
 * "WARNING: " label.
 */
template<typename ...Args>
static void warn(Args... args)
{
    jot(graffine::utils::colorcodes::RED, "WARNING: ",
        graffine::utils::colorcodes::RESET, args...);
}

/** Prints the concatenation of \p args as a warning if \p condition holds.
 */
template<typename ...Args>
static void warn_if(const bool condition,
                    Args... args)
{
    if (condition)
        warn(args...);
}

/** Concatenates \p args, each formatted by its \c operator<<, into a string.
 */
template<typename ...Args>
std::string to_string(Args&& ...args)
{
    const char* separator = "";
    std::ostringstream os;
    (((os << separator << args), separator = ""), ...);

    return os.str();
}

/** Returns \p s preceded by the color code \p c and followed by the reset
 * code, so that it prints in color \p c.
 */
inline auto colored_as(const char* c, const std::string& s) -> std::string
{
    return std::string(c) + s + graffine::utils::colorcodes::RESET;
}

}  // namespace graffine

#endif  // GRAFFINE_UTILS_LOGGER_H