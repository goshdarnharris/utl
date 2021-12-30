#pragma once

#include <utility>
#include <stdio.h>
#include <stdint.h>

#include <utl/irq/handler.hh>
#include <utl/irq/unsafe.hh>
#include <utl/irq/safe.hh>
#include <utl/array.hh>
#include <utl/register.hh>

#include <utl/utility.hh>

#include <utl/linker.hh>

namespace utl::irq {

using vector_t = void(void);

template <auto* F>
void _static_vector()
{
    static constexpr auto irq = get_static_handler_irq(F);
    F(irq);
}

template <auto* F, auto& Capture>
void _bound_vector()
{
    // static constexpr auto& bound_capture = Capture;
    utl::apply(F,Capture);
}

extern "C" uint32_t _stack_top;

// template <typename T>
// using _unwrap_isr_safe_t = typename _unwrap_isr_safe<T>::type;

template <size_t N>
struct [[nodiscard]] vector_table {
    // std::array<bare_vector_t,118> m_table;
    uint32_t* m_stack_begin = utl::linker::stack().end;//&_stack_top;
    utl::array<vector_t*,N> m_table;
    static constexpr size_t n_vectors = N;
    //FIXME: need to initialize all indices to the default vector
    //FIXME: this could be a tuple of specific IRQ types. give it
    //a smidge more type safety?
    //FIXME: if this could use a constexpr pointer to the vector
    //table in memory, it could be more constexpr. would need
    //to use the linker symbol trick. not sure how to handle
    //sizing the type of that symbol appropriately. 
    

    template <auto*... Fs>
        requires (any_static_handler<Fs> and ...)
    constexpr vector_table(wrap_static_handler<Fs>... handlers)
    {
        ((m_table[handlers.irq.number] 
            = _static_vector<handlers.handler>),...);
    }

    template <any_irq I, any_isr_safe... Ts>
    using capture_t = utl::tuple<I,_unwrap_isr_safe_t<Ts>...>;

    template <auto* F, any_isr_safe... Ts>
        requires any_bindable_handler<F,_unwrap_isr_safe_t<Ts>...>
    void bind_handler(Ts&&... args) const
    {
        using irq_t = bound_handler_irq_t<F>;  

        static auto capture = capture_t<irq_t,Ts...>{
            irq_t{}, 
            std::forward<_unwrap_isr_safe_t<Ts>>(args)...
        };

        m_table[irq_t::number] = _bound_vector<F,capture>;
    }

    // friend constexpr auto tag_invoke(register_cast_t, auto r, vector_table& v)
    // {
    //     return utl::assign(r.TBLOFF, reinterpret_cast<const uint32_t>(&v));
    // }
};

// template <size_t N>
// constexpr auto make_set_vector_table(registers::any_register auto r, vector_table<N> const& v)
// {
//     return utl::registers::assign(r.TBLOFF, reinterpret_cast<const uint32_t>(&v));
// }

} //namespace utl::irq

// extern constinit utl::irq::vector_table<118> table;
