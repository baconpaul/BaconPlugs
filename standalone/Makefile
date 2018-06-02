# SUPER crude makefiles for my standalone tests.
# Basically use it as
# make run T=docsaw
# and that will build docsaw.cpp which depends on every header everywhere and voila.

HDRS = $(wildcard ../src/*.hpp) $(wildcard include/*.hpp)

CPPEXE = c++ -I include -I bld -I ../src -I Catch2/include -I../../../dep/include -L../../../dep/lib -lrtaudio -std=c++11 -stdlib=libc++  -Werror
RUN_PREFIX = DYLD_LIBRARY_PATH=../../../dep/lib

bld:
	mkdir bld
	curl -o bld/catch.hpp https://raw.githubusercontent.com/CatchOrg/Catch2/master/single_include/catch.hpp

.PRECIOUS:	bld/exe_demo_% bld/exe_test_% bld

bld/exe_demo_%:	demo/%.cpp $(HDRS) bld
	$(CPPEXE) -o $@ $<

bld/exe_test_%:	test/%.cpp $(HDRS) bld
	$(CPPEXE) -o $@ $<

demo_%:	bld/exe_demo_%
	$(RUN_PREFIX) ./$<

test_%:	bld/exe_test_%
	$(RUN_PREFIX) ./$<

TESTS = $(foreach src, $(wildcard test/*.cpp), $(subst /,_,$(subst .cpp,, $(src))) )

allTests: $(TESTS)

run_%:	exe_%
	$(RUN_PREFIX) ./$<

clean:
	rm -rf bld



