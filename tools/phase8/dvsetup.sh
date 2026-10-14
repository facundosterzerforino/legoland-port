# dvsetup.sh: a scratch worktree of the decomp's main for the 64-bit type work, and its baseline (build + verify)
export PATH="$HOME/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
export UV_PYTHON=3.12
D=$HOME/scratch/dv64
cd ~/legoland || exit 1
if [ ! -d $D ]; then
  git worktree add -q --detach $D main || exit 1
  ln -s ~/legoland/toolchain $D/toolchain
  cp reccmp-user.yml $D/
  sed "s|^project: .*|project: $D|" reccmp-build.yml > $D/reccmp-build.yml
  [ -d .venv ] && ln -s ~/legoland/.venv $D/.venv
fi
cd $D || exit 1
git log --oneline -1
date
cmake --preset msvc6 > /tmp/dv_cfg.log 2>&1; echo "configure exit $?"
cmake --build build > /tmp/dv_build.log 2>&1; echo "build exit $?"; tail -2 /tmp/dv_build.log
date
./tools/verify --silent --json /tmp/dv_baseline.json > /tmp/dv_verify.log 2>&1; echo "verify exit $?"
tail -3 /tmp/dv_verify.log
date
git checkout -q -- uv.lock 2>/dev/null
ls -la /tmp/dv_baseline.json
