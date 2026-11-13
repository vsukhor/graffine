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
 * \brief Reading a file into memory, and an input stream over the read bytes.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>.
 */

#ifndef GRAFFINE_UTILS_IO_H
#define GRAFFINE_UTILS_IO_H

#include "graffine/utils/errors.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

/// Input/output
namespace graffine::utils::io {

/**
 * \brief Reads the whole content of a file into a byte vector.
 * \tparam beLoud Log the name of the file being read.
 * \param file Path to the file.
 * \return The bytes of the file. The optional is always engaged: failures
 * are reported by exceptions.
 * \throws io_error If the file cannot be opened, its size determined or
 * its content read.
 */
template<bool beLoud>
auto read_file_binary(const std::filesystem::path& file)
    -> std::optional<std::vector<uint8_t>>
{
    std::ifstream ifs(file, std::ios::binary);

    if (ifs.fail())
        throw io_error("failed to open " + file.string());

    if constexpr (beLoud)
        jot("Reading into binary stream from file: ", file.string());

    ifs.seekg(0, std::ios::end);
    const auto sizeBytes = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    if (sizeBytes < 0)
        throw io_error("could not determine the size of " + file.string());

    std::vector<uint8_t> fileBufferBytes(static_cast<std::size_t>(sizeBytes));
    ifs.read(reinterpret_cast<char*>(fileBufferBytes.data()), sizeBytes);
    if (!ifs)
        throw io_error("could not read " + file.string());

    return std::optional<std::vector<uint8_t>> {std::move(fileBufferBytes)};
}


/**
 * \brief Read-only stream buffer owning a block of bytes held in memory.
 * \details Supports repositioning, which is needed by parsers that seek
 * within their input.
 */
struct Buffer
    : public std::streambuf
{
    std::vector<uint8_t> b;  ///< The bytes.
    std::size_t size;        ///< Number of bytes.
    char* p_start {};        ///< Start of the bytes.
    char* p_end {};          ///< One past the end of the bytes.

    /// Constructor taking ownership of the bytes \p bb.
    Buffer(std::vector<uint8_t>&& bb)
        : b {std::move(bb)}
        , size {b.size()}
        , p_start {reinterpret_cast<char*>(b.data())}
        , p_end {p_start + size}
    {
        setg(p_start, p_start, p_end);
    }

    /// Moves the read position by \p off relative to the start, the current
    /// position or the end, as selected by \p dir.
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

    /// Moves the read position to the absolute position \p pos.
    pos_type seekpos(const pos_type pos,
                     std::ios_base::openmode which) override
    {
        return seekoff(pos, std::ios_base::beg, which);
    }
};

/**
 * \brief Input stream reading from bytes held in memory.
 * \details Typically fed with the output of read_file_binary(), so that
 * a file is parsed from memory rather than from disk.
 */
struct BufferedStream
    : virtual Buffer
    , public std::istream
{
    /// Constructor taking ownership of the bytes \p b.
    explicit BufferedStream(std::vector<uint8_t>&& b)
        : Buffer {std::move(b)}
        , std::istream(static_cast<std::streambuf*>(this))
    {}
};


}  // namespace graffine::utils::io

#endif  // GRAFFINE_UTILS_IO_H