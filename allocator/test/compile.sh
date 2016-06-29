#!/bin/bash
FILECPP=$1".c"
FILEBC=$1".bc"
FILEBC1=$1".bc1"
FILEBC2=$1".bc2"
FILEBC3=$1".bc3"
FILEBC4=$1".bc4"
FILEOBJ=$1".o"
FILEEXE="test_lib_shared"
echo "$LLVM_ROOT/bin/clang -emit-llvm -c $FILECPP -o $FILEBC `pkg-config --cflags gtk+-2.0` `mysql_config --cflags` -I/root/privsep_malloc/include"
$LLVM_ROOT/bin/clang -g -emit-llvm -c $FILECPP -o $FILEBC `pkg-config --cflags gtk+-2.0` `mysql_config --cflags` -I/root/privsep_malloc/include 
$LLVM_ROOT/bin/llvm-link  $FILEBC -o $FILEBC1 
#$LLVM_ROOT/bin/opt $FILEBC1 -o $FILEBC2 -PrivilegeSeparationOnModule
$LLVM_ROOT/bin/opt $FILEBC1 -o $FILEBC2 -TaggingPropagation 
$LLVM_ROOT/bin/opt $FILEBC2 -o $FILEBC3 -SysCallInsertion 
$LLVM_ROOT/bin/opt $FILEBC3 -o $FILEBC4 -PrivilegeSeparationOnModule 
$LLVM_ROOT/bin/llc -filetype=obj $FILEBC4 -o $FILEOBJ 
echo "$LLVM_ROOT/bin/clang -g $FILEOBJ -o $FILEEXE `pkg-config --cflags --libs gtk+-2.0` `mysql_config --libs` -L/root/privsep_malloc/lib -lprivsep_malloc"
$LLVM_ROOT/bin/clang -g $FILEOBJ -o $FILEEXE `pkg-config --cflags --libs gtk+-2.0` `mysql_config --libs` -L/root/privsep_malloc/lib -lprivsep_malloc
$LLVM_ROOT/bin/clang -g -Xlinker "-Tps_link_script.ld"  $FILEOBJ -o $FILEEXE  `mysql_config --libs` `pkg-config --libs gtk+-2.0` -L/root/privsep_malloc/lib -lprivsep_malloc
#$LLVM_ROOT/bin/clang++ -v -Xlinker "-Tps_link_script.ld"  -Xlinker "--verbose"   $FILEOBJ -o $FILEEXE
#$LLVM_ROOT/bin/clang++ $FILEOBJ -o $FILEEXE
