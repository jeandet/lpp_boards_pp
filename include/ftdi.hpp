#include <array>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace _ftdi
{
#include <libftdi1/ftdi.h>
}

class FtdiCtxWrapper
{
    _ftdi::ftdi_context* ctx = nullptr;

public:
    auto ftdi_call(auto& function, auto&&... args)
    {
        if (ctx != nullptr)
            return function(this->ctx, std::forward<decltype(args)>(args)...);
        else
            throw std::runtime_error("Can't call libftdi function, ctx handle is null");
    }

    FtdiCtxWrapper() : ctx { _ftdi::ftdi_new() } { }
    ~FtdiCtxWrapper()
    {
        if (this->ctx != nullptr)
        {
            this->ftdi_call(_ftdi::ftdi_usb_close);
            this->ftdi_call(_ftdi::ftdi_free);
            this->ctx = nullptr;
        }
    }
    inline operator _ftdi::ftdi_context*() { return ctx; }
};

enum class Interface
{
    ANY = _ftdi::INTERFACE_ANY,
    A = _ftdi::INTERFACE_A,
    B = _ftdi::INTERFACE_B,
    C = _ftdi::INTERFACE_C,
    D = _ftdi::INTERFACE_D,
};

enum class Mode
{
    SERIAL = _ftdi::BITMODE_RESET,
    BITBANG = _ftdi::BITMODE_BITBANG,
    MPSSE = _ftdi::BITMODE_MPSSE,
    SYNCBB = _ftdi::BITMODE_SYNCBB,
    MCU = _ftdi::BITMODE_MCU,
    OPTO = _ftdi::BITMODE_OPTO,
    CBUS = _ftdi::BITMODE_CBUS,
    SYNCFF = _ftdi::BITMODE_SYNCFF,
    FT1284 = _ftdi::BITMODE_FT1284,
};


class FtdiDriver
{
    FtdiCtxWrapper ctx;
    bool _opened = false;

public:
    FtdiDriver();
    FtdiDriver(const FtdiDriver&) = delete;
    FtdiDriver(FtdiDriver&&) = default;
    ~FtdiDriver();

    FtdiDriver& operator=(const FtdiDriver&) = delete;
    FtdiDriver& operator=(FtdiDriver&&) = default;

    static std::vector<std::string> find_by_manufacturer_and_description(
        const std::string& manufacturer, const std::string& description);

    auto ftdi_call(auto& function, auto&&... args)
    {
        return this->ctx.ftdi_call(function, std::forward<decltype(args)>(args)...);
    }

    bool open(const std::string& serial, Interface interface = Interface::ANY,
        Mode mode = Mode::SERIAL, unsigned char io_mask = 0);
    inline bool opened() { return this->_opened; }


    void set_read_buffer_size(std::size_t size);
    void set_write_buffer_size(std::size_t size);
    void set_latency_timer(unsigned char latency);


    void flush_read_buffer();
    void flush_write_buffer();
    void flush_buffers();

    inline auto read(auto& buffer, std::size_t count) noexcept
    {
        auto read = this->ftdi_call(
            _ftdi::ftdi_read_data, reinterpret_cast<unsigned char*>(std::data(buffer)), count);
        return read;
    }
};

template <Interface interface, Mode mode, unsigned char io_mask>
class FtdiDevice
{
    FtdiDriver _driver;

public:
    FtdiDevice() { }
    FtdiDevice(const std::string& serial) { open(serial); }

    inline bool open(const std::string& serial)
    {
        return _driver.open(serial, interface, mode, io_mask);
    }
    inline bool opened() { return this->_driver.opened(); }

    inline void set_read_buffer_size(std::size_t size) { _driver.set_read_buffer_size(size); }
    inline void set_write_buffer_size(std::size_t size) { _driver.set_write_buffer_size(size); }
    inline void set_latency_timer(unsigned char latency) { _driver.set_latency_timer(latency); }

    inline void flush_read_buffer() { _driver.flush_read_buffer(); }
    inline void flush_write_buffer() { _driver.flush_write_buffer(); }
    inline void flush_buffers() { _driver.flush_buffers(); }

    inline auto read(std::size_t count) noexcept
    {
        std::vector<unsigned char> buffer(count);
        auto read = _driver.read(buffer, count);
        if (static_cast<std::size_t>(read) != count)
        {
            if (read >= 0)
                buffer.resize(read);
            else
                buffer.clear();
        }
        return buffer;
    }

    inline auto read(auto&& buffer, std::size_t count) noexcept
    {
        return _driver.read(std::forward<decltype(buffer)>(buffer), count);
    }
};