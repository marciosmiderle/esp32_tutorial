#include "LoggerBase.hpp"

size_t LoggerBase::write(uint8_t c) { return write(&c, 1); }

void LoggerBase::update() {};

bool LoggerBase::isEnabled() const { return enabled; }

void LoggerBase::disable() { enabled = false; }

void LoggerBase::enable() { enabled = true; }
