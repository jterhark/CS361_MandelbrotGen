mandelbrot: mandelbrot.cpp mandelCalc mandelDisplay structs.h
	g++ -std=c++11 -o mandelbrot mandelbrot.cpp structs.h

mandelCalc: mandelbrot-calc.cpp structs.h
	g++ -std=c++11 -o mandelCalc mandelbrot-calc.cpp structs.h

mandelDisplay: mandelbrot-display.cpp structs.h
	g++ -std=c++11 -o mandelDisplay mandelbrot-display.cpp structs.h
