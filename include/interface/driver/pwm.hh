#ifndef UTL_INTERFACE_DRIVER_PWM_HH_
#define UTL_INTERFACE_DRIVER_PWM_HH_

#include "interface/driver/driver.hh"
#include "result.hh"

namespace utl::driver::pwm::interface {

enum class polarity {
    ACTIVE_LOW,
    ACTIVE_HIGH
};

template <class Pwm>
class channel {
    using id_t = typename Pwm::channel_id_t;
    Pwm & m_source;
    id_t m_id;
    polarity m_polarity;
public:
    channel(Pwm & source, id_t id, polarity pol) : m_source{source}, m_id{id}, m_polarity{pol} {}
    utl::result<void> start() { return m_source.start(m_id, m_polarity); }
    utl::result<void> stop() { return m_source.stop(m_id); }
    void set_width_ns(uint32_t width_ns) { m_source.set_width_ns(m_id, width_ns); }
    uint32_t width_ns() const { return m_source.width_ns(m_id); }
    void set_polarity(polarity pol) { m_polarity = pol; }    
};

template <class Pwm>
class dma_channel {
    using id_t = typename Pwm::channel_id_t;
    Pwm & m_source;
    id_t m_id;
    polarity m_polarity;
public:
    dma_channel(Pwm & source, id_t id, polarity pol) : m_source{source}, m_id{id}, m_polarity{pol} {}
    utl::result<void> start(uint32_t* data, uint32_t length) { return m_source.start_dma(m_id, m_polarity, data, length); }
    utl::result<void> stop() { return m_source.stop_dma(m_id); }
    void set_width_ns(uint32_t width_ns) { m_source.set_width_ns(m_id, width_ns); }
    uint32_t width_ns() const { return m_source.width_ns(m_id); }
    void set_polarity(polarity pol) { m_polarity = pol; }    
};

} //namespace utl::driver::pwm::interface

#endif //UTL_INTERFACE_DRIVER_PWM_HH_