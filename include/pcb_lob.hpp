
#include "ftdi.hpp"
#include "simple_protocol.hpp"

class PCB_LOB
{
    decltype(make_simple_decoder<4, 4096>(FtdiDevice<Interface::A, Mode::SYNCFF, 0xff>{})) _dev;

public:
    PCB_LOB(const std::string& serial_number)
            : _dev{make_simple_decoder<4, 4096>(FtdiDevice<Interface::A, Mode::SYNCFF, 0xff>{serial_number})}
    {
    }

    auto& dev() const
    {
        return _dev;
    }

    inline void start()
    {
        _dev.start();
    }

    inline void stop()
    {
        _dev.stop();
    }

    inline auto samples(int timeout_ns = -1)
    {
        return _dev.get_samples(timeout_ns);
    }

    inline const auto& serial_number() const
    {
        return _dev.data_producer().serial_number();
    }

    inline auto opened() const
    {
        return _dev.data_producer().opened();
    }

    inline void flush() const
    {
        _dev.data_producer().flush();
    }

};
