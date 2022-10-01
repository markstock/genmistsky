CC=g++
all : genmistsky

% : %.cpp y_atmosphere.h
	$(CC) -o $@ $<

test : genmistsky
	./genmistsky -n=10 -z=-500 +z=20000 > .test.rad
	cat .test.rad | sed 's/x y z/1 0 -0.02/' > test.rad
	rm .test.rad
	oconv -n 64 -r 15000000 test.rad > test.oct
	rvu -vp 0 0 100000 -vd 0 1 -0.2 -vh 60 -vv 40 -pe -7 -ab 2 -aa 0 -n 8 test.oct
	#ra_ppm img.pic | pamscale -filter=triangle -xsize 1000 | pnmtopng > img.png

clean :
	rm -f genmistsky test.rad test.oct

