#include "AppState.hpp"
#include <chrono>
#include <filesystem>
#include <mutex>
#include <system_error>

void checkStatus(Status &status, SharedState &sharedState);
