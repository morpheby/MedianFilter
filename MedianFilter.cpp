
#include "MedianFilter.h"

template <>
bool MedianFilter<float, double>::is_valid_value(float v)
{
   return !std::isnan(v);
}

template <>
bool MedianFilter<double, double>::is_valid_value(double v)
{
   return !std::isnan(v);
}

template <>
bool MedianFilter<float, float>::is_valid_value(float v)
{
   return !std::isnan(v);
}

template <>
bool MedianFilter<double, float>::is_valid_value(double v)
{
   return !std::isnan(v);
}
