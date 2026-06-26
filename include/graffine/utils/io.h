/* =============================================================================

 Copyright (C) 2009-2025 Valerii Sukhorukov. All Rights Reserved.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

================================================================================
*/

/**
 * \file io.h
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>.
 */

#ifndef GRAFFINE_UTILS_IO_H
#define GRAFFINE_UTILS_IO_H

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <vector>

/// Input/output
namespace graffine::utils::io {

template<bool beLoud>
auto read_file_binary(const std::filesystem::path& file)
    -> std::optional<std::vector<uint8_t>>
{
    std::ifstream ifs(file, std::ios::binary);

    if (ifs.fail()) {

        throw std::runtime_error("could not open binary ifstream to path " + file.string());
        return std::nullopt;
    }
    else {
        if constexpr (beLoud)
            jot("Reading into binary stream from file: ", file.string());

        ifs.seekg(0, std::ios::end);
        const auto sizeBytes = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        std::vector<uint8_t> fileBufferBytes(sizeBytes);
        ifs.read(reinterpret_cast<char*>(fileBufferBytes.data()), sizeBytes);

        return std::optional<std::vector<uint8_t>>{fileBufferBytes};
    }
}


struct Buffer
    : public std::streambuf
{
    std::vector<uint8_t> b;
    std::size_t size;
    char* p_start {};
    char* p_end {};

    Buffer(std::vector<uint8_t>&& bb)
        : b {std::move(bb)}
        , size {b.size()}
        , p_start {reinterpret_cast<char*>(b.data())}
        , p_end {p_start + size}
    {
        setg(p_start, p_start, p_end);
    }

    pos_type seekoff(const off_type off,
                     std::ios_base::seekdir dir,
                     [[maybe_unused]] std::ios_base::openmode which) override
    {
        if (dir == std::ios_base::cur)
            gbump(static_cast<int>(off));
        else
            setg(p_start,
                 ((dir == std::ios_base::beg) ? p_start : p_end) + off,
                 p_end);

        return gptr() - p_start;
    }

    pos_type seekpos(const pos_type pos,
                     std::ios_base::openmode which) override
    {
        return seekoff(pos, std::ios_base::beg, which);
    }
};

struct BufferedStream
    : virtual Buffer
    , public std::istream
{
    explicit BufferedStream(std::vector<uint8_t>&& b)
        : Buffer {std::move(b)}
        , std::istream(static_cast<std::streambuf*>(this))
    {}
};


}  // namespace graffine::utils::io

#endif  // GRAFFINE_UTILS_IO_H