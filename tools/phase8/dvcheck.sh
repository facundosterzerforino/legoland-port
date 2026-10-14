# dvcheck.sh: after editing the scratch decomp (~/scratch/dv64): MSVC6 build (clean when a header changed), verify
# against the baseline (any function that got worse fails), then the 64-bit warnings with the edits applied.
export PATH="$HOME/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
export UV_PYTHON=3.12
S=$(cd "$(dirname "$0")" && pwd)
D=$HOME/scratch/dv64
cd $D || exit 1
if git diff --name-only HEAD | grep -q '\.h$'; then CLEAN=--clean-first; else CLEAN=; fi
if ! cmake --build build $CLEAN > /tmp/dv_build.log 2>&1; then
  echo "MSVC6 BUILD FAILED"
  grep -iE "error" /tmp/dv_build.log | head -20
  exit 1
fi
./tools/verify --silent --json /tmp/dv_new.json > /tmp/dv_verify.log 2>&1
git checkout -q -- uv.lock 2>/dev/null
grep -E "Accuracy|Progress" /tmp/dv_verify.log
python3 $S/vcompare.py /tmp/dv_baseline.json /tmp/dv_new.json | tail -${VLINES:-12}
ok=$?
TOP=${TOP:-8} bash $S/x64check.sh "$@"
exit $ok
