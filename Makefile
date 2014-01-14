all: 
	(cd src; make)
	(cd doc; make)
install: 
	(cd src; make install)
	(cd doc; make install)

uninstall:
	(cd src; make uninstall)
	(cd doc; make uninstall)

tidy:
	(cd src; make tidy)
clean:
	(cd src; make clean)
