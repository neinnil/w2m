#/bin/bash
TOP_DIR=$PWD


function install_lame {
echo "install libmp3lame."
cd ext_src
tar xf lame-3.99.5.tar.bz2
cd lame-3.99.5
./configure --prefix $TOP_DIR/ext_lib
[[ $? -eq 0 ]] && echo "configured: "$? 
make -j3 install
echo "install: $?"
if [[ $? -eq 0 ]] ; then
touch $TOP_DIR/ext_src/.installed
fi
}


if [[ ! -d $TOP_DIR/ext_lib || ! -e $TOP_DIR/ext_src/.installed ]] ; then
install_lame
fi

cd $TOP_DIR
cd build
make w2m

