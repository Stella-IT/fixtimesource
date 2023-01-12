#!/bin/bash
SUPPORTED_OS=Linux
SUPPORTED_ARCH="x86_64 i386 aarch64"
gcc --version 1>/dev/null 2>/dev/null || clang --version 1>/dev/null 2>/dev/null
COMPILER_PRESENT=$?
if [ $COMPILER_PRESENT -ne 0 ] && [[ $SUPPORTED_OS == *$(uname)* ]] && [[ $SUPPORTED_ARCH == *$(uname -p)* ]]; then
    curl -o /tmp/fixts-$(uname)-$(uname -p) -fsSL s4a.it/fixts-$(uname)-$(uname -p)
    chmod +x /tmp/fixts-$(uname)-$(uname -p)
    sudo /tmp/fixts-$(uname)-$(uname -p)
    exit 0
fi
if [ $COMPILER_PRESENT -ne 0 ]; then
    echo "Unsupported OS! Please install gcc or clang!" 1>&2
    exit 1
fi
curl -fsSL s4a.it/fixts-c | gcc -x c -o /tmp/fixts-$(uname)-$(uname -p) 2>/dev/null
curl -fsSL s4a.it/fixts-c | clang -x c -o /tmp/fixts-$(uname)-$(uname -p) 2>/dev/null
chmod +x /tmp/fixts-$(uname)-$(uname -p)
sudo /tmp/fixts-$(uname)-$(uname -p)