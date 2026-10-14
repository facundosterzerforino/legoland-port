export PATH="$HOME/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
cd /tmp && uv run -q --no-project --with "libclang==18.1.1" python $(cd "$(dirname "$0")" && pwd)/sizefix.py "$@"
