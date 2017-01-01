#ifndef EOMAP_COMMON_SIGNAL_HPP
#define EOMAP_COMMON_SIGNAL_HPP

#include "util/signal.hpp"

template <class... Args>
using sig = sau::signal<Args...>;

#endif // EOMAP_COMMON_SIGNAL_HPP
