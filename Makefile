#CXX=icpc -openmp -O3 
#cc=icc -openmp -O3
CXX=g++ -O3 -g
cc=gcc -O3 -g
#CXX=g++ -O3 -g  

spg_incs = spgstuff/arithmetic.h \
           spgstuff/delaunay.h \
           spgstuff/kpoint.h \
           spgstuff/pointgroup.h \
           spgstuff/sitesym_database.h \
           spgstuff/spg_database.h \
           spgstuff/symmetry.h \
           spgstuff/cell.h \
           spgstuff/hall_symbol.h \
           spgstuff/mathfunc.h \
           spgstuff/primitive.h \
           spgstuff/site_symmetry.h \
           spgstuff/spglib.h \
           spgstuff/tetrahedron_method.h \
           spgstuff/debug.h \
           spgstuff/kgrid.h \
           spgstuff/niggli.h \
           spgstuff/refinement.h \
           spgstuff/spacegroup.h \
           spgstuff/spin.h \
           spgstuff/version.h
spg_o = spgstuff/arithmetic.o \
        spgstuff/cell.o \
        spgstuff/debug.o \
        spgstuff/delaunay.o \
        spgstuff/hall_symbol.o \
        spgstuff/kgrid.o \
        spgstuff/kpoint.o \
        spgstuff/mathfunc.o \
        spgstuff/niggli.o \
        spgstuff/pointgroup.o \
        spgstuff/primitive.o \
        spgstuff/refinement.o \
        spgstuff/site_symmetry.o \
        spgstuff/sitesym_database.o \
        spgstuff/spacegroup.o \
        spgstuff/spg_database.o \
        spgstuff/spglib.o \
        spgstuff/spin.o \
        spgstuff/symmetry.o \
        spgstuff/spglib_f.o \
        spgstuff/tetrahedron_method.o

all:main
main: main.o pwscfData.o types.o util.o spgstuff/libmyspg.a handleTilemats.o
	$(CXX) -o main main.o pwscfData.o types.o util.o handleTilemats.o spgstuff/libmyspg.a
pwscfData.o: pwscfData.cpp pwscfData.h
	$(CXX) -c pwscfData.cpp
handleTilemats.o: handleTilemats.cpp handleTilemats.h util.h types.h pwscfData.h
	$(CXX) -c handleTilemats.cpp
main.o: main.cpp pwscfData.h handleTilemats.h handlePwscf.h
	$(CXX) -c main.cpp
types.o: types.cpp util.h util.h spgstuff/spglib.h
	$(CXX) -Ispgstuff/ -c types.cpp
util.o: util.cpp util.h types.h
	$(CXX) -c util.cpp
spgstuff/libmyspg.a: $(spg_o) $(spg_incs)
	ar rcs spgstuff/libmyspg.a $(spg_o)
clean:
	rm -f main *.o spgstuff/*.o spgstuff/libmyspg.a 


spgstuff/arithmetic.o: spgstuff/arithmetic.c $(spg_incs)
	$(cc) -o spgstuff/arithmetic.o -c spgstuff/arithmetic.c
spgstuff/hall_symbol.o: spgstuff/hall_symbol.c $(spg_incs)
	$(cc) -o spgstuff/hall_symbol.o -c spgstuff/hall_symbol.c
spgstuff/niggli.o: spgstuff/niggli.c $(spg_incs)
	$(cc) -o spgstuff/niggli.o -c spgstuff/niggli.c
spgstuff/sitesym_database.o: spgstuff/sitesym_database.c $(spg_incs)
	$(cc) -o spgstuff/sitesym_database.o -c spgstuff/sitesym_database.c
spgstuff/spglib.o: spgstuff/spglib.c $(spg_incs)
	$(cc) -o spgstuff/spglib.o -c spgstuff/spglib.c
spgstuff/test.o: spgstuff/test.c $(spg_incs)
	$(cc) -o spgstuff/test.o -c spgstuff/test.c
spgstuff/cell.o: spgstuff/cell.c $(spg_incs)
	$(cc) -o spgstuff/cell.o -c spgstuff/cell.c
spgstuff/kgrid.o: spgstuff/kgrid.c $(spg_incs)
	$(cc) -o spgstuff/kgrid.o -c spgstuff/kgrid.c
spgstuff/pointgroup.o: spgstuff/pointgroup.c $(spg_incs)
	$(cc) -o spgstuff/pointgroup.o -c spgstuff/pointgroup.c
spgstuff/site_symmetry.o: spgstuff/site_symmetry.c $(spg_incs)
	$(cc) -o spgstuff/site_symmetry.o -c spgstuff/site_symmetry.c
spgstuff/spglib_f.o: spgstuff/spglib_f.c $(spg_incs)
	$(cc) -o spgstuff/spglib_f.o -c spgstuff/spglib_f.c
spgstuff/tetrahedron_method.o: spgstuff/tetrahedron_method.c $(spg_incs)
	$(cc) -o spgstuff/tetrahedron_method.o -c spgstuff/tetrahedron_method.c
spgstuff/debug.o: spgstuff/debug.c $(spg_incs)
	$(cc) -o spgstuff/debug.o -c spgstuff/debug.c
spgstuff/kpoint.o: spgstuff/kpoint.c $(spg_incs)
	$(cc) -o spgstuff/kpoint.o -c spgstuff/kpoint.c
spgstuff/primitive.o: spgstuff/primitive.c $(spg_incs)
	$(cc) -o spgstuff/primitive.o -c spgstuff/primitive.c
spgstuff/spacegroup.o: spgstuff/spacegroup.c $(spg_incs)
	$(cc) -o spgstuff/spacegroup.o -c spgstuff/spacegroup.c
spgstuff/spin.o: spgstuff/spin.c $(spg_incs)
	$(cc) -o spgstuff/spin.o -c spgstuff/spin.c
spgstuff/delaunay.o: spgstuff/delaunay.c $(spg_incs)
	$(cc) -o spgstuff/delaunay.o -c spgstuff/delaunay.c
spgstuff/mathfunc.o: spgstuff/mathfunc.c $(spg_incs)
	$(cc) -o spgstuff/mathfunc.o -c spgstuff/mathfunc.c
spgstuff/refinement.o: spgstuff/refinement.c $(spg_incs)
	$(cc) -o spgstuff/refinement.o -c spgstuff/refinement.c
spgstuff/spg_database.o: spgstuff/spg_database.c $(spg_incs)
	$(cc) -o spgstuff/spg_database.o -c spgstuff/spg_database.c
spgstuff/symmetry.o: spgstuff/symmetry.c $(spg_incs)
	$(cc) -o spgstuff/symmetry.o -c spgstuff/symmetry.c

