#!/bin/sh
set -eu

repo="nesetkab/rp"
dir="${RP_INSTALL_DIR:-$HOME/.local/bin}"

case "$(uname -s)" in
  Darwin) asset="rp-macos-universal" ;;
  Linux)
    case "$(uname -m)" in
      x86_64 | amd64) asset="rp-linux-x86_64" ;;
      aarch64 | arm64) asset="rp-linux-arm64" ;;
      *) echo "no build for $(uname -m) srry, build it urself: https://github.com/$repo" >&2; exit 1 ;;
    esac
    ;;
  *) echo "no build for $(uname -s), use wsl if ur on windows" >&2; exit 1 ;;
esac

url="https://github.com/$repo/releases/latest/download/$asset.tar.gz"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

echo "grabbing $asset..."
curl -fsSL "$url" -o "$tmp/rp.tar.gz"
tar -xzf "$tmp/rp.tar.gz" -C "$tmp"
mkdir -p "$dir"
install -m 755 "$tmp/rp" "$dir/rp"

echo "installed to $dir/rp"
case ":$PATH:" in
  *":$dir:"*) echo "now run: rp --demo" ;;
  *) echo "now run: $dir/rp --demo" ;;
esac
