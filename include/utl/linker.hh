#pragma once

#include <utl/span.hh>
#include <utl/ranges.hh>

namespace utl::linker {

struct section_table_entry {
    uint32_t load_address;
    uint32_t target_address;
    uint32_t size;
};

struct bss_table_entry {
    uint32_t address;
    uint32_t size;
};

struct region {
    void* start;
    void* end;
    size_t size;
};

struct stack {
    void* top;
    void* bottom;
    size_t size;
};

namespace detail {

extern "C" {
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

}

template <typename T>
utl::span<T> load_linker_span(T* start, T* end) {
    size_t length = (reinterpret_cast<uint32_t>(end)
        - reinterpret_cast<uint32_t>(start))/sizeof(T);
    return {start,length}; 
}

}

inline const auto& section_table()
{
    static const auto table = detail::load_linker_span(
        &detail::__section_table_start,
        &detail::__section_table_end
    );
    return table;
}

inline const auto& bss_table()
{
    static const auto table = detail::load_linker_span(
        &detail::__bss_table_start,
        &detail::__bss_table_end
    );
    return table;
}

inline const auto& heap()
{
    static const auto h = region{
        &detail::_heap_start,
        &detail::_heap_end,
        reinterpret_cast<uint32_t>(&detail::_heap_size)
    };
    return h;
}

inline const auto& stack()
{
    static const auto s = region{
        &detail::_stack_top,
        &detail::_stack_bottom,
        reinterpret_cast<uint32_t>(&detail::_stack_size)
    };
    return s;
}

inline void copy(section_table_entry& entry)
{
    utl::span<uint8_t> source{
        reinterpret_cast<uint8_t*>(entry.load_address),
        entry.size
    };

    utl::span<uint8_t> destination{
        reinterpret_cast<uint8_t*>(entry.target_address),
        entry.size
    };

    for(auto [idx, load_addr] : utl::ranges::enumerate(source)) {
        destination[idx] = load_addr;
    }
}

inline void zero(bss_table_entry& entry)
{
    utl::span<uint8_t> zone{
        reinterpret_cast<uint8_t*>(entry.address),
        entry.size
    };

    for(auto& loc : zone) {
        loc = 0u;
    }
}


} //namespace utl::linker
