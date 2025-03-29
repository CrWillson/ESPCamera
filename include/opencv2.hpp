#ifndef OPENCV2_HPP
#define OPENCV2_HPP

// Disable warnings
#ifdef _MSC_VER // For Microsoft Visual C++
    #pragma warning(push)
    #pragma warning(disable : 4996) // Example: Disable deprecated function warnings
#elif defined(__GNUC__) || defined(__clang__) // For GCC and Clang
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wall"
    #pragma GCC diagnostic ignored "-Wextra"
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#endif

// Include OpenCV headers
#undef EPS
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#define EPS 192

// Restore warnings
#ifdef _MSC_VER
    #pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#endif

#endif // OPENCV2_HPP
