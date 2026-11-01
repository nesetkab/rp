#pragma once
#include <filesystem>
#include <string>
#include <vector>

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
