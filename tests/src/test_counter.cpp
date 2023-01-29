#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../../DCS_Utils/include/internal.h"
#include "../../DCS_Core/include/DCS_ModuleCore.h"
#include "../include/DCS_Assert.h"

#include <numeric>

#define NTESTS 1000
#define REPEATS 100

using namespace DCS;

template<typename T>
f64 stdDev(f64 mu, std::vector<T> vals)
{
    f64 devSum = 0.0;
    for(int i = 0; i < vals.size(); i++)
    {
        devSum += std::pow(vals[i] - mu, 2.0);
    }

    return std::sqrt(devSum / NTESTS);
}

int main()
{
    using namespace DCS::Math;
    using namespace DCS::Timer;

    std::vector<f64> total_avgs;
    std::vector<f64> total_devs;

    DCS_START_TEST;


    for(int j = 0; j < REPEATS; j++)
    {
        std::vector<i64> total_nanos;
        total_nanos.reserve(NTESTS);

        for(int t = 0; t < NTESTS; t++)
        {
            //t = 201;
            FILE* f = fopen(("count_test_buffers/c_" + std::to_string(t) + ".txt").c_str(), "r");

            char buffer[64];

            DCS::f64 points[1024];

            int i = 0;

            while(fgets(buffer, 64, f))
            {
                points[i++] = atof(buffer);
            }

            fclose(f);

            SystemTimer timer;

            timer.start();
            CountResult r =  countArrayPeak(points, 1000, 0.01, 0.99, 0);
            i64 ns = timer.getNanoseconds();

            total_nanos.push_back(ns);

            //LOG_DEBUG("Test #%d - [%d ns]", t, ns);
            DCS_ASSERT_EQ(r.num_detected, points[i-1]);

            //t = 2000;
        }
        
        DCS_ASSERT_EQ(total_nanos.size(), NTESTS);
        f64 ns_avg = std::accumulate(total_nanos.begin(), total_nanos.end(), 0.0) / (f64)NTESTS;
        LOG_DEBUG("Avg: %lf ms || Dev: %lf ms", ns_avg / 1000.0, stdDev(ns_avg, total_nanos) / 1000.0);

        total_avgs.push_back(ns_avg);
        total_devs.push_back(stdDev(ns_avg, total_nanos));
    }

    puts("\n\n\n\n\n");

    f64 t_ns_avg = std::accumulate(total_avgs.begin(), total_avgs.end(), 0.0) / (f64)REPEATS;
    f64 t_ns_dev = std::accumulate(total_devs.begin(), total_devs.end(), 0.0) / (f64)REPEATS;
    LOG_DEBUG("==== TOTAL RESULTS ====");
    LOG_DEBUG("Avg: %lf ms || Dev: %lf ms", t_ns_avg / 1000.0, t_ns_dev / 1000.0);

    DCS_RETURN_TEST;
}
