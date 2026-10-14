# vf.sh addr...: rebuild the scratch decomp (incremental) and show reccmp for these functions
export PATH="$HOME/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
export UV_PYTHON=3.12
cd ~/scratch/dv64 || exit 1
cmake --build build > /tmp/vf_build.log 2>&1 || { grep -E ": error " /tmp/vf_build.log | head; exit 1; }
for a in "$@"; do ./tools/verify -v $a > /tmp/vf_$a.log 2>&1; grep -m1 -E "[0-9.]+%" /tmp/vf_$a.log; done
git checkout -q -- uv.lock 2>/dev/null
