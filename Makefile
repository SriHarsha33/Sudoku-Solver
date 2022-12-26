build:
	gcc sudoku.c -o sudoku -lm -lpthread
clean:
	rm sudoku
run:
	make build
	./sudoku 9 ex_input_9x9
run_16:
	make build
	./sudoku 16 test16_hard
run_25:
	make build
	./sudoku 25 test25_2