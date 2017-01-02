# w2m

This program encodes a set of WAV files to MP3.

Usage:
w2m <directory name>


Development environments
 For Windows: Msys2 + MIngw64 on windows7.
 Installation guide of Msys2
  Msys2: https://sourceforge.net/p/msys2/wiki/MSYS2%20installation/
  gcc.exe (Rev2, Built by MSYS2 project) 6.2.0
 
 For Linux: Ubuntu 14.04.5 LTS
  gcc (Ubuntu 4.9.4-2ubuntu1~14.04.1) 4.9.4


How to build?
	Download source from Github.
	And change directory to w2m.
	Run build.sh

	git clone https://github.com/neinnil/w2m.git
	cd w2m
	./build.sh

	Then, you get w2m binaray in the build direcotry. (w2m/build)


[Limitations of this program.]
	Now, w2m can only PCM and IEEE_FLOAT formats of WAV.
	It is a program for a Little endian system.


To Do
	The other formats will be supported in future.


