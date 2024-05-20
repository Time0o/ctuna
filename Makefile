bin/ctuna: src/main.cc src/cap.cc src/tun.cc
	g++ -Wall -Werror $^ -o $@ -Iinclude -lcap

.PHONY: install

install: bin/ctuna
	cp $< /usr/local/bin
