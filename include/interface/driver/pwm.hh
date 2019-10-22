#ifndef UTL_INTERFACE_DRIVER_PWM_HH_
#define UTL_INTERFACE_DRIVER_PWM_HH_

#include "interface/driver/driver.hh"
#include "result.hh"

namespace utl::driver::pwm::interface {

// class pwm : public driver {
// public:
    
// };

template <class Pwm>
class channel {
    using id_t = typename Pwm::channel_id_t;
    Pwm const& m_source;
    id_t m_id;
public:
    channel(Pwm const& source, id_t id) : m_source{source}, m_id{id} {}
    utl::result<void> start() { m_source.start(m_id); }
    utl::result<void> stop() { m_source.stop(m_id); }
    // void set_duty();
};

} //namespace utl::driver::pwm::interface

#endif //UTL_INTERFACE_DRIVER_PWM_HH_