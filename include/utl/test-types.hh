#ifndef UTL_TEST_TYPES_HH_
#define UTL_TEST_TYPES_HH_

#include <stdint.h>
#include "utl/utl.hh"

namespace utl {

struct tracked {
    enum class construction_kind {
        dfault,
        regular,
        copy,
        move
    };

    struct state {
        construction_kind construction;
        uint32_t copy_assigned;
        uint32_t move_assigned;
        uint32_t moved_from;
    };
    state m_state{};

    tracked() : m_state{construction_kind::dfault,0,0,0} {}

    template <typename... Args>
    tracked(Args... args) : m_state{construction_kind::regular,0,0,0} {
        utl::maybe_unused(args...);
    }

    ~tracked() = default;

    tracked(tracked const& other) 
        : m_state{construction_kind::copy,
            other.m_state.copy_assigned,other.m_state.move_assigned,
            other.m_state.moved_from} 
    {}

    tracked(tracked&& other)
        : m_state{construction_kind::move,
            other.m_state.copy_assigned,
            other.m_state.move_assigned,
            other.m_state.moved_from} 
    {
        other.m_state.moved_from++;
    }

    tracked& operator=(tracked const& other) {
        m_state = other.m_state;
        m_state.copy_assigned++;
        return *this;
    }

    tracked& operator=(tracked&& other) {
        m_state = other.m_state;
        other.m_state.moved_from++;
        m_state.move_assigned++;
        return *this;
    }

    [[nodiscard]] bool default_constructed() const {
        return m_state.construction == construction_kind::dfault;
    }

    [[nodiscard]] bool regular_constructed() const {
        return m_state.construction == construction_kind::regular;
    }

    [[nodiscard]] bool copy_constructed() const {
        return m_state.construction == construction_kind::copy;
    }

    [[nodiscard]] bool move_constructed() const {
        return m_state.construction == construction_kind::move;
    }

    [[nodiscard]] uint32_t copy_assigned() const {
        return m_state.copy_assigned;
    }

    [[nodiscard]] uint32_t move_assigned() const {
        return m_state.move_assigned;
    }

    [[nodiscard]] uint32_t moved_from() const {
        return m_state.moved_from;
    }
};


} //namespace utl

#endif //UTL_TEST_TYPES_HH_
