#pragma once

#include <utility>
#include <utl/bitfield.hh>
#include <utl/hof/fold.hh>
#include <utl/hof/compose.hh>

namespace utl::pal {

template <auto R, utl::any_bitfield... Ts>
struct sfr : public utl::bitstruct<Ts...> {};

template <typename... Ts>
struct mm_register : public bitstruct<Ts...> {
    //why aren't these being considered as candidates???
    constexpr mm_register(mm_register volatile const& other) : bitstruct<Ts...>{other} {}
    constexpr mm_register(mm_register volatile& other) : bitstruct<Ts...>{other} {}
    constexpr mm_register(mm_register const& other) : bitstruct<Ts...>{other} {}
    constexpr mm_register(mm_register&&) = delete;
    using bitstruct<Ts...>::operator=;
};

template <typename T> //CRTP parameter
struct peripheral {
	//copyable (that's a read)
	//not movable
};


template <utl::any_bitstruct... Ts>
constexpr auto register_tuple_test(utl::tuple<Ts&...>) {}
template <utl::any_bitstruct... Ts>
constexpr auto register_tuple_test(utl::tuple<Ts volatile&...>) {}

template <typename T>
concept any_peripheral = requires(T v) {
	{ v.registers() } -> utl::any_tuple;
	register_tuple_test(v.registers());
};


//get a field using its tag value
template <utl::any_enum auto V>
constexpr auto get_field(any_peripheral auto& p)
{
	// template <auto V>
	constexpr auto find_accumulate = [](auto&& accum, auto& item) {
		using item_t = std::decay_t<decltype(item)>; //bitstruct (register)

		if constexpr(utl::has_field<item_t,V>) {
			return get_field<V>(item);
		} else {
			return accum;
		}
	};

	return utl::hof::foldl(p.registers(), nullptr, find_accumulate);
}

// template <utl::any_enum auto V, any_peripheral T>
// using field_t = std::remove_reference_t<decltype(get_field<V>(std::declval<T>()))>;

//get a register using its tag type
template <utl::any_enum E>
constexpr auto& get_register(any_peripheral auto& p)
{
	// template <auto V>
	constexpr auto find_accumulate = [](auto&& accum, auto& item) -> decltype(auto) {
		using item_t = std::decay_t<decltype(item)>; //bitstruct (register)

		if constexpr(std::same_as<E,typename item_t::tag_t>) {
			return item;
		} else {
			return accum;
		}
	};

	return utl::hof::foldl(p.registers(), nullptr, find_accumulate);
}

namespace op {
    template <utl::any_enum auto V, typename T>
    struct assign {
        using field_tag_t = std::decay_t<decltype(V)>;
        T value;
        //FIXME: right now, if two assignments to different
        //registers are composed you'll get an error when the
        //composition is called and it will probably be impossible
        //to figure out why.
        constexpr auto operator()(utl::has_field<V> auto&& r) const
            // requires requires(T v, field_value_t<V,decltype(r)> f) {
            //     f = v;
            // }
        {
            auto field = get_field<V>(std::forward<decltype(r)>(r));
            field = static_cast<decltype(field.value())>(value);
            return r;
        }
    };

    template <typename T>
    using field_tag_t = typename std::decay_t<T>::field_tag_t;
} //namespace op

template <utl::any_enum auto V>
constexpr auto assign(auto value)
{
    return op::assign<V,decltype(value)>{value};
}

template <utl::any_enum E>
constexpr auto read(any_peripheral auto& p)
{
    using register_t = std::decay_t<decltype(get_register<E>(p))>;
    return register_t{get_register<E>(p)}; //force a copy to a regular value type
}

template <utl::any_enum auto E>
constexpr auto read(any_peripheral auto& p)
{
    return get_field<E>(p).value();
}

template <any_peripheral P, typename T, typename... Ts>
constexpr auto write(P& p, T&& head, Ts&&... ops)
    requires (std::common_with<op::field_tag_t<T>,op::field_tag_t<Ts>> and ...)
{
    auto init = read<op::field_tag_t<T>>(p); //read to get starting value
    const auto value = hof::compose(
        std::forward<decltype(head)>(head), 
        std::forward<decltype(ops)>(ops)...
    )(std::move(init)); //compose & call
    get_register<op::field_tag_t<T>>(p) = value; //write the result to the register
    return value; //return the new register value
}

} //namespace utl::pal
