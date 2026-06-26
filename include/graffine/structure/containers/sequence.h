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
 * \file sequence.h
 * \brief Contains definition of a vector container class.
 * \details This class provides a simple wrapper around std::vector to manage
 * elements of a specific type. It provides basic functionalities such as
 * accessing elements, checking size, and modifying the container.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_CONTAINERS_SEQUENCE_H
#define GRAFFINE_STRUCTURE_CONTAINERS_SEQUENCE_H

#include <deque>
#include <vector>


namespace graffine::structure::containers {

/**
 * Wrapper around an std sequence container.
 * \details This class provides a simple wrapper around an std sequence
 * container to manage elements of a specific type. It provides basic
 * functionalities such as accessing elements, checking size, and modifying
 * the container.
 * \tparam Type Type of elements stored in the container.
 */
 template<typename Type>
struct Sequence
{
    using Container = Type;  // std::vector<Type>;

    using size_type = Container::size_type;
    using value_type = Container::value_type;
    using iterator = Container::iterator;
    using const_iterator = Container::const_iterator;
    using reference = Container::reference;
    using const_reference = Container::const_reference;

    auto operator[](const size_type i) noexcept -> reference;
    auto operator[](const size_type i) const noexcept -> const_reference;

    constexpr auto size() const noexcept;
    constexpr bool empty() const noexcept;
    constexpr void clear() noexcept;

    constexpr auto front() noexcept -> reference;
    constexpr auto front() const noexcept -> const_reference;

    constexpr auto back() noexcept -> reference;
    constexpr auto back() const noexcept -> const_reference;

    constexpr auto first() const noexcept -> const_reference;
    constexpr auto first() noexcept -> reference;

    constexpr auto last() const noexcept -> const_reference;
    constexpr auto last() noexcept -> reference;

    constexpr auto begin() noexcept -> iterator;
    constexpr auto begin() const noexcept -> const_iterator;

    constexpr auto end() noexcept -> iterator;
    constexpr auto end() const noexcept -> const_iterator;

    template<class... Args>
    constexpr auto emplace_back(Args&&... args) -> reference;

    constexpr void push_back(const value_type& value);
    constexpr void push_back(value_type&& value);

    constexpr void pop_back();

    constexpr auto insert(const_iterator pos,
                          const value_type& value) -> iterator;
    constexpr auto insert(const_iterator pos,
                          value_type&& value) -> iterator;
//    constexpr auto insert(const_iterator pos,
//                          Type&& value) -> iterator;
    template<class InputIt>
    constexpr auto insert(const_iterator pos,
                          InputIt first,
                          InputIt last) -> iterator;

    constexpr auto erase(const_iterator pos) -> iterator;
	constexpr auto erase(const_iterator first, const_iterator last) -> iterator;

    constexpr void resize(const size_type s);
    constexpr void resize(const size_type s, const value_type& value);

    constexpr void reserve(const size_type s);

    constexpr auto data() -> value_type*;

    friend
    constexpr bool operator==(const Sequence<Type>& lhs,
                              const Sequence<Type>& rhs)
    {
        return lhs.m_data == rhs.m_data;
    }

protected:

    Container m_data;
};


template<typename Type>
auto Sequence<Type>::
operator[](const size_type i) noexcept -> reference
{
    return m_data[i];
}

template<typename Type>
auto Sequence<Type>::
operator[](const size_type i) const noexcept -> const_reference
{
    return m_data[i];
}

template<typename Type>
constexpr
auto Sequence<Type>::
size() const noexcept
{
    return m_data.size();
}

template<typename Type>
constexpr
bool Sequence<Type>::
empty() const noexcept
{
    return m_data.empty();
}

template<typename Type>
constexpr
void Sequence<Type>::
clear() noexcept
{
    return m_data.clear();
}

template<typename Type>
constexpr
auto Sequence<Type>::
front() noexcept -> reference
{
    return m_data.front();
}

template<typename Type>
constexpr
auto Sequence<Type>::
front() const noexcept -> const_reference
{
    return m_data.front();
}

template<typename Type>
constexpr
auto Sequence<Type>::
back() noexcept -> reference
{
    return m_data.back();
}

template<typename Type>
constexpr
auto Sequence<Type>::
back() const noexcept -> const_reference
{
    return m_data.back();
}

template<typename Type>
constexpr
auto Sequence<Type>::
first() const noexcept -> const_reference
{
    return m_data.front();
}

template<typename Type>
constexpr
auto Sequence<Type>::
first() noexcept -> reference
{
    return m_data.front();
}

template<typename Type>
constexpr
auto Sequence<Type>::
last() const noexcept -> const_reference
{
    return m_data.back();
}

template<typename Type>
constexpr
auto Sequence<Type>::
last() noexcept -> reference
{
    return m_data.back();
}


template<typename Type>
constexpr
auto Sequence<Type>::
begin() noexcept -> iterator
{
    return m_data.begin();
}

template<typename Type>
constexpr
auto Sequence<Type>::
begin() const noexcept -> const_iterator
{
    return m_data.begin();
}

template<typename Type>
constexpr
auto Sequence<Type>::
end() noexcept -> iterator
{
    return m_data.end();
}

template<typename Type>
constexpr
auto Sequence<Type>::
end() const noexcept -> const_iterator
{
    return m_data.end();
}

template<typename Type>
template<class... Args >
constexpr
auto Sequence<Type>::
emplace_back(Args&&... args) -> reference
{
    return m_data.emplace_back(args...);
}

template<typename Type>
constexpr
void Sequence<Type>::
push_back(const value_type& value)
{
    m_data.push_back(value);
}

template<typename Type>
constexpr
void Sequence<Type>::
push_back(value_type&& value)
{
    m_data.push_back(std::move(value));
}

template<typename Type>
constexpr
void Sequence<Type>::
pop_back()
{
    m_data.pop_back();
}

template<typename Type>
constexpr
auto Sequence<Type>::
insert(const_iterator pos, const value_type& value) -> iterator
{
    return m_data.insert(pos, value);
}
template<typename Type>
constexpr
auto Sequence<Type>::
insert(const_iterator pos, value_type&& value) -> iterator
{
    return m_data.insert(pos, value);
}
template<typename Type>
template<class InputIt>
constexpr
auto Sequence<Type>::
insert(const_iterator pos, InputIt first, InputIt last) -> iterator
{
    return m_data.insert(pos, first, last);
}

template<typename Type>
constexpr
auto Sequence<Type>::
erase(const_iterator pos) -> iterator
{
    return m_data.erase(pos);
}

template<typename Type>
constexpr
auto Sequence<Type>::
erase(const_iterator first, const_iterator last) -> iterator
{
    return m_data.erase(first, last);
}

template<typename Type>
constexpr
void Sequence<Type>::
resize(const size_type s)
{
    m_data.resize(s);
}

template<typename Type>
constexpr
void Sequence<Type>::
resize(const size_type s, const value_type& value)
{
    m_data.resize(s, value);
}

template<typename Type>
constexpr
void Sequence<Type>::
reserve(const size_type s)
{
    m_data.reserve(s);
}


template<typename Type>
constexpr
auto Sequence<Type>::
data() -> value_type*
{
    return m_data.data();
}


}  // namespace graffine::structure::containers

#endif  // GRAFFINE_STRUCTURE_CONTAINERS_SEQUENCE_H