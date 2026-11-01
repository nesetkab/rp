#include "Demo.hpp"
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <system_error>

namespace fs = std::filesystem;

namespace {

const char *demoConfig = R"SH({
  "devicePrefix": "ttyACM",
  "deviceDir": ".dev",
  "actions": [
    { "label": "run",    "key": "r", "command": "PYTHONUNBUFFERED=1 mpremote run main.py" },
    { "label": "deploy", "key": "d", "command": "mpremote fs cp main.py :main.py", "kind": "deploy" },
    { "label": "repl",   "key": "p", "command": "mpremote repl", "kind": "interactive" },
    { "label": "ls",     "key": "l", "command": "mpremote fs ls" },
    { "label": "reset",  "key": "b", "command": "mpremote reset" },
    { "label": "stop",   "key": "s", "kind": "stop" },
    { "label": "edit",   "key": "e", "command": "demo-edit" },
    { "label": "unplug", "key": "u", "command": "demo-unplug" }
  ]
}
)SH";

const char *demoMain = R"SH(from machine import ADC, Pin
import time

led = Pin("LED", Pin.OUT)
sensor = ADC(4)

while True:
    led.toggle()
    volts = sensor.read_u16() * 3.3 / 65535
    temp = 27 - (volts - 0.706) / 0.001721
    print("temp: {:.1f} C".format(temp))
    time.sleep(1)
)SH";

const char *demoBoot = R"SH(# boot.py -- run on boot-up
)SH";

// a pretend mpremote. it acts like a pico is plugged in while
// .dev/ttyACM0 exists, and keeps the "board" files in .board/
const char *fakeMpremote = R"SH(#!/bin/sh
here=$(cd "$(dirname "$0")/.." && pwd)
board="$here/.board"

if [ ! -e "$here/.dev/ttyACM0" ]; then
  echo "mpremote: no device found" >&2
  exit 1
fi

[ "$1" = "fs" ] && shift

case "$1" in
  run)
    echo "running $2 on the pico (demo stops after 20 s, press s to stop now)"
    i=0
    while [ $i -lt 20 ]; do
      i=$((i + 1))
      awk -v i="$i" 'BEGIN { printf "temp: %.1f C\n", 24.6 + 1.3 * sin(i / 3) + (i * 7 % 5) / 10 }'
      sleep 1
    done
    ;;
  cp)
    src="$2"
    dst=$(echo "$3" | sed 's/^://')
    echo "cp $src :$dst"
    sleep 1
    cp "$here/$src" "$board/$dst" || exit 1
    echo "copied $(wc -c < "$here/$src" | tr -d ' ') bytes"
    ;;
  ls)
    echo "ls :"
    for f in "$board"/*; do
      [ -e "$f" ] || continue
      printf "%9d %s\n" "$(wc -c < "$f" | tr -d ' ')" "$(basename "$f")"
    done
    ;;
  reset)
    echo "resetting the board..."
    sleep 1
    echo "MPY: soft reboot"
    ;;
  devs)
    echo "/dev/ttyACM0 e6614c311b7e5a2f 2e8a:0005 MicroPython Board in FS mode"
    ;;
  repl)
    esc=$(printf '\035')
    cx=$(printf '\030')
    echo "Connected to MicroPython at /dev/ttyACM0"
    echo "Use Ctrl-] or Ctrl-x then Enter to exit this shell (Ctrl-D also works)"
    echo "MicroPython v1.24.1 on 2024-11-29; Raspberry Pi Pico with RP2040"
    echo 'Type "help()" for more information.'
    while printf '>>> ' && IFS= read -r line; do
      case "$line" in
        *"$esc"*|*"$cx"*|"exit()"|"quit()") break ;;
        "") ;;
        "help()")
          echo "This is a pretend MicroPython REPL for the rp demo."
          echo "Try: print(\"hi\"), 2 + 3 * 4, import machine, exit()"
          ;;
        import\ *|from\ *) ;;
        print\(*\))
          inner=$(echo "$line" | sed 's/^print(//; s/)$//')
          case "$inner" in
            \"*\"|\'*\') echo "$inner" | sed 's/^.//; s/.$//' ;;
            *) echo "$inner" | grep -Eq '^[0-9+*/(). -]+$' && awk "BEGIN { print $inner }" || echo "$inner" ;;
          esac
          ;;
        *)
          if echo "$line" | grep -Eq '^[0-9+*/(). -]+$'; then
            awk "BEGIN { print $line }"
          else
            name=$(echo "$line" | sed 's/[^A-Za-z0-9_].*//')
            echo "Traceback (most recent call last):"
            echo '  File "<stdin>", line 1, in <module>'
            echo "NameError: name '${name:-$line}' isn't defined"
          fi
          ;;
      esac
    done
    echo
    ;;
  *)
    echo "mpremote (demo): this command is not part of the demo: $*" >&2
    exit 1
    ;;
esac
)SH";

const char *fakeEdit = R"SH(#!/bin/sh
here=$(cd "$(dirname "$0")/.." && pwd)
echo "# edited at $(date +%H:%M:%S)" >> "$here/main.py"
echo "main.py changed. the status bar now says it is not deployed"
)SH";

const char *fakeUnplug = R"SH(#!/bin/sh
here=$(cd "$(dirname "$0")/.." && pwd)
dev="$here/.dev/ttyACM0"
if [ -e "$dev" ]; then
  rm "$dev"
  echo "pico unplugged. press u again to plug it back in"
else
  touch "$dev"
  echo "pico plugged in"
fi
)SH";

void writeFile(const fs::path &path, const std::string &text,
               bool executable = false) {
  std::ofstream out(path);
  if (!out) {
    throw std::runtime_error("demo: could not write " + path.string());
  }
  out << text;
  out.close();
  if (executable) {
    chmod(path.c_str(), 0755);
  }
}

} // namespace

DemoProject::DemoProject() {
  char tmpl[] = "/tmp/rp-demo-XXXXXX";
  if (mkdtemp(tmpl) == nullptr) {
    throw std::runtime_error("demo: could not make a folder in /tmp");
  }
  dir = fs::canonical(tmpl);
  fs::create_directories(dir / ".bin");
  fs::create_directories(dir / ".dev");
  fs::create_directories(dir / ".board");

  writeFile(dir / "config.json", demoConfig);
  writeFile(dir / "main.py", demoMain);
  writeFile(dir / ".board" / "boot.py", demoBoot);
  writeFile(dir / ".dev" / "ttyACM0", "");
  writeFile(dir / ".bin" / "mpremote", fakeMpremote, true);
  writeFile(dir / ".bin" / "demo-edit", fakeEdit, true);
  writeFile(dir / ".bin" / "demo-unplug", fakeUnplug, true);

  // put the fake mpremote first on PATH so commands find it
  const char *oldPath = std::getenv("PATH");
  std::string path = (dir / ".bin").string();
  if (oldPath != nullptr && *oldPath != '\0') {
    path += ":" + std::string(oldPath);
  }
  setenv("PATH", path.c_str(), 1);
  fs::current_path(dir);
}

DemoProject::~DemoProject() {
  std::error_code ec;
  fs::current_path("/tmp", ec);
  fs::remove_all(dir, ec);
}

std::string DemoProject::configPath() const {
  return (dir / "config.json").string();
}

std::vector<std::string> DemoProject::welcomeLines() const {
  return {"welcome to the rp demo! a pretend pico is plugged in,",
          "so you do not need any hardware.",
          " ",
          "try these keys:",
          "  r  run main.py and watch the output stream in",
          "  s  stop the running command",
          "  d  deploy main.py to the board",
          "  e  edit main.py (status bar shows 'changed, not deployed')",
          "  u  unplug or plug in the pico (watch the sparkles change color)",
          "  p  open the repl (type exit() to come back)",
          "  q  quit",
          " ",
          "demo files are in " + dir.string() + " and get deleted on quit",
          " "};
}
