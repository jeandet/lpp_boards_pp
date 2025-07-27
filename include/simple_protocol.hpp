#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "concepts.hpp"
#include <channels/channels.hpp>
#include <cstddef>
#include "auto_recycled_channel.hpp"

void _next(pointer_to_contiguous_memory auto& output, auto&... outputs)
{
    if constexpr (sizeof...(outputs))
    {
        _next(outputs...);
    }
    output += 1;
}

void _extract(const char* const buffer, std::size_t index, pointer_to_contiguous_memory auto output,
    auto&&... outputs)
{
    *output
        = static_cast<uint16_t>(buffer[index]) + (static_cast<uint16_t>(buffer[index + 1]) << 8);
    if constexpr (sizeof...(outputs))
    {
        _extract(buffer, index + 2, outputs...);
    }
}

inline bool _is_sync_word(const auto& buffer, std::size_t index)
{
    return (static_cast<unsigned char>(buffer[index]) == 0xf0)
        && (static_cast<unsigned char>(buffer[index + 1]) == 0x0f);
}

/** Simple decoder for a simple protocol.
 *  @tparam channels_count The number of channels in the protocol.
 *  @tparam window_size The number of samples in a window.
 *  @tparam data_producer_t The data producer type.
 */
template <std::size_t channels_count, std::size_t window_size, data_producer data_producer_t>
class simple_decoder
{
    constexpr static auto bytes_per_sample = 2;
    constexpr static auto bytes_per_packet = (channels_count + 2) * bytes_per_sample;
    constexpr static auto bytes_per_window = window_size * bytes_per_packet;
    std::array<char, bytes_per_window * 2> _buffer;
    std::size_t _buffer_start_index = 0;
    std::size_t _buffer_stop_index = 0;
    data_producer_t _data_producer;
    std::atomic<bool> _running = true;
    std::thread _thread;

    void _resync()
    {
        auto missing = (2 * bytes_per_packet) - 2;
        for (auto i = bytes_per_window - 2; missing; i--)
        {
            if (_is_sync_word(_buffer, i))
            {
                break;
            }
            missing--;
        }
        if (missing)
            _data_producer.read(std::data(_buffer), missing);
    }

    simple_decoder() = default;

    auto_recycled_channel<std::array<std::array<int16_t, window_size>, channels_count + 1>, 8,
            channels::full_policy::overwrite_last>
            samples;

public:

    auto& data_producer() const { return _data_producer; }

    /** Construct a simple decoder.
     *  @param producer The data producer, it must implement the data_producer concept.
     */
    simple_decoder(data_producer_t&& producer) : _data_producer(std::move(producer))
    {
        for (auto i = 0UL; i < 8; i++)
        {
            samples.recycle({});
        }
    }


    /** Stop the decoder thread.
     *  @see start
     */
    inline void stop()
    {
        _running.store(false);
        if (_thread.joinable()) {
            _thread.join();
        }
    }

    /** Decode a buffer of samples according to the simple protocol.
     *  @param buffer The buffer to decode.
     *  @param input_bytes The number of bytes in the input buffer.
     *  @param outputs_capacity The number of samples that can be stored in the outputs.
     *  @param outputs The outputs where the decoded samples will be stored.
     *  @return A tuple with the number of samples decoded and the number of bytes consumed.
     *  @note The number of outputs must be equal to the number of channels plus one. The first
     * output is the sample counter.
     */
    static inline std::tuple<std::size_t, std::size_t> decode(const char* const buffer,
        std::size_t input_bytes, std::size_t outputs_capacity, auto&&... outputs)
    {
        static_assert(sizeof...(outputs) == channels_count + 1);
        std::size_t decoded_samples = 0;
        std::size_t input_index = 0;
        while (
            input_index < (input_bytes - bytes_per_packet) && !_is_sync_word(buffer, input_index))
        {
            input_index++;
        }
        decoded_samples
            = std::min((input_bytes - input_index) / bytes_per_packet, outputs_capacity);
        for (auto i = 0UL; i < decoded_samples; i++)
        {
            _extract(buffer + input_index, 0, outputs...);
            _next(outputs...);
            input_index += bytes_per_packet;
        }
        return { decoded_samples, input_index + 1 };
    }

    /** Start the decoder in a new thread.
     *  @see stop
     */
    inline void start() { _thread = std::thread(&simple_decoder::run, this); }

    /** Run the decoder in an infinite loop. Prefer using start instead.
     *  @note This function is blocking.
     *  @see start
     */
    inline void run()
    {
        while (_running.load())
        {
            auto got = _data_producer.read(std::data(_buffer), bytes_per_window);
            if (got == bytes_per_window && _is_sync_word(_buffer, 0))
            {
                while(_running.load())
                {
                    if (auto out =samples.get_new(1000000); out.has_value())
                    {
                        auto& out_ref = *out;
                        auto [decoded_sample, consumed_bytes]
                            = [&]<std::size_t... Is>(std::index_sequence<Is...>)
                        {
                            return simple_decoder::decode(
                                std::data(_buffer), got, window_size, std::data(out_ref[Is])...);
                        }(std::make_index_sequence<std::tuple_size<std::decay_t<decltype(out_ref)>> {}> {});
                        break;
                    }
                }
            }
            else
            {
                _resync();
            }
        }
    }

    auto get_samples(int timeout_ns = -1)
    {
        return samples.take(timeout_ns);
    }
};

template <std::size_t channels_count, std::size_t window_size, data_producer data_producer_t>
auto make_simple_decoder(data_producer_t&& producer)
{
    return simple_decoder<channels_count, window_size, data_producer_t>(std::move(producer));
}
