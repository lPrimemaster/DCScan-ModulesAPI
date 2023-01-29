#include "../include/internal.h"
#include <algorithm>

DCS::Core::PID::PID(float min, float max, float Kp, float Kd, float Ki) : min(min), max(max), Kp(Kp), Kd(Kd), Ki(Ki)
{
    le = 0.0f;
    integral = 0.0f;

    setTargetAndBias(0.0f, 0.0f);
}

void DCS::Core::PID::setTargetAndBias(float target, float bias)
{
    if(target > max || target < min)
    {
        LOG_ERROR("Cannot set a PID target value out of the specified bounds.");
        LOG_WARNING("Setting target to '0.0'.");
        LOG_WARNING("Setting   bias to '0.0'.");
        this->target = 0.0f;
        this->bias = 0.0f;;
        return;
    }

    this->target = target;
    this->bias = bias;
}

float DCS::Core::PID::calculate(float value)
{
    auto p = std::chrono::steady_clock::now();
    float dt = std::chrono::duration_cast<std::chrono::milliseconds>(p - last_point).count() / 1000.0f;
    last_point = p;
    return calculate(value, dt);
}
			
float DCS::Core::PID::calculate(float value, float dt)
{
    float e = target - value;
    float P = Kp * e;
    float I = Ki * (integral += e * dt);
    float D = Kd * ((e - le) / dt);
    le = e;
    return std::clamp(P + I + D + bias, min, max);
}
