/* =============================================================================

Copyright (c) 2021-2026 Valerii Sukhorukov <vsukhorukov@yahoo.com>
All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

================================================================================
*/

#include "graffine/utils/colorcodes.h"
#include "graffine/utils/errors.h"
#include "graffine/utils/logger.h"
#include "graffine/utils/io.h"
#include "graffine/utils/misc.h"
#include "graffine/utils/msgr.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace graffine::tests::utils {

namespace cc = graffine::utils::colorcodes;
namespace io = graffine::utils::io;

using graffine::utils::index_min;
using graffine::utils::remove_vector_element;
using graffine::utils::Msgr;

/// Counts its calls, to tell whether a message argument was evaluated.
struct CallCounter
{
    int calls {};

    auto operator()() -> std::string
    {
        ++calls;
        return "evaluated";
    }
};

/// Message of exception \p E thrown by \p f; fails the test if none is.
template<typename E,
         typename F>
auto message_of(F&& f) -> std::string
{
    try {
        f();
    }
    catch (const E& e) {
        return e.what();
    }
    ADD_FAILURE() << "the expected exception was not thrown";
    return {};
}

/// Temporary file removed at the end of the scope.
struct TempFile
{
    std::filesystem::path path;

    explicit TempFile(const std::string& name)
        : path {std::filesystem::temp_directory_path() / name}
    {}

    ~TempFile()
    {
        std::error_code ec;
        std::filesystem::remove(path, ec);
    }

    void write(const std::vector<std::uint8_t>& bytes) const
    {
        std::ofstream ofs(path, std::ios::binary);
        ofs.write(reinterpret_cast<const char*>(bytes.data()),
                  static_cast<std::streamsize>(bytes.size()));
    }

    auto read() const -> std::string
    {
        std::ifstream ifs(path, std::ios::binary);
        return {std::istreambuf_iterator<char>(ifs),
                std::istreambuf_iterator<char>()};
    }
};

// =============================================================================
// Checks: utils::raise and the GRAFFINE_REQUIRE/ENSURE/ABORT macros.

/// raise() composes the message, the failed condition and the location.
TEST(UtilsRaise, MessageFormat)
{
    const auto loc = std::source_location::current();
    const auto msg = message_of<graffine::precondition_error>([&] {
        graffine::utils::raise<graffine::precondition_error>(
            "x > 0", loc, "x = ", -1, '.');
    });

    EXPECT_EQ(msg, "x = -1. [failed: x > 0] (" + std::string(loc.file_name())
                   + ':' + std::to_string(loc.line()) + ')');
}

/// raise() with no condition text and no message parts.
TEST(UtilsRaise, NoConditionNoMessage)
{
    const auto msg = message_of<graffine::invariant_error>([] {
        graffine::utils::raise<graffine::invariant_error>(
            nullptr, std::source_location::current());
    });

    EXPECT_EQ(msg.find("[failed:"), std::string::npos) << msg;
    EXPECT_EQ(msg.rfind(" (", 0), 0u) << msg;
}

/// GRAFFINE_REQUIRE throws precondition_error naming the failed condition.
TEST(UtilsChecks, RequireThrowsPreconditionError)
{
    const int x = -1;
    const auto msg = message_of<graffine::precondition_error>([&] {
        GRAFFINE_REQUIRE(x > 0, "x is ", x);
    });

    EXPECT_NE(msg.find("x is -1"), std::string::npos) << msg;
    EXPECT_NE(msg.find("[failed: x > 0]"), std::string::npos) << msg;
}

/// GRAFFINE_ENSURE throws invariant_error naming the failed condition.
TEST(UtilsChecks, EnsureThrowsInvariantError)
{
    const int x = 3;
    const auto msg = message_of<graffine::invariant_error>([&] {
        GRAFFINE_ENSURE(x == 4, "x is ", x);
    });

    EXPECT_NE(msg.find("x is 3"), std::string::npos) << msg;
    EXPECT_NE(msg.find("[failed: x == 4]"), std::string::npos) << msg;
}

/// GRAFFINE_ABORT throws unconditionally.
TEST(UtilsChecks, AbortThrowsInvariantError)
{
    const auto msg = message_of<graffine::invariant_error>([] {
        GRAFFINE_ABORT(unreachable, "reached");
    });

    EXPECT_NE(msg.find("reached"), std::string::npos) << msg;
    EXPECT_NE(msg.find("[failed: unreachable]"), std::string::npos) << msg;
}

/// The checks accept a call without message arguments.
TEST(UtilsChecks, NoMessageArguments)
{
    EXPECT_THROW(GRAFFINE_REQUIRE(false), graffine::precondition_error);
    EXPECT_THROW(GRAFFINE_ENSURE(false), graffine::invariant_error);
}

/// A satisfied check neither throws nor evaluates its message arguments.
TEST(UtilsChecks, SatisfiedCheckEvaluatesNothing)
{
    CallCounter f;

    EXPECT_NO_THROW(GRAFFINE_REQUIRE(true, f()));
    EXPECT_NO_THROW(GRAFFINE_ENSURE(true, f()));
    EXPECT_EQ(f.calls, 0);
}

/// A failed check evaluates its message arguments once.
TEST(UtilsChecks, FailedCheckEvaluatesMessageOnce)
{
    CallCounter f;

    const auto msg = message_of<graffine::precondition_error>([&] {
        GRAFFINE_REQUIRE(false, f());
    });

    EXPECT_EQ(f.calls, 1);
    EXPECT_NE(msg.find("evaluated"), std::string::npos) << msg;
}

/// The condition is evaluated once, whether the check succeeds or fails.
TEST(UtilsChecks, ConditionEvaluatedOnce)
{
    int n {};

    GRAFFINE_REQUIRE(++n > 0);
    EXPECT_EQ(n, 1);

    EXPECT_THROW(GRAFFINE_ENSURE(++n < 0), graffine::invariant_error);
    EXPECT_EQ(n, 2);
}

// =============================================================================
// io::read_file_binary.

/// The file content is returned byte by byte, including zero bytes.
TEST(UtilsIO, ReadFileBinary)
{
    const std::vector<std::uint8_t> bytes {0x00, 0xff, 'p', 'l', 'y', 0x0a,
                                           0x00, 0x7f};
    TempFile f {"graffine_utils_read_binary.bin"};
    f.write(bytes);

    const auto res = io::read_file_binary<false>(f.path);

    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(*res, bytes);
}

/// An empty file yields an empty buffer.
TEST(UtilsIO, ReadEmptyFileBinary)
{
    TempFile f {"graffine_utils_read_empty.bin"};
    f.write({});

    const auto res = io::read_file_binary<false>(f.path);

    ASSERT_TRUE(res.has_value());
    EXPECT_TRUE(res->empty());
}

/// A missing file raises io_error naming it.
TEST(UtilsIO, ReadMissingFileBinary)
{
    const auto path = std::filesystem::temp_directory_path() /
                      "graffine_utils_no_such_file.bin";
    std::filesystem::remove(path);

    const auto msg = message_of<graffine::io_error>([&] {
        [[maybe_unused]] auto r = io::read_file_binary<false>(path);
    });

    EXPECT_NE(msg.find(path.string()), std::string::npos) << msg;
}

// =============================================================================
// io::BufferedStream.

/// Stream over bytes "0123456789".
static auto digits_stream() -> io::BufferedStream
{
    const std::string s {"0123456789"};
    return io::BufferedStream {std::vector<std::uint8_t>(s.begin(), s.end())};
}

/// Sequential reading yields the bytes in order, then reports the end.
TEST(UtilsBufferedStream, SequentialRead)
{
    auto bs = digits_stream();

    std::string s;
    bs >> s;

    EXPECT_EQ(s, "0123456789");
    EXPECT_TRUE(bs.eof());
}

/// Binary reads advance the position reported by tellg().
TEST(UtilsBufferedStream, TellgFollowsReads)
{
    auto bs = digits_stream();
    char buf[4] {};

    EXPECT_EQ(bs.tellg(), 0);
    bs.read(buf, 3);
    EXPECT_EQ(std::string(buf, 3), "012");
    EXPECT_EQ(bs.tellg(), 3);
}

/// Absolute positioning, from the beginning.
TEST(UtilsBufferedStream, SeekFromBeginning)
{
    auto bs = digits_stream();

    bs.seekg(7);
    EXPECT_EQ(bs.tellg(), 7);
    EXPECT_EQ(bs.get(), '7');

    bs.seekg(2, std::ios::beg);
    EXPECT_EQ(bs.get(), '2');
}

/// Positioning relative to the current position, forwards and backwards.
TEST(UtilsBufferedStream, SeekFromCurrent)
{
    auto bs = digits_stream();

    bs.seekg(4);
    bs.seekg(3, std::ios::cur);
    EXPECT_EQ(bs.tellg(), 7);
    EXPECT_EQ(bs.get(), '7');

    bs.seekg(-5, std::ios::cur);
    EXPECT_EQ(bs.tellg(), 3);
    EXPECT_EQ(bs.get(), '3');
}

/// Positioning relative to the end: tellg() there gives the size.
TEST(UtilsBufferedStream, SeekFromEnd)
{
    auto bs = digits_stream();

    bs.seekg(0, std::ios::end);
    EXPECT_EQ(bs.tellg(), 10);

    bs.seekg(-2, std::ios::end);
    EXPECT_EQ(bs.tellg(), 8);
    EXPECT_EQ(bs.get(), '8');
}

/// Reading past the end fails, and seeking back recovers after clear().
TEST(UtilsBufferedStream, RecoverAfterEnd)
{
    auto bs = digits_stream();
    char buf[16] {};

    bs.read(buf, sizeof(buf));
    EXPECT_EQ(bs.gcount(), 10);
    EXPECT_TRUE(bs.fail());

    bs.clear();
    bs.seekg(0);
    EXPECT_EQ(bs.get(), '0');
}

// =============================================================================
// index_min and remove_vector_element.

/// Index of the minimal element, wherever it is.
TEST(UtilsMisc, IndexMin)
{
    EXPECT_EQ(index_min(std::vector {5}), 0u);
    EXPECT_EQ(index_min(std::vector {1, 2, 3}), 0u);
    EXPECT_EQ(index_min(std::vector {3, 1, 2}), 1u);
    EXPECT_EQ(index_min(std::vector {3, 2, 1}), 2u);
    EXPECT_EQ(index_min(std::vector {0.5, -1.5, 2.0}), 1u);
}

/// Of several equal minimal elements, the first is chosen.
TEST(UtilsMisc, IndexMinTie)
{
    EXPECT_EQ(index_min(std::vector {2, 1, 3, 1}), 1u);
    EXPECT_EQ(index_min(std::vector {4, 4, 4}), 0u);
}

/// Only the first instance of the element is removed.
TEST(UtilsMisc, RemoveVectorElement)
{
    std::vector v {1, 2, 3, 2, 4};

    remove_vector_element(v, 2);
    EXPECT_EQ(v, (std::vector {1, 3, 2, 4}));

    remove_vector_element(v, 4);
    EXPECT_EQ(v, (std::vector {1, 3, 2}));

    remove_vector_element(v, 1);
    EXPECT_EQ(v, (std::vector {3, 2}));
}

// =============================================================================
// Msgr.

/// oss() concatenates the values without separators.
TEST(UtilsMsgr, Oss)
{
    EXPECT_EQ(Msgr::oss("a", 1, '-', 2.5).str(), "a1-2.5");
    EXPECT_EQ(Msgr::oss().str(), "");
}

/// The screen receives the colour codes, the logfile does not.
TEST(UtilsMsgr, ColourCodesStrippedInLogfileOnly)
{
    TempFile f {"graffine_utils_msgr.log"};
    std::ostringstream screen;
    {
        std::ofstream log(f.path);
        Msgr m {&screen, &log, 6};
        m.print(cc::BOLDRED, "error", cc::RESET, ": ",
                cc::BOLD, "x", cc::RESET, cc::BOLDCYAN, "y", cc::RESET);
    }
    const auto plain = std::string("error: xy \n");
    const auto coloured = Msgr::oss(cc::BOLDRED, "error", cc::RESET, ": ",
                                    cc::BOLD, "x", cc::RESET,
                                    cc::BOLDCYAN, "y", cc::RESET).str()
                        + " \n";

    EXPECT_EQ(f.read(), plain);
    EXPECT_EQ(screen.str(), coloured);
}

/// Every colour code is stripped from the logfile, in any order.
TEST(UtilsMsgr, AllColourCodesStrippedInLogfile)
{
    TempFile f {"graffine_utils_msgr_all.log"};
    {
        std::ofstream log(f.path);
        Msgr m {nullptr, &log, 6};
        std::string s;
        for (auto it = cc::sall.rbegin(); it != cc::sall.rend(); ++it)
            s += *it + "|";
        m.print(s);
    }

    EXPECT_EQ(f.read(), std::string(cc::sall.size(), '|') + " \n");
}

/// print<false> ends no line, so consecutive prints share it.
TEST(UtilsMsgr, PrintWithoutEndline)
{
    std::ostringstream screen;
    Msgr m {&screen, nullptr, 6};

    m.print<false>("a");
    m << "b" << 1;

    EXPECT_EQ(screen.str(), "a b 1 ");
}

}  // namespace graffine::tests::utils
