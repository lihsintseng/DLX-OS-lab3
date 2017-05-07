default:
	cd makeprocs; make
	cd fork_test1; make
	cd fork_test2; make

clean:
	cd makeprocs; make clean
	cd fork_test1; make clean
	cd fork_test2; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 30; ee469_fixterminal
