all: compile

compile: SLR_parser.cpp
	g++ SLR_parser.cpp -o SLR_parser

run: SLR_parser
	./SLR_parser input.txt output.txt
clean: SLR_parser
	rm SLR_parser
