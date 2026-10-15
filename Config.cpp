#include "AppState.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>

using json = nlohmann::json;

std::vector<Action> parseActions(std::string path) {
  std::ifstream in(path);
  json config = json::parse(in);
  std::vector<Action> actions;

  for (const json &item : config.at("actions")) {
    std::string label = item.at("label").get<std::string>();
    char key = item.at("key").get<std::string>()[0];
    std::string command = item.at("command").get<std::string>();
    std::string kindS = item.value("kind", "normal");
    ActionKind kind = ActionKind::Normal;
    if (kindS == "interactive") {
      kind = ActionKind::Interactive;
    } else if (kindS == "stop") {
      kind = ActionKind::Stop;
    } else if (kindS == "deploy") {
      kind = ActionKind::Deploy;
    }

    actions.push_back({label, key, command, kind});
  }
  return actions;
}
