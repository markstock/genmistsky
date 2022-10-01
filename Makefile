CC=g++
all : genmistsky

% : %.cpp y_atmosphere.h
	$(CC) -o $@ $<

test : genmistsky
	./genmistsky -n=15 -z=1 +z=90000 -unif > .test.rad
	cat .test.rad | sed 's/x y z/1 0 -0.05/' > test.rad
	rm .test.rad
	oconv -n 64 -r 15000000 test.rad > test.oct
	rvu -vp 0 0 400000 -vd 0 1 -0.4 -vh 60 -vv 40 -pe -7 -ab 2 -aa 0 -ms 400 -n 8 test.oct
	#ra_ppm img.pic | pamscale -filter=triangle -xsize 1000 | pnmtopng > img.png

clean :
	rm -f genmistsky test.rad test.oct

