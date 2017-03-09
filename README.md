# w2m

This program encodes a set of WAV files to MP3.

Usage:
w2m \<directory name\>


Development environments<br>
 For Windows: MSYS2 + MinGW-w64 on windows7.<br>
  gcc.exe (Rev2, Built by MSYS2 project) 6.2.0

  You can get the guide of installation of MSYS2 from MSYS2 homepage.(<a href="http://www.msys2.org/">here</a>)
 
 For Linux: Ubuntu 14.04.5 LTS<br>
  gcc (Ubuntu 4.9.4-2ubuntu1~14.04.1) 4.9.4


How to build?<br>
	Download source from Github.
	And change directory to w2m.
	Run build.sh
<pre>
	git clone https://github.com/neinnil/w2m.git
	cd w2m
	./build.sh
</pre>
	Then, you get w2m binary in the build directory. (w2m/build)


Limitations<br>
	Now, w2m can only PCM and IEEE_FLOAT formats of WAV.
	It is a program for a Little endian system.


To Do <br>
The other formats will be supported in future.


