/* =============================================================================

This file is part of graffine, a lightweight graph transformation library.

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

/**
 * \file context.h
 * \brief Context of the hooks: the composed transformations enclosing them.
 * \details Some transformations are composed of others: e.g. the 2+2 vertex
 * split performs the splits to 1+3 and 1+2 and a vertex merger, and the
 * deletion of an edge together with its host chain splits the junction and
 * deletes the resulting component. The hooks of these inner transformations
 * run between the on_start and on_end hooks of the outer one.
 *
 * A hook learns inside what it runs by accepting a trailing argument
 * <tt>const Context&</tt> after its usual ones, e.g.
 * \code
 * void on_start(const EndSlot& s, const graffine::transforms::Context& ctx);
 * \endcode
 * The transformations pass the context to each hook overload that accepts
 * it, and call the others as before, so that hooks opt in one by one. The
 * context lists the composed transformations under way, from the outermost
 * one: it is empty for a transformation applied by the client, including in
 * the hooks of a composed transformation itself.
 *
 * The context is kept per thread, and only for transformations having a
 * property processor: transformations instantiated with PP void do not
 * maintain it.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_CONTEXT_H
#define GRAFFINE_TRANSFORMS_CONTEXT_H

#include "graffine/transforms/common.h"

#include <array>
#include <cstddef>
#include <span>
#include <type_traits>
#include <utility>

namespace graffine::transforms {

/**
 * Composed transformation under way.
 * \details The degrees identify the transformation within its family:
 * - VertexSplit: degrees of the resulting vertices, as in To<deg1, deg2>;
 * - VertexMerger: degrees of the merged vertices, as in From<deg1, deg2>;
 * - EdgeDeletion: degree of the junction at the connected end of the deleted
 *   single-edge chain, as in DeletingHostChain<deg1> (the host chain is
 *   deleted, i.e. isChainDeleted is true);
 * - EdgeCreation: degree of the source vertex, as in InNewChain<deg1>;
 * - Pulling: degree of the driver vertex, as in On<deg1>;
 * - undefined otherwise.
 */
struct Frame {
    TransformType type {};
    Degree deg1 {undefined<Degree>};
    Degree deg2 {undefined<Degree>};

    friend constexpr auto operator==(const Frame&, const Frame&) -> bool = default;
};


/**
 * Composed transformations enclosing a hook call, from the outermost one.
 * \details Hooks receive it as a trailing argument <tt>const Context&</tt>,
 * if they accept one (see context.h).
 */
class Context {

public:

    /// Maximal nesting depth.
    static constexpr std::size_t capacity {16};

    /// Number of enclosing composed transformations.
    constexpr auto depth() const noexcept -> std::size_t { return n; }

    /// Whether the hook belongs to a transformation applied by the client.
    constexpr auto empty() const noexcept -> bool { return n == 0; }

    /// Enclosing composed transformations, from the outermost one.
    constexpr auto frames() const noexcept -> std::span<const Frame>
    {
        return {ff.data(), n};
    }

    constexpr auto begin() const noexcept { return frames().begin(); }
    constexpr auto end() const noexcept { return frames().end(); }

    /// \p i th enclosing transformation, the outermost one being 0.
    constexpr auto operator[](const std::size_t i) const -> const Frame&
    {
        GRAFFINE_REQUIRE(i < n, "context depth ", n, " <= ", i);

        return ff[i];
    }

    /// The transformation directly enclosing the hook.
    constexpr auto innermost() const -> const Frame& { return (*this)[n - 1]; }

    /// The transformation applied by the client.
    constexpr auto outermost() const -> const Frame& { return (*this)[0]; }

    /// Whether a transformation of type \p t encloses the hook.
    constexpr auto is_inside(const TransformType t) const noexcept -> bool
    {
        for (const auto& f : frames())
            if (f.type == t)
                return true;

        return false;
    }

    /// Whether transformation \p f encloses the hook.
    constexpr auto is_inside(const Frame& f) const noexcept -> bool
    {
        for (const auto& g : frames())
            if (g == f)
                return true;

        return false;
    }

private:

    friend class Nesting;

    std::array<Frame, capacity> ff {};
    std::size_t n {};
};


namespace detail {

/// Context of the hooks running on this thread.
inline thread_local Context context;

}  // namespace detail

/// Context of the hooks running on this thread.
inline auto current_context() noexcept -> const Context&
{
    return detail::context;
}


/// Adds \p f to the context during its lifetime.
class Nesting {

public:

    explicit Nesting(const Frame& f)
    {
        auto& c = detail::context;
        GRAFFINE_REQUIRE(c.n < Context::capacity,
                         "transformations nested deeper than ", Context::capacity);
        c.ff[c.n++] = f;
    }

    ~Nesting() { --detail::context.n; }

    Nesting(const Nesting&) = delete;
    Nesting& operator=(const Nesting&) = delete;
};


/**
 * Calls \p inner, a step of the composed transformation \p f performed by
 * other transformations, in the context of \p f.
 * \details Leaves the context untouched if there is no property processor.
 * \tparam PP Property processor type of the composed transformation.
 */
template<typename PP,
         typename F>
constexpr auto nested(
    const Frame& f,
    F&& inner
) -> decltype(auto)
{
    if constexpr (std::is_void_v<PP>)
        return std::forward<F>(inner)();
    else {
        const Nesting nesting {f};
        return std::forward<F>(inner)();
    }
}


/**
 * Calls of the hooks, passing them the context if they accept it.
 * \details Each function calls hook object \p h with arguments \p a,
 * followed by the current context if \p h has an overload accepting it.
 * The overloads with a template parameter pass it explicitly to the hook,
 * as in h.template on_start<B>(a...). The functions are constrained on the
 * hook being callable, so that the Processor concepts use them to accept
 * hooks either with or without the context.
 */
namespace hooks {

template<typename H,
         typename... A>
    requires requires (H& h, A&&... a) {
                 h.on_start(std::forward<A>(a)..., current_context()); } ||
             requires (H& h, A&&... a) {
                 h.on_start(std::forward<A>(a)...); }
constexpr void on_start(H& h, A&&... a)
{
    if constexpr (requires { h.on_start(std::forward<A>(a)..., current_context()); })
        h.on_start(std::forward<A>(a)..., current_context());
    else
        h.on_start(std::forward<A>(a)...);
}

template<bool B,
         typename H,
         typename... A>
    requires requires (H& h, A&&... a) {
                 h.template on_start<B>(std::forward<A>(a)..., current_context()); } ||
             requires (H& h, A&&... a) {
                 h.template on_start<B>(std::forward<A>(a)...); }
constexpr void on_start(H& h, A&&... a)
{
    if constexpr (requires {
                      h.template on_start<B>(std::forward<A>(a)..., current_context()); })
        h.template on_start<B>(std::forward<A>(a)..., current_context());
    else
        h.template on_start<B>(std::forward<A>(a)...);
}

template<typename T,
         typename H,
         typename... A>
    requires requires (H& h, A&&... a) {
                 h.template on_start<T>(std::forward<A>(a)..., current_context()); } ||
             requires (H& h, A&&... a) {
                 h.template on_start<T>(std::forward<A>(a)...); }
constexpr void on_start(H& h, A&&... a)
{
    if constexpr (requires {
                      h.template on_start<T>(std::forward<A>(a)..., current_context()); })
        h.template on_start<T>(std::forward<A>(a)..., current_context());
    else
        h.template on_start<T>(std::forward<A>(a)...);
}

template<typename H,
         typename... A>
    requires requires (H& h, A&&... a) {
                 h.on_end(std::forward<A>(a)..., current_context()); } ||
             requires (H& h, A&&... a) {
                 h.on_end(std::forward<A>(a)...); }
constexpr void on_end(H& h, A&&... a)
{
    if constexpr (requires { h.on_end(std::forward<A>(a)..., current_context()); })
        h.on_end(std::forward<A>(a)..., current_context());
    else
        h.on_end(std::forward<A>(a)...);
}

template<bool B,
         typename H,
         typename... A>
    requires requires (H& h, A&&... a) {
                 h.template on_end<B>(std::forward<A>(a)..., current_context()); } ||
             requires (H& h, A&&... a) {
                 h.template on_end<B>(std::forward<A>(a)...); }
constexpr void on_end(H& h, A&&... a)
{
    if constexpr (requires {
                      h.template on_end<B>(std::forward<A>(a)..., current_context()); })
        h.template on_end<B>(std::forward<A>(a)..., current_context());
    else
        h.template on_end<B>(std::forward<A>(a)...);
}

}  // namespace hooks
}  // namespace graffine::transforms

#endif  // GRAFFINE_TRANSFORMS_CONTEXT_H
