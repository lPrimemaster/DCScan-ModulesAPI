#include "../../DCS_Core/include/DCS_ModuleCore.h"
#include "../include/DCS_Assert.h"

#define NTESTS 1000

int main()
{
    using namespace DCS::Math;


    DCS_START_TEST;

    for(int t = 0; t < NTESTS; t++)
    {
        t = 201;
        FILE* f = fopen(("count_test_buffers/c_" + std::to_string(t) + ".txt").c_str(), "r");

        char buffer[64];

        DCS::f64 points[1024];

        int i = 0;

        while(fgets(buffer, 64, f))
        {
            points[i++] = atof(buffer);
        }

        fclose(f);

        // f64* arr, u64 size, f64 vlo, f64 vhi, f64 vth
        CountResult r =  countArrayPeak(points, 1000, 0.25, 0.3, 0);

        LOG_DEBUG("Test #%d", t);
        DCS_ASSERT_EQ(r.num_detected, points[i-1]);

        for(int d = 0; d < r.num_detected; d++)
        {
            LOG_DEBUG("maximizer - %d", r.maximizers[d]);
        }

        t = 2000;
    }

    DCS_RETURN_TEST;
}