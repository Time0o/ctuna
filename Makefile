bin/ctuna: src/ctuna.cc
	g++ -Wall -Werror $< -o $@ -lcap

.PHONY: install

install: bin/ctuna
	cp $< /usr/local/bin
