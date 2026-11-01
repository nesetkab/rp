#pragma once
#include <filesystem>
#include <string>
#include <vector>

// rp --demo makes a throwaway pico project in /tmp with a fake mpremote,
// so you can try rp without a real board. the folder is deleted on exit.
class DemoProject {
public:
  DemoProject();
  ~DemoProject();
  DemoProject(const DemoProject &) = delete;
  DemoProject &operator=(const DemoProject &) = delete;

  std::string configPath() const;
  std::vector<std::string> welcomeLines() const;

private:
  std::filesystem::path dir;
};
