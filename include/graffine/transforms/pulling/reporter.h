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
 * \file reporter.h
 * \brief Contains class logging formatted summary of pulling operation.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_PULLING_REPORTER_H
#define GRAFFINE_TRANSFORMS_PULLING_REPORTER_H

#include "graffine/transforms/pulling/path.h"  // for Log


namespace graffine::transforms::pulling {

/**
 * Class handling printing out formatted summqry at different stages of pulling
 * transformation.
 * \tparam F Functor class for the edge pulling initiated from a vertex of
 *           specific degree.
 */
template<typename F>
struct Reporter
{
    using Path = pulling::Path<typename F::Compt>;

    const F& functor;

    constexpr Reporter(const F& functor)
        : functor {functor}
    {}


    void before(
        const Path& pp,
        const int n
    ) const noexcept
    {
        const auto chs = pp.chains(pp.pthc);
        const auto& schain = cn(chs.back(), pp);
        const auto src_is_engulfed = schain.length() > 1 &&
                                    n == static_cast<int>(schain.length());

        jot(colorcodes::GREEN, functor.fullName, " :: ", colorcodes::YELLOW,
            n, " step", n > 1 ? "s " : " ", colorcodes::RESET, "over path:");
        pp.print_detailed("   ");
        pp.template print_short<true>("");
        jot("");

        for (std::size_t i {}; const auto& w: chs)
            cn(w, pp).print("path ch ", i++, " before: ");

        if (src_is_engulfed) {
            const auto ss = pp.src().opp();  // connected slot of the source chain
            if (chs.size() > 1) {
                const auto next = chs[chs.size() - 2];
                for (const auto& s : cn(ss.w, pp).ngs[ss.e]())
                    if (s.w != next && s.w != ss.w)
                        cn(s.w, pp).print("before src side chain ");
            }
        }
        jot("");
    }


    void after(const Path& pp) const noexcept
    {
        const auto chs = pp.chains(pp.pthc);

        jot("");
        jot(colorcodes::GREEN, functor.shortName, colorcodes::RESET, " producing:");

        for (std::size_t i {}; const auto& w: chs)
            cn(w, pp).print("path ch ", i++, " after ");

        jot("");
    }

    constexpr
    auto cn(
        const ChIdG w,
        const Path& pp
    ) const noexcept -> const Path::Chain&
    {
        return pp.cmp->chain(w);
    }
};

}  // namespace graffine::transforms::pulling

#endif  // GRAFFINE_TRANSFORMS_PULLING_REPORTER_H
