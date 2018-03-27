# SUPER crude makefiles for my standalone tests

CPPEXE = c++ -I../../../dep/include -L../../../dep/lib -lrtaudio -std=c++11 -stdlib=libc++  -Werror
RUN_PREFIX = DYLD_LIBRARY_PATH=../../../dep/lib

docsaw:	docsaw.cpp
	$(CPPEXE) -o docsaw docsaw.cpp

run_docsaw:	docsaw
	$(RUN_PREFIX) ./docsaw

clean:
	rm docsaw



