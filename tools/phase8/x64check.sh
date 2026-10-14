# x64check.sh [file.c...]: the 64-bit pointer warnings of the port's game code with the scratch decomp's edits
# applied (what the port will build after the merge). The port's src/legoland is copied to an overlay, the
# scratch decomp's diff against main is applied to it, and each file is compiled as 64-bit (syntax only, with the
# port's x64 flags). Writes every warning to /tmp/x64check.txt. With no arguments: every file.
D=$HOME/scratch/dv64
P=$HOME/wt/port
X=$HOME/xwin
B=$P/build-clang-x64
O=/tmp/x64overlay
rm -rf $O
mkdir -p $O/src
cp -r $P/src/legoland $O/src/
for d in port miniwin extensions; do ln -s $P/$d $O/$d; done
: > /tmp/x64check.conflicts
# each file the scratch decomp changed: merged into the port's copy as git merge will (base: main)
# base: the last decomp commit the port merged (the port has none of the decomp's later commits yet)
BASE=$(git -C $P merge-base port main)
for f in $(git -C $D diff --name-only $BASE -- src/legoland); do
  git -C $D show $BASE:$f > /tmp/x64check.base
  cp $O/$f /tmp/x64check.ours
  if ! git merge-file -q /tmp/x64check.ours /tmp/x64check.base $D/$f; then
    # both sides changed the same lines: the check takes the decomp's; the real merge resolves it by hand
    echo "conflict (to resolve in the merge): $f" >> /tmp/x64check.conflicts
    git merge-file -q --theirs $O/$f /tmp/x64check.base $D/$f
  else
    cp /tmp/x64check.ours $O/$f
  fi
done
FLAGS="/nologo -DDIRECTINPUT_VERSION=0x0500 -DLEGOLAND_PORT=1 -D_CRT_SECURE_NO_WARNINGS -I$P/port -I$P/extensions
 -I$P/miniwin/include -I$B/_deps/sdl3-build/include-revision -I$B/_deps/sdl3-src/include
 --target=x86_64-pc-windows-msvc -fms-compatibility /imsvc $X/crt/include /imsvc $X/sdk/include/ucrt
 /imsvc $X/sdk/include/um /imsvc $X/sdk/include/shared /imsvc $X/sdk/include/winrt /DWIN32 /D_WINDOWS
 -fno-delete-null-pointer-checks -Wno-error=int-conversion -Wno-error=incompatible-function-pointer-types
 -Wno-error=return-mismatch -ferror-limit=0 /FI$P/miniwin/include/miniwin/redirect.h -fsyntax-only"
KINDS='int-to-pointer-cast|pointer-to-int-cast|int-conversion|void-pointer-to-int-cast|int-to-void-pointer-cast|incompatible-function-pointer-types|pointer-integer-compare'
cd $O/src/legoland || exit 1
if [ $# -eq 0 ]; then
  set -- $(ls *.c | grep -v '^imports.c$\|^bootstrap.c$\|^crt.c$')
fi
: > /tmp/x64check.txt
: > /tmp/x64check.errors
for f in "$@"; do
  f=$(basename $f)
  (clang-cl $FLAGS $f > /tmp/x64check.$f.log 2>&1
   grep -E "warning: .*\[-W($KINDS)\]" /tmp/x64check.$f.log | sort -u >> /tmp/x64check.txt
   grep -E ": error:" /tmp/x64check.$f.log >> /tmp/x64check.errors
   rm -f /tmp/x64check.$f.log) &
  while [ $(jobs -r | wc -l) -ge 8 ]; do sleep 0.1; done
done
wait
cat /tmp/x64check.conflicts
echo "$(wc -l < /tmp/x64check.txt) warnings, $(wc -l < /tmp/x64check.errors) errors"
grep -oE '^[^(]+' /tmp/x64check.txt | sort | uniq -c | sort -rn | head -${TOP:-15}
