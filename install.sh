#!/bin/sh
# installs the latest rp release for your computer.
#   curl -fsSL https://github.com/nesetkab/rp/releases/latest/download/install.sh | sh
# set RP_INSTALL_DIR to pick the folder (default: ~/.local/bin)
set -eu

repo="nesetkab/rp"
dir="${RP_INSTALL_DIR:-$HOME/.local/bin}"

case "$(uname -s)" in
  Darwin) asset="rp-macos-universal" ;;
  Linux)
    case "$(uname -m)" in
      x86_64 | amd64) asset="rp-linux-x86_64" ;;
      aarch64 | arm64) asset="rp-linux-arm64" ;;
      *) echo "rp: no prebuilt binary for $(uname -m). build from source: https://github.com/$repo" >&2; exit 1 ;;
    esac
    ;;
  *) echo "rp: no prebuilt binary for $(uname -s). on windows, use WSL." >&2; exit 1 ;;
esac

url="https://github.com/$repo/releases/latest/download/$asset.tar.gz"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

echo "downloading $url"
curl -fsSL "$url" -o "$tmp/rp.tar.gz"
tar -xzf "$tmp/rp.tar.gz" -C "$tmp"
mkdir -p "$dir"
install -m 755 "$tmp/rp" "$dir/rp"

echo "installed rp to $dir/rp"
case ":$PATH:" in
  *":$dir:"*) echo "try it now:  rp --demo" ;;
  *) echo "try it now:  $dir/rp --demo"
     echo "(add $dir to your PATH to type just 'rp')" ;;
esac
