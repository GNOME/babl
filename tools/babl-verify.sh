#!/bin/sh

# this is a tool for debugging available babl fast paths relatd to
# a given pixel format - the script is intended to be run as is -
# as well as modified as needed including more relevant conversions

rm ~/.cache/babl/babl-fishes

format=$1
if [ "x$format" = "x" ];then
  echo "pass a babl format to verify (in quotes) - running with \"R'G'B'A u8\""
  format="R'G'B'A u8"
fi

base_path=`realpath $0`
base_path=`dirname $base_path`/..
base_path=`realpath $base_path`
echo $base_path

make -C $base_path/extensions || exit

export BABL_PATH=$base_path/extensions/.libs

echo ""
echo "[$format]"
$base_path/tools/babl-verify "$format" "cairo-ARGB32" "x"
$base_path/tools/babl-verify "cairo-ARGB32" "$format" "x"
$base_path/tools/babl-verify "$format" "RaGaBaA float" "x"
$base_path/tools/babl-verify "RaGaBaA float" "$format" "x"
$base_path/tools/babl-verify "$format" "RGBA float" "x"
$base_path/tools/babl-verify "RGBA float" "$format" "x"
$base_path/tools/babl-verify "$format" "R'G'B'A float" "x"
$base_path/tools/babl-verify "R'G'B'A float" "$format" "x"
$base_path/tools/babl-verify "$format" "R~G~B~A float" "x"
$base_path/tools/babl-verify "R~G~B~A float" "$format" "x"

$base_path/tools/babl-verify "$format" "cairo-ARGB32"
$base_path/tools/babl-verify "cairo-ARGB32" "$format"
$base_path/tools/babl-verify "$format" "RaGaBaA float"
$base_path/tools/babl-verify "RaGaBaA float" "$format"
$base_path/tools/babl-verify "$format" "RGBA float"
$base_path/tools/babl-verify "RGBA float" "$format"
$base_path/tools/babl-verify "$format" "R'G'B'A float"
$base_path/tools/babl-verify "R'G'B'A float" "$format"
$base_path/tools/babl-verify "$format" "R~G~B~A float"
$base_path/tools/babl-verify "R~G~B~A float" "$format"

