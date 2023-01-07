// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <utl/span.hh>
#include <utl/ranges.hh>

namespace utl::linker {

struct section_table_entry {
    uint8_t* load_address;
    uint8_t* target_address;
    uint32_t size;
};

struct bss_table_entry {
    uint8_t* address;
    uint32_t size;
};

struct region {
    uint32_t* start;
    uint32_t* end;
    [[nodiscard]] size_t size() const
    {
        auto s = reinterpret_cast<size_t>(start);
        auto e = reinterpret_cast<size_t>(end);
        return e - s;
    }
};

struct stack {
    uint32_t* top;
    uint32_t* bottom;
    size_t size;
};

template <typename T>
struct linker_span {
    T* m_start;
    T* m_end;
    [[nodiscard]] auto begin() const { return m_start; }
    [[nodiscard]] auto end() const { return m_end; }
};

template <typename T>
linker_span(T*,T*) -> linker_span<T>;

namespace detail {

extern "C" {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-identifier"
extern section_table_entry __section_table_start;
extern section_table_entry __section_table_end;

extern bss_table_entry __bss_table_start;
extern bss_table_entry __bss_table_end;

extern uint32_t _heap_start;
extern uint32_t _heap_end;
extern uint32_t _heap_size;

extern uint32_t _stack_bottom;
extern uint32_t _stack_top;
extern uint32_t _stack_size;
#pragma clang diagnostic pop
} //extern C

} //namespace detail

[[gnu::section(".bootstrap")]]
inline constexpr auto section_table()
{
    return linker_span{
        &detail::__section_table_start, 
        &detail::__section_table_end
    };
}

[[gnu::section(".bootstrap")]]
inline constexpr auto bss_table()
{
    return linker_span{
        &detail::__bss_table_start,
        &detail::__bss_table_end
    };
}

[[gnu::section(".bootstrap")]]
inline constexpr auto heap()
{
    return region{
        &detail::_heap_start,
        &detail::_heap_end
    };
}

[[gnu::section(".bootstrap")]]
inline constexpr auto stack()
{
   constexpr auto s = region{
        &detail::_stack_bottom,
        &detail::_stack_top
    };
    return s;
}

[[gnu::section(".bootstrap")]]
inline void copy(section_table_entry const& entry)
{
    utl::span<uint8_t> source{
        entry.load_address,
        entry.size
    };

    utl::span<uint8_t> destination{
        entry.target_address,
        entry.size
    };

    for(auto [idx, load_addr] : utl::ranges::enumerate(source)) {
        destination[idx] = load_addr;
    }
}

[[gnu::section(".bootstrap")]]
inline void zero(bss_table_entry const& entry)
{
    utl::span<uint8_t> zone{
        entry.address,
        entry.size
    };

    for(auto& loc : zone) {
        loc = 0u;
    }
}

[[gnu::section(".bootstrap")]]
inline void bootstrap()
{
    for(auto const& entry : section_table()) copy(entry);
    for(auto const& entry : bss_table()) zero(entry);
}


} //namespace utl::linker
