#include "LLDB.h"

std::unique_ptr<Logger> Logger::s_instance;
std::mutex Logger::s_mutex;
