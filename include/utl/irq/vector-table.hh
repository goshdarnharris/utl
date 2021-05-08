#pragma once

#include <stdint.h>
#include <concepts>
#include <utl/array.hh>
#include <utl/irq/handler.hh>

namespace utl::irq {

template <typename IRQn_t, size_t N_VECTORS>
class vector_table {
    utl::array<handler_t,N_VECTORS> m_vectors;

    static constexpr int32_t IRQ_EXCEPTION_OFFSET = 16;

    [[nodiscard]] static constexpr size_t vector_index(IRQn_t irqn)
    {
        return static_cast<size_t>(irqn + IRQ_EXCEPTION_OFFSET);
    }
public:
    template <typename... Ts>
    constexpr vector_table(Ts&&... args) : m_vectors{std::forward<Ts>(args)...} 
    {
        static_assert(sizeof...(Ts) == N_VECTORS, "incorrect number of interrupt vectors.");
    }

    [[nodiscard]] uint32_t location() const { return reinterpret_cast<uint32_t>(&m_vectors); }

    [[nodiscard]] constexpr handler_t get_handler(IRQn_t irqn) const
    {
        const size_t idx = vector_index(irqn);
        if(idx >= m_vectors.size()) return nullptr;
        return m_vectors[idx];
    }

    bool set_handler(IRQn_t irqn, handler_t handler)
    {
        const size_t idx = vector_index(irqn);
        if(idx >= m_vectors.size()) return false;
        m_vectors[idx] = handler;
        return true;
    }
};

} //namespace utl::irq
