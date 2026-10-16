#include "Config.hpp"
#include "AppState.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>

using json = nlohmann::json;

std::vector<Action> parseActions(const json &config) {
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
    ActionKind kind;
    if (kindS == "interactive") {
      kind = ActionKind::Interactive;
    } else if (kindS == "stop") {
      kind = ActionKind::Stop;
    } else if (kindS == "deploy") {
      kind = ActionKind::Deploy;
    } else if (kindS == "normal") {
      kind = ActionKind::Normal;
    } else {
      throw std::runtime_error("config error: " + label +
                               " has an incorrect kind \"" + kindS +
                               "\". accepted kids are: "
                               "interactive, stop, deploy, and normal.");
    }

    actions.push_back({label, keyS[0], command, kind});
  }
  return actions;
}

std::string getPrefix(const json &config) {
  std::string prefix = config.at("devicePrefix").get<std::string>();
  return prefix;
}

Config loadConfig(const std::string &path) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error("config error: path \"" + path +
                             "\" does not exist");
  }
  json config = json::parse(in);
  return {parseActions(config), getPrefix(config)};
}
