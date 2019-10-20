
#include "error.hh"
#include "system-error.hh"

//This file exists to attach error_category's vtable to a translation unit (-Wweak-vtables)

namespace utl {

struct system_error_category : error_category {
    const utl::string_view message(int32_t value) const final;
    const utl::string_view name() const final;
};

const utl::string_view system_error_category::message(int32_t value) const {
    auto code = static_cast<system_error>(value);
    switch(code) {
        case system_error::OK:
            return "system_error::OK"_sv;
        case system_error::UNKNOWN:
            return "Unknown system error"_sv;
    }
}

const utl::string_view system_error_category::name() const {
    return "system"_sv;
}

error_code make_error_code(system_error code) {
    static const system_error_category category{};
    return {static_cast<int32_t>(code), &category};
}



const utl::string_view error_category::message(int32_t value) const { 
    utl::maybe_unused(value); return ""_sv; 
}

const utl::string_view error_category::name() const { 
    return ""_sv; 
}

}
