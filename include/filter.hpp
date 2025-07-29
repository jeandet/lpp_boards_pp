#pragma once
#include <channels/channels.hpp>
#include "auto_recycled_channel.hpp"
#include "array2d.hpp"

class Filter
{
    /*
     * generated with:
     *
     * from scipy import signal
     * sos=signal.iirfilter(6, 0.25, rs=80, btype='lowpass',analog=False,
     * ftype='cheby2',output='sos') print(f""" const double b[{sos.shape[0]}][3] = {{{ ', '.join([
     * '{' + ', '.join(list(map(str , stage))) + '}' for stage in sos[:,:3] ])}}}; const double
     * a[{sos.shape[0]}][3] = {{{ ', '.join([ '{' + ', '.join(list(map(str , stage))) + '}' for
     * stage in sos[:,3:] ])}}};
     * """)
     */
    const double b[3][3]
        = { { 0.0003093761776877881, 0.00027126310014594703, 0.00030937617768778814 },
              { 1.0, -0.9780833528217364, 1.0 }, { 1.0, -1.3786886998937251, 1.0 } };
    const double a[3][3] = { { 1.0, -1.449543617902121, 0.5298911166658338 },
        { 1.0, -1.570227988783793, 0.6515750588208723 },
        { 1.0, -1.7779954896683987, 0.8644540496942458 } };
    double ctx[3][3] = { { 0. } };

public:
    inline explicit Filter() { }
    inline ~Filter() { }

    inline double filter(double x)
    {
        // Direct-Form-II
        for (int i = 0; i < 3; i++)
        {
            double W = (x - (a[i][1] * ctx[i][0]) - (a[i][2] * ctx[i][1]));
            x = (b[i][0] * W) + (b[i][1] * ctx[i][0]) + (b[i][2] * ctx[i][1]);
            ctx[i][1] = ctx[i][0];
            ctx[i][0] = W;
        }
        return x;
    }
};


template <typename source_t, std::size_t channel_count>
struct MultiChannelFilter
{


    inline source_t filter(const source_t& value, std::size_t channel)
    {
        if (channel >= channel_count)
        {
            throw std::out_of_range("Channel index out of range");
        }
        return filters[channel].filter(value);
    }

    inline void reset()
    {
        for (auto& filter : filters)
        {
            filter = Filter();
        }
    }    
    private:
        std::array<Filter, channel_count> filters;
        source_t& source;
};
