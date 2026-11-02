#include "Config.hpp"
#include "AppState.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

using json = nlohmann::json;
namespace fs = std::filesystem;

#ifdef __APPLE__
const std::string defaultPrefix = "cu.usbmodem";
#else
const std::string defaultPrefix = "ttyACM";
#endif

std::string defaultConfig() {
  return R"JSON({
  "devicePrefix": ")JSON" +
         defaultPrefix + R"JSON(",
  "actions": [
    { "label": "run",    "key": "r", "command": "PYTHONUNBUFFERED=1 mpremote run main.py" },
    { "label": "deploy", "key": "d", "command": "mpremote fs cp main.py :main.py", "kind": "deploy" },
    { "label": "repl",   "key": "p", "command": "mpremote repl", "kind": "interactive" },
    { "label": "ls",     "key": "l", "command": "mpremote fs ls" },
    { "label": "reset",  "key": "b", "command": "mpremote reset" },
    { "label": "stop",   "key": "s", "kind": "stop" }
  ]
}
)JSON";
}

fs::path globalConfigPath() {
  const char *xdg = std::getenv("XDG_CONFIG_HOME");
  if (xdg != nullptr && *xdg != '\0') {
    return fs::path(xdg) / "rp" / "config.json";
  }
  const char *home = std::getenv("HOME");
  if (home == nullptr || *home == '\0') {
    throw std::runtime_error(
        "config error: HOME is not set, so there is no default config");
  }
  return fs::path(home) / ".config" / "rp" / "config.json";
}

std::string findConfig() {
  std::error_code ec;
  if (fs::exists("config.json", ec)) {
    return "config.json";
  }
  fs::path global = globalConfigPath();
  if (!fs::exists(global, ec)) {
    fs::create_directories(global.parent_path(), ec);
    std::ofstream out(global);
    if (!out) {
      throw std::runtime_error("config error: could not write default config \"" +
                               global.string() + "\"");
    }
    out << defaultConfig();
  }
  return global.string();
}

std::vector<Action> parseActions(const json &config) {
  const std::map<std::string, ActionKind> kinds = {
      {"normal", ActionKind::Normal},
      {"interactive", ActionKind::Interactive},
      {"stop", ActionKind::Stop},
      {"deploy", ActionKind::Deploy}};
  std::vector<Action> actions;
  for (const json &item : config.at("actions")) {
    std::string label = item.at("label").get<std::string>();
    std::string keyS = item.at("key").get<std::string>();
    if (keyS.size() != 1) {
      throw std::runtime_error("config error: \"" + label +
                               "\" should only have 1 letter for key");
    } else if (keyS[0] == 'q' || keyS[0] == 'R') {
      throw std::runtime_error(
          "config error: \"" + label +
          "\" shouldn't use the letter q (quit) or R (reload config)");
    }
    std::string command = item.value("command", "");
    std::string kindS = item.value("kind", "normal");
    auto found = kinds.find(kindS);
    if (found == kinds.end()) {
      std::string accK;
      for (const auto &pair : kinds)
        accK += accK.empty() ? pair.first : ", " + pair.first;
      throw std::runtime_error("config error: \"" + label +
                               "\" has an incorrect kind \"" + kindS +
                               "\". accepted kinds are: " + accK);
    }
    ActionKind kind = found->second;
    actions.push_back({label, keyS[0], command, kind});
  }
  return actions;
}

std::string getPrefix(const json &config) {
  std::string prefix = config.at("devicePrefix").get<std::string>();
  return prefix;
}

Config loadConfig(const std::string &path) {
  try {
    std::ifstream in(path);
    if (!in) {
      throw std::runtime_error("config error: could not open config file \"" +
                               path + "\"");
    }
    json config = json::parse(in);
    return {parseActions(config), getPrefix(config),
            config.value("deviceDir", "/dev")};
  } catch (const nlohmann::json::exception &e) {
    throw std::runtime_error(std::string("config error: ") + e.what());
  }
}
