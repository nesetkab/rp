#include "StatusCheck.hpp"
#include <string>

void checkDeviceFound(Status &status, const std::string &devicePrefix) {
  namespace fs = std::filesystem;
  fs::path dev = "/dev";
  status.deviceFound = false;
  std::error_code ec;
  fs::directory_iterator it(dev, ec);
  if (ec)
    return;
  for (const fs::directory_entry &entry : it) {
    if (entry.path().filename().string().rfind(devicePrefix, 0) == 0) {
      status.deviceFound = true;
      break;
    }
  }
}

void checkFilesChanged(Status &status,
                       std::filesystem::file_time_type deployTime) {
  status.filesFound = FilesState::Unknown;
  if (deployTime == std::filesystem::file_time_type{}) {
    return;
  }
  namespace fs = std::filesystem;
  std::error_code ec;
  fs::recursive_directory_iterator it(status.path, ec);
  if (ec)
    return;
  for (const fs::directory_entry &entry : it) {
    if (entry.is_regular_file() && entry.path().extension() == ".py" &&
        entry.last_write_time(ec) > deployTime) {
      status.filesFound = FilesState::Changed;
      return;
    }
  }
  status.filesFound = FilesState::Deployed;
}

void checkStatus(Status &status, SharedState &sharedState,
                 const std::string &devicePrefix) {
  auto now = std::chrono::steady_clock::now();
  auto elapsed = now - status.lastCheck;
  if (elapsed >= std::chrono::seconds(1)) {
    status.lastCheck = now;
    checkDeviceFound(status, devicePrefix);
    std::filesystem::file_time_type deployTime;
    {
      std::lock_guard<std::mutex> lock(sharedState.mutex);
      deployTime = sharedState.lastDeploy;
    }
    checkFilesChanged(status, deployTime);
  }
}
