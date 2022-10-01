CC=g++
all : genmistsky

% : %.cpp y_atmosphere.h
	$(CC) -o $@ $<

test : genmistsky
	./genmistsky > test.rad
	oconv -n 64 -r 30000000 test.rad > test.oct
	rvu -vp 0 0 100000 -vd 0 1 -0.2 -vh 60 -vv 40 -pe -7 -ab 2 -aa 0 -n 8 test.oct
	#ra_ppm img.pic | pamscale -filter=triangle -xsize 1000 | pnmtopng > img.png

clean :
	rm -f genmistsky test.rad test.oct

