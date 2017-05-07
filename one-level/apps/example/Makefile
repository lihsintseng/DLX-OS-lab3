default:
	cd makeprocs; make
	cd 1; make
	cd 2; make
	cd 3; make
	cd 4; make
	cd 5; make
	cd 6; make

clean:
	cd makeprocs; make clean
	cd 1; make clean
	cd 2; make clean
	cd 3; make clean
	cd 4; make clean
	cd 5; make clean
	cd 6; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 30; ee469_fixterminal
