#!/bin/bash
TOP_DIR=$PWD
export TOP_DIR

install_lame() {
if [[ ! -d $TOP_DIR/ext_lib || ! -e $TOP_DIR/ext_src/.installed ]] ; then
	echo "install libmp3lame."
	cd $TOP_DIR/ext_src
	tar xf lame-3.99.5.tar.bz2
	if [[ -d $TOP_DIR/ext_src/lame-3.99.5 ]] ; then 
		cd lame-3.99.5 || exit 0
		./configure --prefix $TOP_DIR/ext_lib && make -j3 install && touch $TOP_DIR/ext_src/.installed
	fi
	cd $TOP_DIR
fi
}

remove_lame() {
	echo "To remove libmp3lame."
	rm -rf $TOP_DIR/ext_lib
	cd ext_src
	rm -f .installed
	[[ -d $TOP_DIR/ext_src/lame-3.99.5 ]] && rm -rf $TOP_DIR/ext_src/lame-3.99.5
#	make -C $TOP_DIR/ext_src/lame-3.99.5 distclean
#	fi
	cd $TOP_DIR
}

if [ $# -gt 0 ] ; then
while [ $# -gt 0 ]
do
	case $1 in 
		rmlame)
			echo "remove libmp3lame"
			remove_lame
			;;
		clean)
			echo "Clean objects"
			make -C build clean
			;;
		w2m)
			install_lame
			echo "build w2m"
			make -C build w2m
			;;
		*)
			echo "./build.sh [clean|w2m|rmlame]"
			exit
			;;
	esac
	shift
done
else
	install_lame
	make -C build w2m
fi


