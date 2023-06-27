#include "../include/internal.h"
#include <algorithm>

DCS::Core::PID::PID(f64 min, f64 max, f64 Kp, f64 Kd, f64 Ki) : min(min), max(max), Kp(Kp), Kd(Kd), Ki(Ki)
{
    le = 0.0f;
    integral = 0.0f;

    setTargetAndBias(0.0f, 0.0f);
}

void DCS::Core::PID::setTargetAndBias(f64 target, f64 bias)
{
    if(target > max || target < min)
    {
        LOG_ERROR("Cannot set a PID target value out of the specified bounds.");
        LOG_WARNING("Setting target to '0.0'.");
        LOG_WARNING("Setting   bias to '0.0'.");
        this->target = 0.0f;
        this->bias = 0.0f;
        return;
    }

    this->target = target;
    this->bias = bias;
}

DCS::f64 DCS::Core::PID::calculate(f64 value)
{
    auto p = std::chrono::steady_clock::now();
    f64 dt = std::chrono::duration_cast<std::chrono::milliseconds>(p - last_point).count() / 1000.0f;
    last_point = p;
    return calculate(value, dt);
}
			
DCS::f64 DCS::Core::PID::calculate(f64 value, f64 dt)
{
    f64 e = target - value;
    f64 P = Kp * e;
    f64 I = Ki * (integral += (e * dt));
    f64 D = Kd * ((e - le) / dt);
    le = e;
    return std::clamp(P + I + D + bias, min, max);
}
