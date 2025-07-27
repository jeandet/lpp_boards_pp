#include "ftdi.hpp"
#include <algorithm>
#include <optional>

inline bool _compare(const std::string& ref, const auto& data)
{
    const auto len = std::min(std::size(ref), std::size(data));
    for (auto i = 0UL; i < len; i++)
    {
        if (ref[i] != data[i])
            return false;
    }
    return true;
}

inline void _clear(auto& data)
{
    std::fill(std::begin(data), std::end(data), 0);
}

inline void _clear(auto&&... data)
{
    (_clear(std::forward<decltype(data)>(data)), ...);
}

std::optional<FtdiDriver> open_device(const std::string& serial)
{
    FtdiDriver driver;
    struct _ftdi::ftdi_device_list *devlist, *curdev;
    std::array<char, 128> _serial;
    if (driver.ftdi_call(_ftdi::ftdi_usb_find_all, &devlist, 0, 0) < 0)
        return std::nullopt;
    for (curdev = devlist; curdev != NULL; curdev = curdev->next)
    {
        driver.ftdi_call(_ftdi::ftdi_usb_get_strings, curdev->dev, static_cast<char*>(NULL), 0,
            static_cast<char*>(NULL), 0, _serial.data(), 128);
        if (_compare(serial, _serial))
        {
            driver.ftdi_call(_ftdi::ftdi_usb_open_dev, curdev->dev);
            ftdi_list_free(&devlist);
            return std::move(driver);
        }
    }
    ftdi_list_free(&devlist);
    return std::nullopt;
}

FtdiDriver::FtdiDriver() { }

FtdiDriver::~FtdiDriver() { }

std::vector<std::string> FtdiDriver::find_by_manufacturer_and_description(
    const std::string& manufacturer, const std::string& description)
{
    std::vector<std::string> found;
    if (FtdiCtxWrapper ftdi; ftdi != nullptr)
    {
        struct _ftdi::ftdi_device_list *devlist, *curdev;
        std::array<char, 128> _serial, _manufacturer, _description;
        if (ftdi.ftdi_call(_ftdi::ftdi_usb_find_all, &devlist, 0, 0) < 0)
            return {};
        for (curdev = devlist; curdev != NULL; curdev = curdev->next)
        {
            _clear(_manufacturer, _description, _serial);

            ftdi.ftdi_call(_ftdi::ftdi_usb_get_strings, curdev->dev, _manufacturer.data(), 128,
                _description.data(), 128, _serial.data(), 128);

            if (_compare(manufacturer, _manufacturer) && _compare(description, _description))
            {
                found.push_back(std::string { _serial.data() });
            }
        }
        ftdi_list_free(&devlist);
    }
    return found;
}

bool FtdiDriver::open(
    const std::string& serial, Interface interface, Mode mode, unsigned char io_mask)
{
    struct _ftdi::ftdi_device_list *devlist, *curdev;
    std::array<char, 128> _serial;
    if (this->ftdi_call(_ftdi::ftdi_usb_find_all, &devlist, 0, 0) < 0)
        return false;
    for (curdev = devlist; curdev != NULL; curdev = curdev->next)
    {
        this->ftdi_call(_ftdi::ftdi_usb_get_strings, curdev->dev, static_cast<char*>(NULL), 0,
            static_cast<char*>(NULL), 0, _serial.data(), 128);
        if (_compare(serial, _serial))
        {
            this->_opened = (this->ftdi_call(_ftdi::ftdi_usb_open_dev, curdev->dev) == 0);
            this->_opened &= (this->ftdi_call(_ftdi::ftdi_set_interface,
                                  static_cast<enum _ftdi::ftdi_interface>(interface))
                == 0);
            this->_opened &= (this->ftdi_call(_ftdi::ftdi_set_bitmode, io_mask,
                                  static_cast<unsigned char>(mode))
                == 0);
            break;
        }
    }
    ftdi_list_free(&devlist);
    return this->_opened;
}

void FtdiDriver::set_read_buffer_size(std::size_t size)
{
    this->ftdi_call(_ftdi::ftdi_read_data_set_chunksize, size);
}

void FtdiDriver::set_write_buffer_size(std::size_t size)
{
    this->ftdi_call(_ftdi::ftdi_write_data_set_chunksize, size);
}

void FtdiDriver::set_latency_timer(unsigned char latency)
{
    this->ftdi_call(_ftdi::ftdi_set_latency_timer, latency);
}

void FtdiDriver::flush_read_buffer() const{ this->ftdi_call(_ftdi::ftdi_tciflush); }

void FtdiDriver::flush_write_buffer() const{ this->ftdi_call(_ftdi::ftdi_tcoflush); }

void FtdiDriver::flush_buffers()const { this->ftdi_call(_ftdi::ftdi_tcioflush); }
