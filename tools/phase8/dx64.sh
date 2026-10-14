# dx64.sh file.c...: the scratch decomp's own files compiled as 64-bit (syntax only, xwin SDK) -> /tmp/dx64.txt
export PATH="$HOME/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
X=$HOME/xwin
D=$HOME/scratch/dv64/src/legoland
FLAGS="/nologo -DDIRECTINPUT_VERSION=0x0500 -D_CRT_SECURE_NO_WARNINGS --target=x86_64-pc-windows-msvc -fms-compatibility
 /imsvc $X/crt/include /imsvc $X/sdk/include/ucrt /imsvc $X/sdk/include/um /imsvc $X/sdk/include/shared /DWIN32 /D_WINDOWS
 -Wno-error=int-conversion -Wno-error=incompatible-function-pointer-types -Wno-error=return-mismatch -ferror-limit=0 -fsyntax-only -vfsoverlay /tmp/dx64vfs.yaml"
KINDS='int-to-pointer-cast|pointer-to-int-cast|int-conversion|void-pointer-to-int-cast|int-to-void-pointer-cast|incompatible-function-pointer-types|pointer-integer-compare'
printf "{\"version\":0,\"roots\":[{\"name\":\"$HOME/scratch/dv64/toolchain/msvc6/Include\",\"type\":\"directory\",\"contents\":[{\"name\":\"math.h\",\"type\":\"file\",\"external-contents\":\"$X/sdk/include/ucrt/math.h\"}]}]}" > /tmp/dx64vfs.yaml
cd $D || exit 1
[ $# -eq 0 ] && set -- $(ls *.c | grep -v '^imports.c$\|^bootstrap.c$\|^crt.c$')
: > /tmp/dx64.txt; : > /tmp/dx64.err
for f in "$@"; do
  (clang-cl $FLAGS $f > /tmp/dx64.$f.log 2>&1
   grep -E "warning: .*\[-W($KINDS)\]" /tmp/dx64.$f.log | sort -u >> /tmp/dx64.txt
   grep -E ": error:" /tmp/dx64.$f.log >> /tmp/dx64.err; ) &
  while [ $(jobs -r | wc -l) -ge 8 ]; do sleep 0.1; done
done
wait
echo "$(wc -l < /tmp/dx64.txt) warnings, $(wc -l < /tmp/dx64.err) errors"
