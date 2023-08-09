DOXY_DIR=cubix.doxygen
DOXY_FILE=cubix-dox-file.cfg

if [[ -v CX_SOURCE ]]; then
echo $CX_SOURCE
else
CX_SOURCE=$PWD/..
echo $CX_SOURCE
fi

mkdir -p ./$DOXY_DIR
cp $CX_SOURCE/documentation/$DOXY_FILE ./

cp $CX_SOURCE/documentation/css/* ./
cp $CX_SOURCE/documentation/js/* ./
cp $CX_SOURCE/documentation/layout/* ./
cp -r $CX_SOURCE/documentation/mdfiles $DOXY_DIR/
cp -r $CX_SOURCE/documentation/images $DOXY_DIR
cp $CX_SOURCE/icons/* $DOXY_DIR/images/

mkdir -p $DOXY_DIR/examples
cp $CX_SOURCE/examples/*.{cpp,C} $DOXY_DIR/examples

cp -r $CX_SOURCE/src $DOXY_DIR
find cubix.doxygen/src \( -name '*.hpp' -o -name '*.json' -o -name '*LinkDef.h' -o -name '*.md'  -o -name '*.txt' -o -name '*.cmake' -o -name 'sqlite3' -o -name 'tkn-lib' -o -name 'src' \) -exec rm -rf {} +

MAJOR=`grep -m1 Cubix_VERSION_MAJOR ${CX_SOURCE}/CMakeLists.txt | cut -d\" -f2`
MINOR=`grep -m1 Cubix_VERSION_MINOR ${CX_SOURCE}/CMakeLists.txt | cut -d\" -f2`

PROJECT_NUMBER="${MAJOR}.${MINOR}"
echo "Cubix version: ${PROJECT_NUMBER}"

echo "PROJECT_NUMBER = ${PROJECT_NUMBER}" >> $DOXY_FILE

doxygen $DOXY_FILE

