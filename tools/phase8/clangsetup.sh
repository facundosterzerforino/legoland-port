# clangsetup.sh: what the libclang tools (views_scan.py, retarget.py) need to parse the decomp as MSVC6 sees it:
# lowercase links to MSVC6's (uppercase) headers in ~/scratch/msvc6inc, and a clang VFS overlay mapping the
# decomp's relative "../../toolchain/msvc6/Include" includes there (~/scratch/msvc6vfs.yaml).
I=~/scratch/msvc6inc
T=~/legoland/toolchain/msvc6/Include
mkdir -p $I
for f in $T/*; do
  b=$(basename "$f"); l=$(echo "$b" | tr A-Z a-z)
  if [ -d "$f" ]; then
    mkdir -p "$I/$l"
    for g in "$f"/*; do gb=$(basename "$g"); ln -sf "$g" "$I/$l/$gb"; gl=$(echo "$gb" | tr A-Z a-z); [ "$gl" != "$gb" ] && ln -sf "$g" "$I/$l/$gl"; done
  else
    ln -sf "$f" "$I/$b"; [ "$l" != "$b" ] && ln -sf "$f" "$I/$l"
  fi
done
cat > ~/scratch/msvc6vfs.yaml <<Y
{ 'version': 0, 'roots': [ { 'type': 'directory-remap', 'name': '$HOME/scratch/dv64/toolchain/msvc6/Include', 'external-contents': '$I' } ] }
Y
echo "$(ls $I | wc -l) entries in $I"
