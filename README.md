# Trellis: Privilege Separation for Multi-User Applications Made Easy

## Installation Guide

### Download
#### Trellis patches and modules
```
git clone https://github.com/m4mbr3/trellis.git

export TRELLIS_HOME=$PWD/trellis

cd $TRELLIS_HOME
```
#### LLVM and Clang

```
git clone http://llvm.org/git/llvm.git --branch release_35 --depth 1 llvm

cd llvm/tools

git clone http://llvm.org/git/clang.git --branch release_35 --depth 1 clang
```
#### Linux kernel

```
cd $TRELLIS_HOME

git clone git@github.com/torvalds/linux.git --branch v3.9 --depth 1 kernel

tar -xvf linux-3.9.tar.gz
```

#### Glibc

```
git clone git://sourceware.org/git/glibc.git --branch glibc-2.19 glibc

```


### Patch and Build

At this point all the required software to build trellis is ready for patching and building

#### Patch Linux kernel
```
cd $TRELLIS_HOME/kernel

patch -p1 < $TRELLIS_HOME/patches/trellis_kernel-3.9.patch

```

#### Build Linux Kernel
```
make menuconfig  	<--Configure the kernel as you wish

make -j $NUM_CORES

ln -s /usr/src/linux $TRELLIS_HOME/kernel
```
Install the kernel image in your system and reboot in it.

#### Patch llvm
```
cd $TRELLIS_HOME/llvm

patch -p1 < $TRELLIS_HOME/patches/trellis_llvm-3.5.patch
```
#### Patch clang
```
cd $TRELLIS_HOME/llvm/tools/clang

patch -p1 < $TRELLIS_HOME/patches/trellis_clang-3.5.patch
```

#### Build llvm/clang
```
cd $TRELLIS_HOME

mkdir llvm_build

mkdir llvm_root

export LLVM_SRC=$TRELLIS_HOME/llvm

export LLVM_BUILD=$TRELLIS_HOME/llvm_build

export LLVM_ROOT=$TRELLIS_HOME/llvm_root

cd $LLVM_BUILD

$LLVM_SRC/configure --prefix=$LLVM_ROOT --enable-shared

make -j $NUM_CORES

make install

```
#### Patch glibc
```
cd $TRELLIS_HOME/glibc/

patch -p1 < $TRELLIS_HOME/patches/trellis_glibc-2.19.patch
```
#### Build glibc
```
cd $TRELLIS_HOME

mkdir glibc_build

mkdir glibc_root

export GLIBC_SRC=$TRELLIS_HOME/glibc

export GLIBC_BUILD=$TRELLIS_HOME/glibc_build

export GLIBC_ROOT=$PTRELLIS_HOME/glibc_root

cd $GLIBC_BUILD

$GLIBC_SRC/configure --prefix=$GLIBC_ROOT

make -j $NUM_CORE

make install
```

#### Build Other tools
```
cd $TRELLIS_HOME/allocator
make
cd $TRELLIS_HOME/ps_char_device
```
# To be continued
