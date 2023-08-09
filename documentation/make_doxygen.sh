DOXY_DIR=tkn.doxygen
DOXY_FILE=tkn-dox-file.cfg

if [[ -v TKN_SOURCE ]]; then
echo $TKN_SOURCE
else
TKN_SOURCE=$PWD/..
echo $TKN_SOURCE
fi

mkdir -p ./$DOXY_DIR
cp $TKN_SOURCE/documentation/$DOXY_FILE ./

cp $TKN_SOURCE/documentation/css/* ./
cp $TKN_SOURCE/documentation/js/* ./
cp $TKN_SOURCE/documentation/layout/* ./
cp -r $TKN_SOURCE/documentation/mdfiles $DOXY_DIR/
cp -r $TKN_SOURCE/documentation/images $DOXY_DIR
cp $TKN_SOURCE/icons/* $DOXY_DIR/images/

mkdir -p $DOXY_DIR/examples
cp $TKN_SOURCE/examples/*.{cpp,C} $DOXY_DIR/examples

cp -r $TKN_SOURCE/src $DOXY_DIR
find tkn.doxygen/src \( -name '*.hpp' -o -name '*.json' -o -name '*LinkDef.h' -o -name '*.md'  -o -name '*.txt' -o -name '*.cmake' -o -name 'sqlite3' \) -exec rm -rf {} +

MAJOR=`grep -m1 TkN_VERSION_MAJOR ${TKN_SOURCE}/CMakeLists.txt | cut -d\" -f2`
MINOR=`grep -m1 TkN_VERSION_MINOR ${TKN_SOURCE}/CMakeLists.txt | cut -d\" -f2`

PROJECT_NUMBER="${MAJOR}.${MINOR}"
echo "TkN version: ${PROJECT_NUMBER}"

echo "PROJECT_NUMBER = ${PROJECT_NUMBER}" >> $DOXY_FILE

doxygen $DOXY_FILE

