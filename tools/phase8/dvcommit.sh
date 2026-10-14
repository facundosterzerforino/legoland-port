# dvcommit.sh <message file>: formats the scratch decomp's changed files, checks the build and the match once more,
# commits there, and moves the decomp's main to that commit (the shared ~/legoland: its working tree and index
# updated for those files only, nothing else touched).
export PATH="$HOME/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
export UV_PYTHON=3.12
S=$(cd "$(dirname "$0")" && pwd)
D=$HOME/scratch/dv64
MSG=$1
cd $D || exit 1
files=$(git diff --name-only HEAD)
[ -n "$files" ] || { echo "nothing to commit"; exit 1; }
for f in $files; do
  case $f in *.c|*.h) clang-format -i $f ;; esac
done
uv run tools/needsdecl.py > /tmp/needsdecl.log 2>&1 || { tail -20 /tmp/needsdecl.log; git checkout -q -- uv.lock; exit 1; }
git checkout -q -- uv.lock 2>/dev/null
bash $S/dvcheck.sh > /tmp/dvc.out 2>&1
ok=$?
head -6 /tmp/dvc.out
[ $ok -eq 0 ] || { echo "not committed: the match got worse or the build failed"; exit 1; }
old=$(git rev-parse HEAD)
git add $files
git commit -q -F $MSG || exit 1
new=$(git rev-parse HEAD)
cd ~/legoland || exit 1
if [ "$(git rev-parse main)" != "$old" ]; then
  echo "main moved meanwhile (now $(git rev-parse --short main)): not updated; rebase the scratch commit"
  exit 1
fi
git update-ref refs/heads/main $new $old || exit 1
if [ "$(git symbolic-ref -q HEAD)" = "refs/heads/main" ]; then
  git reset -q -- $files
  git -C $D diff $old $new | git apply || echo "WARNING: ~/legoland's working tree didn't take the diff"
fi
git log --oneline -1 main
git status --short | head
# the new baseline
cp /tmp/dv_new.json /tmp/dv_baseline.json
