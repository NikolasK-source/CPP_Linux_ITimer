all: static_lib
static_lib: libITimer.a

libITimer.a: ITimer.o
	ar rcs $@ $^

ITimer.o: ITimer.cpp
	g++ -std=c++11 -O2 -c $< -o $@

clean:
	rm -f *.o *.a
