#include "helpers/time_interface.hpp"

time_interface::time_interface(QObject* parent)
    : BaseClock(parent) { }

time_interface::~time_interface() = default;
