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

template <auto*> using any_register_helper = void;

template <typename T>
concept any_register = requires(const T v) {
    std::remove_reference_t<T>::location();
    v.value();
    { v.value() } -> std::same_as<std::remove_pointer_t<decltype(std::remove_reference_t<T>::location())>>;
    *std::remove_reference_t<T>::location() = v.value();
    typename any_register_helper<std::remove_reference_t<T>::location()>;
};

template <typename R1, typename R2>
concept same_register_as = any_register<R1> and any_register<R2> and
    (std::remove_reference_t<R1>::location() == std::remove_reference_t<R2>::location());

template <any_register T>
using register_value_t = std::remove_reference_t<decltype(std::declval<T>().value())>;

BFG_TAG_INVOKE_DEF(register_cast);

// constexpr void tag_invoke(register_cast_t, auto&&, auto&&)
// {
//     static_assert()
// }

//need to bring together fields & registers more...
//I shouldn't have to define separate ops & customization points
//for each; just one that dispatches on the register/field type.
//at the end of the day, everything under the hood is dealing with
//register-wide values.
//I should be able to:
// - convert a value (that customizes reg::convert) into a composed
//    operation
// - use any convertible value in a composition
// - use any register in a composition
// - use any field in a composition

namespace ops {

    BFG_TAG_INVOKE_DEF(apply);
    BFG_TAG_INVOKE_DEF(compose);

    template <typename T, typename R>
    concept any_register_op = requires(R r, T o, typename T::value_t v) {
        { utl::reg::ops::apply(r,o,v) } -> std::same_as<typename T::value_t>;
    };

    template <typename T>
    concept has_target_register = requires() {
        typename std::remove_reference_t<T>::register_t;
    };

    template <has_target_register T>
    using target_register_t = typename std::remove_reference_t<T>::register_t;

    template <typename T>
    concept any_targeted_register_op = has_target_register<T> and any_register_op<T,target_register_t<T>>;


    template <typename F>
    struct field_op {
        using field_t = F;
        using register_t = typename F::register_t;
        using value_t = typename register_t::value_t;
    };

    template <typename R, typename F>
    struct op_info {
        using register_t = R;
        using field_t = F;
        using value_t = typename R::value_t;
    };

    template <typename R1, typename F1, typename R2, typename F2>
    constexpr void check_compatibility(op_info<R1,F2>, op_info<R2,F2>)
    {
        static_assert(same_register_as<R1,R2>,
            "register operations must target the same register");

        //FIXME: this might not be a correct assertion
        static_assert(not same_as<F1,F2>,
            "register operations must not target the same field");
    }

    template <typename O1, typename O2>
    struct composed {
        using op_a_register_t = target_register_t<O1>;
        using op_b_register_t = target_register_t<O2>;

        using register_t = op_a_register_t;
        using value_t = typename register_t::value_t;
        const O1 op1;
        const O2 op2;

        friend constexpr value_t tag_invoke(apply_t, const register_t reg, composed comp, const value_t in)
        {
            return apply(reg,comp.op2,apply(reg,comp.op1,in));
        }
    };

    template <typename O1, typename O2>
    composed(O1&&,O2&&) -> composed<O1,O2>; 


    template <typename R>
    struct manipulate {
        //these masks must be constructed such that they are orthogonal
        //to guarantee commutativity
        using register_t = R;
        using value_t = typename R::value_t;
        value_t set_mask;
        value_t clear_mask;

        friend constexpr value_t tag_invoke(apply_t, const register_t, const manipulate<register_t> op, const value_t in)
        {
            return (in | op.set_mask) & ~op.clear_mask;
        }

        template <typename T>
        friend constexpr auto tag_invoke(compose_t, const register_t, const manipulate<register_t> op1, const manipulate<T> op2)
        {
            static_assert(same_register_as<register_t,T>, "only operations targeting the same"
                " register may be composed");
            return manipulate<R>{(op1.set_mask | op2.set_mask) & ~op2.clear_mask, (op1.clear_mask | op2.clear_mask) & ~op2.set_mask};
        }
    };

    template <typename F>
    struct read : field_op<F> {
        using register_t = typename field_op<F>::register_t;
        using value_t = typename register_t::value_t;
        manipulate<register_t> reduced{value_t{},value_t{}};

        friend constexpr value_t tag_invoke(apply_t, const register_t reg, const read op)
        {
            //FIXME: need a "whole register" read operation
            //FIXME: this needs to dealign things properly.
            return (reg.value() | op.reduced.set_mask) & ~op.reduced.clear_mask;
        }

        template <typename T>
        friend constexpr auto tag_invoke(compose_t, const read op1, const manipulate<T> op2)
        {
            // static_assert(same_register_as<register_t,T>, "only operations targeting the same"
            //     " register may be composed");
            auto red = compose(op1.reduced,op2);
            return read{red.set_mask,red.clear_mask};
        }

        template <typename T>
        [[deprecated("any operations prior to a 'read' operation will be discarded")]]
        friend constexpr auto tag_invoke(compose_t, const any_register_op<register_t> auto, read op)
        {
            return op;
        }
    };

    //TODO: implement atomic operations
    template <typename T>
    struct atomic;
    
    
    constexpr auto tag_invoke(apply_t, const any_register auto reg, const auto&& castable, const register_value_t<decltype(reg)> in)
    {
        return apply(register_cast(reg, castable), in);
    }

    template <typename R>
    constexpr auto tag_invoke(register_cast_t, const any_register_op<R> auto op)
    {
        return op;
    }

    template <typename R>
    constexpr auto tag_invoke(compose_t, const any_register_op<R> auto a, const any_register_op<R> auto b)
    {
        auto cast_a = register_cast(reg,a);
        auto cast_b = register_cast(reg,b);
        check_compatibility(a.info,b.info);
        return composed{a,b};
    }

    template <any_
    constexpr auto tag_invoke(compose_t, )

    template <any_targeted_register_op T>
    constexpr auto operator |(const T a, const auto&& b)
    {
        return compose(a,b);
    }

    template <any_targeted_register_op T>
    constexpr auto operator |(const auto&& a, const T b)
    {
        return compose(a,b);
    }

    template <any_targeted_register_op T, any_targeted_register_op U>
    constexpr auto operator |(const T a, const U b)
    {
        return compose(a,b);
    }

    // template <has_target_register T, has_target_register U>
    // constexpr auto operator |(U a, T b)
    // {
    //     static_assert(same_register_as<target_register_t<T>,target_register_t<U>>,
    //         "only operations targeting fields from the same register may be composed");
    //     return compose(a,b);
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
    // return ops::assign<F>{{},value};
    auto aligned = fields::align_to_field<F>(value);
    return ops::manipulate<typename F::register_t>{
        aligned,
        F::mask & ~aligned
    };
}

template <typename F>
constexpr auto set(const F, const fields::value_t<F> value)
{
    static_assert(fields::is_writable<F>, "field is not writable");
    // return ops::set<F>{{value}};
    auto aligned = fields::align_to_field<F>(value);
    return ops::manipulate<typename F::register_t>{aligned,fields::value_t<F>{}};
}

template <typename F>
constexpr auto clear(const F, const fields::value_t<F> value)
{
    static_assert(fields::is_writable<F>, "field is not writable");
    // return ops::clear<F>{{value}};
    auto aligned = fields::align_to_field<F>(value);
    return ops::manipulate<typename F::register_t>{fields::value_t<F>{},aligned};
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


template <typename R, auto* A>
struct sfr {
    using value_t = std::remove_pointer_t<std::decay_t<decltype(A)>>;
    using register_t = R;
    static constexpr auto width_bits = sizeof(value_t)*8u;
    static constexpr volatile value_t* loc = A;

    constexpr explicit operator value_t() const { return *loc; }
    constexpr explicit operator value_t*() const { return A; }

    static constexpr value_t* location() { return A; }
    constexpr value_t value() const { return *loc; }

    //allows a literal register to take part in composition. just returns
    //the register's current value.
    friend constexpr value_t tag_invoke(ops::apply_t, const register_t, const register_t reg, const value_t)
    {
        return reg.value();
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
    constexpr auto const& operator =(const auto&& op) const
    {
        using op_register_t = typename std::remove_reference_t<decltype(op)>::register_t;
        static_assert((std::is_base_of_v<register_t,op_register_t>),
            "operation must target fields in the target register");
        //apply the register operation
        constexpr auto nop = ops::manipulate<register_t>{};
        const auto& this_register = static_cast<register_t const&>(*this);
        *location() = ops::apply(this_register, ops::compose(this_register, nop,op), value_t{});
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

    constexpr auto const& operator |=(const ops::any_register_op<register_t> auto op) const
    {
        //apply the register operation
        *location() = ops::apply(static_cast<register_t const&>(*this), op, value());
        return *this;
    }


    constexpr bool operator ==(const any_register auto&& reg)
    {
        return same_register_as<sfr,decltype(reg)>;
    }
};

} //namespace utl::reg


// changes to make:
// - allow operations targeting distinct registers to be composed
//  how best to do this? a tuple of reduced compositions...?
//  ideally I'd have some kind of map from register type to a compose op
// - apply should return a value if there's a single field read, a tuple if multiple
// - should an op/composition be callable?
// - peripherals should be accessed via an instance, so it can be easily passed around for generic code

// need to be able to compose distinct register values. being able to represent a "configuration"
// as a value and pass it around would be extremely useful, and maybe lends itself to mixins?


