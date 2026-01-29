/**
@filename framework_bootstrap_log.h
@brief Declaration of the bootstrap logging function used before Logger initialization or during catastrophic failures.
@author Kooler Fan
**/
#pragma once

#include <source_location>
#include <string_view>

void FrameworkBootstrapLog(std::string_view message, const std::source_location& loc = std::source_location::current()) noexcept;
