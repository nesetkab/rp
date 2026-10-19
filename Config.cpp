#include "Config.hpp"
#include "AppState.hpp"
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

using json = nlohmann::json;

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
    } else if (keyS[0] == 'q') {
      throw std::runtime_error("config error: \"" + label +
                               "\" shouldn't use the letter q. q is quit.");
    }
    std::string command = item.value("command", "");
    std::string kindS = item.value("kind", "normal");
    auto found = kinds.find(kindS);
    if (found == kinds.end()) {
      std::string accK;
      for (const auto &pair : kinds)
        accK += pair.first + ", ";
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
    return {parseActions(config), getPrefix(config)};
  } catch (const nlohmann::json::exception &e) {
    throw std::runtime_error(std::string("config error: ") + e.what());
  }
}
