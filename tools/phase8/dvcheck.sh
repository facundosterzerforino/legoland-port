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
# MSVC6's warnings for a prototype that differs from its definition or a pointer of another type, in the changed
# files (C4028/C4029: a parameter differs from the declaration)
for f in $(git diff --name-only HEAD -- src/legoland | grep '\.c$'); do
  grep -E "$(basename $f)\([0-9]+\) : warning C(4028|4029)" /tmp/dv_build.log | head -5
done
./tools/verify --silent --json /tmp/dv_new.json > /tmp/dv_verify.log 2>&1
git checkout -q -- uv.lock 2>/dev/null
grep -E "Accuracy|Progress" /tmp/dv_verify.log
python3 $S/vcompare.py /tmp/dv_baseline.json /tmp/dv_new.json | tail -${VLINES:-12}
ok=$?
TOP=${TOP:-8} bash $S/x64check.sh "$@"
exit $ok
