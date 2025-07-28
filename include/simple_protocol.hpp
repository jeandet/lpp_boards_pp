#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "array2d.hpp"
#include "auto_recycled_channel.hpp"
#include "concepts.hpp"
#include <channels/channels.hpp>
#include <cstddef>
#include <cstring>

inline bool _is_sync_word(const auto& buffer, std::size_t index)
{
    return (static_cast<unsigned char>(buffer[index]) == 0xf0)
        && (static_cast<unsigned char>(buffer[index + 1]) == 0x0f);
}


template <std::size_t channels_count, data_producer data_producer_t>
class simple_decoder
{
    constexpr static auto _bytes_per_sample = sizeof(uint16_t);
    constexpr static uint16_t _sync_word = 0xf00f;
    constexpr static auto _sync_word_bytes = sizeof(uint16_t);
    // constexpr static auto bytes_per_packet = (channels_count + 2) * bytes_per_sample;
    // constexpr static auto bytes_per_window = window_size * bytes_per_packet;
    std::size_t _bytes_per_packet = 0;
    std::size_t _bytes_per_window = 0;
    std::size_t _window_size = 0;
    std::size_t _buffer_start_index = 0;
    std::size_t _buffer_stop_index = 0;
    data_producer_t _data_producer;
    std::atomic<bool> _running = true;
    std::thread _thread;

    void _resync(auto& buffer)
    {
        _data_producer.flush();
        auto missing = (2 * _bytes_per_packet) - 2;
        for (auto i = _bytes_per_window - 2; missing; i--)
        {
            if (_is_sync_word(buffer, i))
            {
                break;
            }
            missing--;
        }
        if (missing)
            _data_producer.read(std::data(buffer), missing);
    }

    auto _wait_for_output_bufer()
    {
        auto out = samples.get_new(std::chrono::milliseconds(1));
        out.has_value();
        do
        {
            if (out.has_value())
            {
                return out;
            }
            out = samples.get_new(std::chrono::milliseconds(1));
        } while (!out.has_value() && is_running());
        return out;
    }

    simple_decoder() = default;

    auto_recycled_channel<DynamicArray2D<uint16_t>, 64, channels::full_policy::overwrite_last>
        samples;

public:
    auto& data_producer() const { return _data_producer; }

    /** Construct a simple decoder.
     *  @param producer The data producer, it must implement the data_producer concept.
     */
    simple_decoder(std::size_t samples_count, data_producer_t&& producer)
            : _data_producer(std::move(producer))
    {
        _bytes_per_packet = (channels_count + 2) * _bytes_per_sample;
        _bytes_per_window = _bytes_per_packet * samples_count;
        _window_size = samples_count;
        for (auto i = 0UL; i < decltype(samples)::max_size; i++)
        {
            samples.recycle({ samples_count, channels_count +1});
        }
    }

    /** Stop the decoder thread.
     *  @see start
     */
    inline void stop()
    {
        _running.store(false);
        if (_thread.joinable())
        {
            _thread.join();
        }
    }

    std::size_t get_raw_data(char* buffer, std::size_t count)
    {
        bool running = _running.load();
        if (running)
            stop();
        const auto got = _data_producer.read(buffer, count);
        if (running)
        {
            start();
        }
        return got;
    }

    /** Start the decoder in a new thread.
     *  @see stop
     */
    inline void start() { _thread = std::thread(&simple_decoder::run, this); }

    inline bool is_running() const { return _running.load(); }

    /** Run the decoder in an infinite loop. Prefer using start instead.
     *  @note This function is blocking.
     *  @see start
     */
    inline void run()
    {
        using namespace std::chrono_literals;
        std::vector<char> _buffer;
        _buffer.resize(_bytes_per_window*2);
        auto consume_data = [&]()->std::size_t
        {
            std::size_t got = _data_producer.read(std::data(_buffer), _bytes_per_window);
            if (!_is_sync_word(_buffer, 0))
            {
                _resync(_buffer);
                got = _data_producer.read(std::data(_buffer), _bytes_per_window);
            }
            if (got < _bytes_per_packet)
            {
                return std::size_t{0};
            }
            return got;
        };

        auto out_cursor = 0UL;
        auto in_cursor = 0UL;
        auto got = consume_data();
        auto out = _wait_for_output_bufer();
        do
        {
            if (got > _bytes_per_packet && _is_sync_word(_buffer, 0))
            {
                if (!out.has_value())
                {
                    return;
                }
                for (auto parsed_packets = 0UL;
                    parsed_packets < std::min(got / _bytes_per_packet, _window_size);)
                {
                    auto& out_ref = *out;
                    std::memcpy(&out_ref[{ out_cursor, 0 }],
                        std::data(_buffer) + in_cursor + _sync_word_bytes,
                        _bytes_per_packet - _sync_word_bytes);
                    parsed_packets += 1;
                    in_cursor += _bytes_per_packet;
                    out_cursor++;
                }
                if (out_cursor >= _window_size)
                {
                    out_cursor = 0;
                    out = _wait_for_output_bufer();
                }
                if (in_cursor >= got)
                {
                    in_cursor = 0;
                    got = consume_data();
                }
            }
            else
            {
                _resync(_buffer);
            }

        } while (is_running());
    }

    auto get_samples(std::optional<std::chrono::nanoseconds> timeout_ns = std::nullopt)
    {
        return samples.take(timeout_ns);
    }
};

template <std::size_t channels_count, data_producer data_producer_t>
auto make_simple_decoder(std::size_t samples_count , data_producer_t&& producer)
{
    return simple_decoder<channels_count, data_producer_t>(samples_count , std::move(producer));
}
