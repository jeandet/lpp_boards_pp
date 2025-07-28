
#include "ftdi.hpp"
#include "simple_protocol.hpp"

class PCB_LOB
{
public:
    static constexpr auto channel_count = 4;
    using decoder_type = decltype(make_simple_decoder<channel_count>(4096,
        FtdiDevice<Interface::A, Mode::SYNCFF, 0xff> {}));

    PCB_LOB(const std::string& serial_number, std::size_t samples_count = 4096)
            : _dev { make_simple_decoder<channel_count>(samples_count,
                  FtdiDevice<Interface::A, Mode::SYNCFF, 0xff> { serial_number }) }
    {
    }

    std::size_t get_raw_data(char* buffer, std::size_t count)
    {
       return _dev.get_raw_data(buffer, count);
    }

    auto& dev() const { return _dev; }

    inline void start() { _dev.start(); }

    inline void stop() { _dev.stop(); }

    inline auto samples(std::optional<std::chrono::nanoseconds> timeout_ns = std::nullopt)
    {
        return _dev.get_samples(timeout_ns);
    }

    inline const auto& serial_number() const { return _dev.data_producer().serial_number(); }

    inline auto opened() const { return _dev.data_producer().opened(); }

    inline void flush() const { _dev.data_producer().flush(); }

private:
    decoder_type _dev;
};
