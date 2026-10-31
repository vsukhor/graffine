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

/**
 * \file hook_dispatch.cu
 * \brief Compile check of the dispatch of the hooks to their overloads
 * taking the context, and of the detection of the planning hooks, under nvcc.
 * \details nvcc (13.4) evaluated requires-expressions written inline in
 * hooks::on_start and on_end to false when the hook object is a member of
 * a class template in another namespace, called from a class template:
 * the call without the context was then compiled, and failed for hooks
 * taking only the context. The hooks below have no overload without the
 * context, in that arrangement, so that this file compiles only if each
 * overload of hooks::on_start and on_end dispatches correctly. Likewise, a
 * transformation applied with a processor having a planning hook, which
 * vetoes it, is rejected only if the hook is detected (see plan.h).
 */

#include "graffine/transforms/context.h"
#include "graffine/transforms/plan.h"

namespace library {

/// Plan of the transformation below.
template<typename G>
struct Plan {
    int x {};

    template<typename PP>
    static constexpr auto hooks(PP& pp) -> decltype((pp.hooks))
    {
        return pp.hooks;
    }
};

}  // namespace library

namespace client {

struct E {};

template<typename G>
struct Hooks {
    void on_start(E&, const graffine::transforms::Context&) {}
    template<bool B> void on_start(E&, E&, const graffine::transforms::Context&) {}
    template<typename T> void on_start(const T&, const graffine::transforms::Context&) {}
    void on_end(E&, const graffine::transforms::Context&) {}
    template<bool B> void on_end(E&, const graffine::transforms::Context&) {}
    bool plan(const library::Plan<G>& p) { return p.x > 0; }
};

template<typename G>
struct Processor {
    Hooks<G> hooks;
};

}  // namespace client

namespace library {

namespace hooks = graffine::transforms::hooks;

template<typename PP>
struct Transformation {
    using PropertyProcessor = PP;

    PP* pp;

    auto plan(const int x) const -> Plan<int> { return {x}; }
    auto processor() const noexcept -> PP* { return pp; }

    /// Whether the planning hook lets the transformation for \p x proceed.
    auto vet(const int x) const -> bool
    {
        return graffine::transforms::plans::vet(*this, x).has_value();
    }

    void apply()
    {
        client::E e;
        hooks::on_start(pp->hooks, e);
        hooks::on_start<true>(pp->hooks, e, e);
        hooks::on_start<client::E>(pp->hooks, e);
        hooks::on_end(pp->hooks, e);
        hooks::on_end<false>(pp->hooks, e);
    }
};

}  // namespace library

namespace app {

auto run() -> bool
{
    client::Processor<int> p;
    const library::Transformation<client::Processor<int>> t {&p};
    library::Transformation<client::Processor<int>> {&p}.apply();

    return t.vet(1) && !t.vet(0);
}

}  // namespace app

int main()
{
    return app::run() ? 0 : 1;
}
