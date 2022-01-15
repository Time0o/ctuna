bin/ctuna: src/ctuna.cc src/tun.cc
	g++ -Wall -Werror $^ -o $@ -Iinclude -lcap

.PHONY: install

install: bin/ctuna
	cp $< /usr/local/bin
