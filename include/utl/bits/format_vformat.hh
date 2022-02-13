#pragma once

#include <utl/bits/format_specifier.hh>
#include <utl/bits/format_output.hh>

namespace utl::fmt {

namespace detail {
    struct error{};
    
    struct varg {
        constexpr varg() = default;
        constexpr varg(varg const&) = default;
        constexpr auto operator=(varg const&) = delete;
        constexpr virtual ~varg() = default;
        constexpr virtual void format(output& out, field const& f) const = 0;
    };

    template <typename T>
    struct arg_t final : public virtual varg {
        using type = T;
        const T value;
        arg_t(T v) : value{v} {}            
        // arg_t(T& v) : value{v} {}
        void format(output& out, field const& f) const final
        {
            format_arg(value, out, f);
        }
    };

    template <typename T>
    struct arg_t<T&> final : public virtual varg {
        using type = T;
        T const& value;
        arg_t(T const& v) : value{v} {}
        void format(output& out, field const& f) const final
        {
            format_arg(value, out, f);
        }
    };

    template <typename T>
    arg_t(T) -> arg_t<T>;

    // template <typename T>
    // arg_t(T&) -> arg_t<T&>;

    template <typename... Ts>
    struct arg_storage {
        static constexpr size_t n_args = sizeof...(Ts);
        using arg_storage_t = utl::tuple<arg_t<Ts>...>;
        using varg_storage_t = utl::array<varg const* const,n_args>;

        const arg_storage_t args;
        const varg_storage_t vargs;

        template <size_t... N>
        constexpr arg_storage(std::index_sequence<N...>, std::convertible_to<Ts> auto&&... args_)
        : args{arg_t<Ts>{std::forward<decltype(args_)>(args_)}...},
            vargs{&utl::get<N>(args)...}
        {}

        constexpr arg_storage(std::convertible_to<Ts> auto&&... args_) 
        : arg_storage{std::make_index_sequence<n_args>{}, std::forward<decltype(args_)>(args_)...}
        {}
    };

    template <typename... Ts>
    arg_storage(Ts...) -> arg_storage<Ts...>;

    struct arglist {
        using arg_view_t = utl::span<varg const* const>;
        struct next_arg_tag{};

        const arg_view_t view;
        size_t next_arg;

        template <typename... Ts>
        arglist(arg_storage<Ts...> const& s) : view{s.vargs.data(),s.vargs.size()}, next_arg{0} {}

        varg const& consume_next() { return *view[next_arg++]; }
        [[nodiscard]] varg const& get(size_t idx) const { return *view[idx]; }
        [[nodiscard]] bool valid_id(size_t idx) const { return idx < view.size(); }
        [[nodiscard]] bool valid_id(next_arg_tag) const { return next_arg < view.size(); }
    };
    
    template <typename... Ts>
    constexpr auto wrap_args(Ts&&... vs)
    {
        return arg_storage{std::forward<Ts>(vs)...};
    }

    template <typename... Ts>
    constexpr auto erase_args(arg_storage<Ts...> const& s)
    {
        return arglist{s};
    }

    template <typename... Ts>
    constexpr utl::tuple<arglist,arg_storage<Ts...>> make_args(Ts&&... vs)
    {
        auto storage = arg_storage{std::forward<Ts>(vs)...};
        return {arglist{storage},storage};
    }

} //namespace detail

void vformat(output& out, utl::string_view format, detail::arglist& args, 
    arg_spec::modes id_mode = arg_spec::modes::UNKNOWN);

//NOLINTNEXTLINE(misc-no-recursion)
inline auto make_field(utl::string_view view, detail::arglist& args)
{
    constexpr utl::string_view field_spec_start = ":";
    if(view.length() == 0) return field{};

    size_t pos = 0;
    utl::string<MAX_SPEC_SIZE> formatted{};
    auto write_buffer = [&] (char c) {
        if(pos < formatted.size()) formatted[pos++] = c;
    };
    output_t into_buffer{write_buffer};

    const size_t mark = view.find(field_spec_start);

    if(mark != utl::npos and mark+1 < view.length()) {
        auto spec_view = view.substr(mark+1,utl::npos);
        if(spec_view.length() > 0 and spec_view.length() < MAX_SPEC_SIZE) {
            vformat(into_buffer, view.substr(mark+1,utl::npos), args);
        }
    }

    return field{arg_spec{view.substr(0,mark)}, formatted};
}

//NOLINTNEXTLINE(readability-function-cognitive-complexity,misc-no-recursion)
inline void vformat(output& out, utl::string_view format, detail::arglist& args, 
    arg_spec::modes id_mode)
{
    //FIXME: id_mode isn't being passed around properly.
    struct format_state {
        enum states {
            ECHO,
            ESCAPED_ENTRY,
            ESCAPED_EXIT,
            PROCESS_FIELD
        };
        states active = ECHO;
    };
    
    constexpr auto field_entry = '{';
    constexpr auto field_exit = '}';
    size_t field_inner_depth = 0;
    auto state = format_state{};
    size_t mark{};

    auto process_field = [&](field f) {
        if(id_mode == arg_spec::modes::UNKNOWN) {
            id_mode = f.id.mode();
        }

        if(f.id.mode() != id_mode) {            
            //switching field numbering modes isn't allowed.
            out(error_char);
            return;
        }

        if(f.id.mode() == arg_spec::modes::AUTOMATIC) {
            if(args.valid_id(detail::arglist::next_arg_tag{})) {
                args.consume_next().format(out,f);
            }
        } else if(args.valid_id(f.id.id())) {
            args.get(f.id.id()).format(out,f);
        } else {
            out(error_char);
        }
    };

    for(auto&& [pos,c] : utl::ranges::enumerate(format)) {
        switch(state.active) {
            case format_state::ECHO:
                if(c == field_entry) {
                    state.active = format_state::ESCAPED_ENTRY;
                } else if(c == field_exit) {
                    state.active = format_state::ESCAPED_EXIT;
                    continue;
                } else {
                    out(c);
                }
                break;
            case format_state::ESCAPED_ENTRY:   
                if(c == field_entry) {
                    //escaped for output
                    out(c);
                    state.active = format_state::ECHO;
                } else if(c == field_exit) {
                    //you are standing in an empty field...
                    process_field(field{});
                    state.active = format_state::ECHO;
                    break;
                } else {
                    mark = pos;
                    state.active = format_state::PROCESS_FIELD;
                }             
                break;
            case format_state::ESCAPED_EXIT:
                if(c == field_entry) {
                    state.active = format_state::ESCAPED_ENTRY;
                } else {
                    out(c);
                    state.active = format_state::ECHO;
                }
                break;
            case format_state::PROCESS_FIELD:
                if(c == '{') {
                    //we support fields in fields
                    field_inner_depth++;
                } else if(c == '}' and field_inner_depth > 0) {
                    field_inner_depth--;
                } else if(c == '}') {
                    //FIXME: need to know if formatting the field yielded an error.
                    auto f = make_field(format.substr(mark, pos - mark), args);
                    process_field(f);
                    state.active = format_state::ECHO;
                }
                break;
        }
    }
}

template <typename T>
concept formattable = requires(T const& arg, output& out, field& spec) {
    format_arg(arg, out, spec);
};

} //namespace utl::fmt
