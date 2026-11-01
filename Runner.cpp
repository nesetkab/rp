#include "Runner.hpp"
#include "AppState.hpp"
#include <atomic>
#include <cstdio>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <filesystem>
#include <mutex>
#include <ncurses.h>
#include <spawn.h>
#include <string>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

extern char **environ;

std::string nowText() {
  std::time_t now = std::time(nullptr);
  std::tm local{};
  localtime_r(&now, &local);
  char buffer[16];
  std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &local);
  return buffer;
}

void setResult(SharedState &sharedState, const std::string &label, bool passed,
               bool isDeploy,
               std::filesystem::file_time_type deployTime =
                   std::filesystem::file_time_type{}) {
  std::lock_guard<std::mutex> lock(sharedState.mutex);
  sharedState.result.resultPassed = passed;
  if (isDeploy && passed) {
    sharedState.lastDeploy = deployTime;
  }
  sharedState.result.resultLabel = label;
  sharedState.result.resultTime = nowText();
}

void addLine(SharedState &sharedState, const std::string &line) {
  const std::size_t maxLines = 1000;
  std::lock_guard<std::mutex> lock(sharedState.mutex);
  sharedState.lines.push_back(line);
  if (sharedState.lines.size() > maxLines) {
    sharedState.lines.erase(sharedState.lines.begin());
  }
}

pid_t spawnCommand(const std::string &command, int outFd) {
  posix_spawn_file_actions_t actions;
  posix_spawn_file_actions_init(&actions);
  posix_spawn_file_actions_addopen(&actions, STDIN_FILENO, "/dev/null",
                                   O_RDONLY, 0);
  posix_spawn_file_actions_adddup2(&actions, outFd, STDOUT_FILENO);
  posix_spawn_file_actions_adddup2(&actions, outFd, STDERR_FILENO);

  posix_spawnattr_t attr;
  posix_spawnattr_init(&attr);
  posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETPGROUP);
  posix_spawnattr_setpgroup(&attr, 0);

  std::string shell = "/bin/sh", flag = "-c", cmd = command;
  char *argv[] = {shell.data(), flag.data(), cmd.data(), nullptr};
  pid_t pid = 0;
  int err = posix_spawn(&pid, "/bin/sh", &actions, &attr, argv, environ);

  posix_spawnattr_destroy(&attr);
  posix_spawn_file_actions_destroy(&actions);
  return err == 0 ? pid : -1;
}

bool runNormalAction(SharedState &sharedState, const std::string &command) {
  int fds[2];
  if (pipe(fds) != 0) {
    addLine(sharedState, "ERROR: could not open a pipe. command did not start");
    return false;
  }
  fcntl(fds[0], F_SETFD, FD_CLOEXEC);
  fcntl(fds[1], F_SETFD, FD_CLOEXEC);

  pid_t pid = spawnCommand(command, fds[1]);
  close(fds[1]);
  if (pid < 0) {
    close(fds[0]);
    addLine(sharedState, "ERROR: command did not start");
    return false;
  }
  sharedState.pid = pid;

  FILE *out = fdopen(fds[0], "r");
  char buffer[512];
  while (out != nullptr && std::fgets(buffer, sizeof(buffer), out) != nullptr) {
    std::string line = buffer;
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
      line.pop_back();
    }
    addLine(sharedState, line);
  }
  if (out != nullptr) {
    std::fclose(out);
  } else {
    close(fds[0]);
  }

  int status = 0;
  waitpid(pid, &status, 0);
  sharedState.pid = 0;
  if (WIFSIGNALED(status)) {
    addLine(sharedState, "stopped by signal " + std::to_string(WTERMSIG(status)));
    addLine(sharedState, " ");
    return false;
  }
  int exitcode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
  addLine(sharedState, "finished! exit code " + std::to_string(exitcode));
  addLine(sharedState, " ");

  return exitcode == 0;
}

bool runInteractiveAction(const std::string &command) {
  def_prog_mode();
  endwin();
  int status = std::system(command.c_str());
  reset_prog_mode();
  clearok(curscr, TRUE);
  int exitcode = -1;
  if (WIFEXITED(status)) {
    exitcode = WEXITSTATUS(status);
  }

  return exitcode == 0;
}

void stopCommand(SharedState &sharedState) {
  if (!sharedState.busy) {
    addLine(sharedState, "nothing to stop");
    return;
  }
  pid_t pid = sharedState.pid;
  if (pid > 0 && kill(-pid, SIGTERM) == 0) {
    addLine(sharedState, "stopping...");
  } else {
    addLine(sharedState, "could not stop the command");
  }
}

void selectAnAction(SharedState &sharedState,
                    const std::vector<Action> &actions, int selAction,
                    std::thread &worker) {
  std::string command = actions.at(selAction).command;
  std::string label = actions.at(selAction).label;
  ActionKind actionKind = actions.at(selAction).actionKind;

  if (actionKind == ActionKind::Stop) {
    stopCommand(sharedState);
    return;
  } else if (sharedState.busy) {
    addLine(sharedState, "busy, wait for the current command");
    return;
  } else if (actionKind == ActionKind::Interactive) {
    addLine(sharedState, label + " opened, exit with ctrl-x or ctrl-]");
    bool resultPassed = runInteractiveAction(command);
    setResult(sharedState, label, resultPassed, false);
    return;
  }

  addLine(sharedState, command);
  if (worker.joinable()) {
    worker.join();
  }

  std::filesystem::file_time_type deployTime =
      std::filesystem::file_time_type::clock::now();
  sharedState.busy = true;

  worker =
      std::thread([&sharedState, command, label, deployTime, actionKind]() {
        bool resultPassed = runNormalAction(sharedState, command);
        setResult(sharedState, label, resultPassed,
                  actionKind == ActionKind::Deploy, deployTime);
        sharedState.busy = false;
      });
}
