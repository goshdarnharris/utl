#pragma once

#include <stdint.h>
#include <stddef.h>
#include <concepts>
#include <utl/type-list.hh>
#include <utl/tuple.hh>
#include <numeric>

#include <utl/utility.hh>
#include <bfg/tag_invoke.h>

namespace utl::reg {

namespace fields {

    template <typename R, size_t P, size_t W, bool Rd, bool Wr>
    struct field {
        using this_t = field<R,P,W,Rd,Wr>;
        using register_t = R;
        using register_value_t = typename register_t::value_t;

        static_assert(P + W <= register_t::width_bits, "field extends beyond register width");

        static constexpr size_t position = P;
        static constexpr size_t width = W;
        static constexpr bool readable = Rd;
        static constexpr bool writable = Wr;
        static constexpr register_value_t mask = ((1u << W) - 1u) << P;

        explicit operator register_value_t() const { return *register_t::value; }
    };

    template <typename F>
    using value_t = typename F::register_value_t;

    template <typename F>
    constexpr value_t<F> align_to_field(const value_t<F> v)
    {
        return (v << F::position) & F::mask;
    }

    template <typename F>
    constexpr value_t<F> clear_field(const value_t<F> v)
    {
        return ~F::mask & v;
    }

    template <typename R, size_t P, size_t W>
    struct read_only : field<R,P,W,true,false> {};

    template <typename R, size_t P, size_t W>
    struct write_only : field<R,P,W,false,true> {};

    template <typename R, size_t P, size_t W>
    struct read_write : field<R,P,W,true,true> {};


    template <typename F>
    static constexpr bool is_writable = F::writable;

    template <typename F>
    static constexpr bool is_readable = F::readable;

    template <typename F>
    static constexpr bool is_cleared_on_read = false;

    template <typename F>
    static constexpr bool is_set_on_read = false;

    // constexpr auto get_register_value()
}

namespace convert {
    //define conversion customization points here
}

namespace ops {

    BFG_TAG_INVOKE_DEF(apply);

    template <typename F>
    struct field_op {
        using field_t = F;
        using register_t = typename F::register_t;
        using value_t = typename register_t::value_t;
    };

    template <typename R>
    struct register_op {
        using register_t = R;
        using value_t = typename register_t::value_t;
    };

    template <typename F>
    struct assign : field_op<F> {
        typename field_op<F>::value_t value;
        friend constexpr auto tag_invoke(apply_t, assign op, const fields::value_t<F> in)
        {
            const auto cleared = fields::clear_field<F>(in);
            const auto aligned = fields::align_to_field<F>(op.value);
            
            return cleared | aligned;
        }
    };

    // template <typename R>
    // struct assign_register : register_op<R> {
    //     typename register_op<R>::value_t value;
    //     friend constexpr auto tag_invoke(apply_t, assign op, const R& in)
    //     {

    //     }
    // }

    template <typename F>
    struct set : field_op<F> {
        typename field_op<F>::value_t mask;
        friend constexpr auto tag_invoke(apply_t, set op, const fields::value_t<F> in)
        {
            return in | fields::align_to_field<F>(op.mask);
        }
    };

    template <typename F>
    struct clear : field_op<F> {
        typename field_op<F>::value_t mask;
        friend constexpr auto tag_invoke(apply_t, clear op, const fields::value_t<F> in)
        {
            return in & ~fields::align_to_field<F>(op.mask);
        }
    };

    template <typename F>
    struct read : field_op<F> {
        using register_t = typename field_op<F>::register_t;
        friend constexpr auto tag_invoke(apply_t, read)
        {
            return *register_t::value;
        }
    };

    template <typename O1, typename O2>
    struct composed {
        using op_1_register_t = typename std::remove_reference_t<O1>::register_t;
        using op_2_register_t = typename std::remove_reference_t<O2>::register_t;
        static_assert(std::is_same_v<op_1_register_t,op_2_register_t>
            or std::is_base_of_v<op_1_register_t,op_2_register_t> 
            or std::is_base_of_v<op_2_register_t,op_1_register_t>,
            "only operations targeting fields from the same register may be composed");
        using register_t = op_1_register_t;
        using value_t = typename register_t::value_t;
        const O1 op1;
        const O2 op2;

        friend constexpr auto tag_invoke(apply_t, composed comp, const value_t in)
        {
            return apply(comp.op2,apply(comp.op1,in));
        }
    };

    template <typename O1, typename O2>
    composed(O1&&,O2&&) -> composed<O1,O2>; 

    //TODO: implement atomic operations
    template <typename T>
    struct atomic;
    
    template <typename T>
    concept any_register_op = requires(T o, typename T::value_t v) {
        { utl::reg::ops::apply(o,v) } -> std::same_as<typename T::value_t>;
    };

    constexpr auto operator |(any_register_op auto a, const any_register_op auto b)
    {
        using op_a_register_t = typename decltype(a)::register_t;
        using op_b_register_t = typename decltype(b)::register_t;
        static_assert(std::is_same_v<op_a_register_t,op_b_register_t>,
            "only operations targeting fields from the same register may be composed");
        return ops::composed<decltype(a),decltype(b)>{a,b};
    }

    // template <typename... Ts>
    // constexpr auto operator |(tuple<Ts...> tup, const any_register_op auto op)
    // {        
    //     using op_b_register_t = typename decltype(op)::register_t;
    //     static_assert((std::is_same_v<typename Ts::register_t,op_b_register_t> and ...),
    //         "only operations targeting fields from the same register may be composed");
    //     return utl::apply([op](auto... args) {
    //         return tuple{args...,op};
    //     }, tup);
    // }

} //namespace ops

constexpr auto mask(auto value, auto&&... fields)
{
    return value & (fields.mask | ...);
}

template <typename F>
constexpr auto assign(const F, const fields::value_t<F> value)
{
    static_assert(fields::is_writable<F>, "field is not writable");
    return ops::assign<F>{{},value};
}

template <typename F>
constexpr auto set(const F, const fields::value_t<F> value)
{
    static_assert(fields::is_writable<F>, "field is not writable");
    return ops::set<F>{{value}};
}

template <typename F>
constexpr auto clear(const F, const fields::value_t<F> value)
{
    static_assert(fields::is_writable<F>, "field is not writable");
    return ops::clear<F>{{value}};
}

template <typename F>
constexpr auto read(const F)
{
    static_assert(fields::is_readable<F>, "field is not readable");
    static_assert(not fields::is_cleared_on_read<F>, 
        "this field and/or its register will be cleared on read; use read_clear(...) instead");
    static_assert(not fields::is_set_on_read<F>,
        "this field and/or its register will be set on read; use read_set(...) instead");
    return ops::read<F>{};
}

template <auto* A>
struct sfr {
    using value_t = std::remove_pointer_t<std::decay_t<decltype(A)>>;
    using register_t = sfr;
    static constexpr auto width_bits = sizeof(value_t)*8u;
    static constexpr volatile value_t* value = A;

    constexpr explicit operator value_t() const { return *value; }

    //allows a literal register to take part in composition. just returns
    //the register's current value.
    friend constexpr value_t tag_invoke(ops::apply_t, const sfr reg, const value_t)
    {
        return *reg.value;
    }

    // constexpr auto operator |(const ops::any_register_op auto op) const
    // {
    //     static_assert(std::is_base_of_v<sfr,typename decltype(op)::register_t>,
    //         "register reads are only composable with operations targeting that register");
    //     return ops::composed{ops::read{this},op};
    // }

    //FIXME: a composed op should be an op type, not a tuple type
    // I should end up with a tree of ops that's applied from the top down.

    //FIXME: consider mapping familiar register operation operators to
    // operators.

    //NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)
    constexpr auto const& operator =(const ops::any_register_op auto&& op) const
    {
        using op_register_t = typename std::remove_reference_t<decltype(op)>::register_t;
        static_assert((std::is_base_of_v<sfr,op_register_t>),
            "operation must target fields in the target register");
        //apply the register operation
        *value = ops::apply(op,value_t{});
        return static_cast<op_register_t const&>(*this);

        //we have the underlying register type,
        //but also provide a customization object
        //that allows user conversions from arbitrary
        //types to a register value.
        //this allows the definition of data structures
        //that map to registers.
        //there must be a tag_invoke specialization for
        //the conversion operation, and it must return
        //a register operation corresponding to its
        //state. constexpr if possible.

        //how do I denote the register to apply the operatio
        //to?

        //how are datatypes for registers useful?
        //- allows c-style mapping between a register and a register value
        //- difficulty is that this function needs to be defined for the type
        //- in c, it's just a bitfield that's directly converted
        //- but I want type safety. so I think this is unavoidable.
    }

    constexpr auto const& operator |=(const ops::any_register_op auto&& op) const
    {
        using op_register_t = typename std::remove_reference_t<decltype(op)>::register_t;
        static_assert((std::is_base_of_v<sfr,op_register_t>),
            "operation must target fields in the target register");
        //apply the register operation
        *value = ops::apply(ops::composed{static_cast<op_register_t const&>(*this),op},value_t{});
        return *this;
    }
};

} //namespace utl::reg



