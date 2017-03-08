#/bin/bash
TOP_DIR=$PWD
export TOP_DIR

install_lame() {
	echo "install libmp3lame."
	cd ext_src
	tar xf lame-3.99.5.tar.bz2
	cd lame-3.99.5
	./configure --prefix $TOP_DIR/ext_lib
	[[ $? -eq 0 ]] && echo "configured: "$? 
	make -j3 install
	echo "install: $?"
	if [ $? -eq 0 ] ; then
	touch $TOP_DIR/ext_src/.installed
	fi
}

if [[ ! -d $TOP_DIR/ext_lib || ! -e $TOP_DIR/ext_src/.installed ]] ; then
	install_lame
	cd $TOP_DIR
fi

if [ $# -gt 0 ] ; then
	case $1 in 
		clean)
			echo "Clean objects"
			make -C build clean
			;;
		w2m)
			echo "build w2m"
			make -C build w2m
			;;
		*)
			echo "./build.sh [clean|w2m]"
			;;
	esac
else
	make -C build w2m
fi


