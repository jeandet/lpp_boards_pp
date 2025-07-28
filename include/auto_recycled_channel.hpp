#pragma once

#include "concepts.hpp"
#include <channels/channels.hpp>

template <typename _value_type, typename _chanel_t>
struct auto_recycled_value
{
    using value_type = _value_type;
    using chanel_t = _chanel_t;

    auto_recycled_value(_value_type&& value, _chanel_t* channel)
            : value(std::move(value)), channel(channel)
    {
    }

    auto_recycled_value(const auto_recycled_value&) = delete;

    auto_recycled_value(auto_recycled_value&& other)
            : value(std::move(other.value)), channel(other.channel)
    {
        other.value = std::nullopt; // Prevent recycling of the moved-from value
    }

    auto_recycled_value() : value(std::nullopt), channel{nullptr} { }

    auto_recycled_value& operator=(const auto_recycled_value&) = delete;
    auto_recycled_value& operator=(auto_recycled_value&& other)
    {
        if (this != &other)
        {
            recycle();
            value = std::move(other.value);
            channel = other.channel;
            other.value = std::nullopt; // Prevent recycling of the moved-from value
        }
        return *this;
    }

    operator bool() const noexcept { return has_value(); }

    bool has_value() const noexcept { return value.has_value(); }


    ~auto_recycled_value()
    {
        recycle();
    }

    void recycle()
    {
        if (channel && !channel->closed() && value.has_value())
        {
            channel->add(std::move(*value));
            value = std::nullopt; // Clear the value after recycling
        }
    }

    _value_type& operator*() { return *value; }
    const _value_type& operator*() const { return *value; }

private:
    std::optional<_value_type> value;
    _chanel_t* channel;
};



template <typename _value_type, std::size_t _max_size = CHANNEL_DEFAULT_SIZE,
    typename _full_policy_t = channels::full_policy::wait_for_space>
struct auto_recycled_channel
{
private:
    channels::channel<_value_type, _max_size, _full_policy_t> _channel;
    channels::channel<_value_type, _max_size, _full_policy_t> _recycled;

public:
    using tag = channels::channel_tag;
    using in_value_type = _value_type;
    using out_value_type = _value_type;
    using recycled_value_t = auto_recycled_value<_value_type, decltype(_recycled)>;
    using full_policy_t = _full_policy_t;
    static inline constexpr std::size_t max_size = _max_size;

    auto_recycled_channel() = default;
    auto_recycled_channel(const auto_recycled_channel&) = delete;

    inline auto_recycled_channel& operator<<(in_value_type&& item)
    {
        _channel.add(std::move(item));
        return *this;
    }

    inline auto_recycled_channel& operator<<(const in_value_type& item)
    {
        _channel.add(item);
        return *this;
    }

    inline recycled_value_t take(std::optional<std::chrono::nanoseconds> timeout_ns = std::nullopt)
    {
        if (auto item = _channel.take(timeout_ns))
        {
            return recycled_value_t(std::move(*item), &_recycled);
        }
        else
        {
            return recycled_value_t();
        }
    }

    inline void add(in_value_type&& item) { _channel.add(std::move(item)); }
    inline void add(const in_value_type& item) { _channel.add(item); }

    inline void recycle(in_value_type&& item)
    {
        _recycled.add(std::move(item));
    }

    inline recycled_value_t get_new(std::optional<std::chrono::nanoseconds> timeout_ns = std::nullopt)
    {
        if (auto item = _recycled.take(timeout_ns))
        {
            return recycled_value_t(std::move(*item), &_channel);
        }
        else
        {
            return recycled_value_t();
        }
    }

    inline bool closed() { return _channel.closed(); }
    inline void close() { _channel.close(); }

    inline std::size_t size() const noexcept { return _channel.size(); }
};
