#ifndef UTL_INTERFACE_DRIVER_PWM_HH_
#define UTL_INTERFACE_DRIVER_PWM_HH_

#include "utl/interface/hal/driver.hh"
#include "utl/result.hh"

namespace utl::hal::pwm::interface {

enum class polarity {
    ACTIVE_LOW,
    ACTIVE_HIGH
};

template <class Pwm>
class channel {
public:
    using channel_id_t = typename Pwm::channel_id_t;
    using time_t = typename Pwm::time_t;
    using pwm_t = Pwm;

    Pwm & m_source;
    channel_id_t m_id;
    polarity m_polarity;
public:
    channel(Pwm & source, channel_id_t id, polarity pol) : m_source{source}, m_id{id}, m_polarity{pol} {}
    utl::result<void> start() { return m_source.start(m_id, m_polarity); }
    utl::result<void> stop() { return m_source.stop(m_id); }
    void set_width(time_t width) { m_source.set_width(m_id, width); }
    time_t width() const { return m_source.width(m_id); }
    void set_polarity(polarity pol) { m_polarity = pol; }  
};

template <class Pwm, class Dma>
class dma_channel : public channel<Pwm> {
    using channel<Pwm>::m_source;
    using channel<Pwm>::m_id;
    using channel<Pwm>::m_polarity;
public:
    using channel_id_t = typename Pwm::channel_id_t;
    using time_t = typename Pwm::time_t;
    using pwm_t = Pwm;

    using channel<Pwm>::channel;

    utl::result<void> start(uint32_t* data, uint32_t length) { return m_source.start_dma(m_id, m_polarity, data, length); }
    utl::result<void> stop() { return m_source.stop_dma(m_id); }
    constexpr uint32_t width_to_dma_value(time_t width) const { return m_source.width_to_dma_value(width); }  
    void link_dma(Dma& dma) { m_source.link_dma(m_id, dma); }
};

} //utl::hal::pwm::interface

#endif //UTL_INTERFACE_DRIVER_PWM_HH_