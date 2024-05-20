bin/ctuna: src/main.cc src/cap.cc src/tun.cc
	g++ -Wall -Werror -Wextra $^ -o $@ -Iinclude -lcap
	sudo setcap cap_net_admin=ep $@

.PHONY: clean

clean:
	rm bin/ctuna
