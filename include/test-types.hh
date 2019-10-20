#ifndef UTL_TEST_TYPES_HH_
#define UTL_TEST_TYPES_HH_

#include <stdint.h>
#include "utl.hh"

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
    } m_state;

    tracked() : m_state{construction_kind::dfault,0,0} {}

    template <typename... Args>
    tracked(Args... args) : m_state{construction_kind::regular,0,0} {
        utl::maybe_unused(args...);
    }

    ~tracked() = default;

    tracked(tracked const& other) 
        : m_state{construction_kind::copy,other.m_state.copy_assigned,other.m_state.move_assigned} 
    {}

    tracked(tracked&& other)
        : m_state{construction_kind::move,other.m_state.copy_assigned,other.m_state.move_assigned} 
    {}

    tracked& operator=(tracked const& other) {
        m_state = other.m_state;
        m_state.copy_assigned++;
        return *this;
    }

    tracked& operator=(tracked&& other) {
        m_state = other.m_state;
        m_state.move_assigned++;
        return *this;
    }

    bool default_constructed() const {
        return m_state.construction == construction_kind::dfault;
    }

    bool regular_constructed() const {
        return m_state.construction == construction_kind::regular;
    }

    bool copy_constructed() const {
        return m_state.construction == construction_kind::copy;
    }

    bool move_constructed() const {
        return m_state.construction == construction_kind::move;
    }

    uint32_t copy_assigned() const {
        return m_state.copy_assigned;
    }

    uint32_t move_assigned() const {
        return m_state.move_assigned;
    }
};


} //namespace utl

#endif //UTL_TEST_TYPES_HH_
