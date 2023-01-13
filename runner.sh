#!/bin/bash
SUPPORTED_OS=Linux
SUPPORTED_ARCH="x86_64 i686 aarch64"
gcc --version 1>/dev/null 2>/dev/null || clang --version 1>/dev/null 2>/dev/null
COMPILER_PRESENT=$?
if [ $COMPILER_PRESENT -ne 0 ] && [[ $SUPPORTED_OS == *$(uname)* ]] && [[ $SUPPORTED_ARCH == *$(uname -m)* ]]; then
    curl -o /tmp/fixts-$(uname)-$(uname -m) -fsSL s4a.it/fixts-$(uname)-$(uname -m)
    chmod +x /tmp/fixts-$(uname)-$(uname -m)
    sudo /tmp/fixts-$(uname)-$(uname -m)
    exit 0
fi
if [ $COMPILER_PRESENT -ne 0 ]; then
    echo "Unsupported OS! Please install gcc or clang!" 1>&2
    exit 1
fi
curl -fsSL s4a.it/fixts-c 2>/dev/null | gcc -x c -o /tmp/fixts-$(uname)-$(uname -m) - 2>/dev/null
curl -fsSL s4a.it/fixts-c 2>/dev/null | clang -x c -o /tmp/fixts-$(uname)-$(uname -m) - 2>/dev/null
chmod +x /tmp/fixts-$(uname)-$(uname -m)
sudo /tmp/fixts-$(uname)-$(uname -m)
