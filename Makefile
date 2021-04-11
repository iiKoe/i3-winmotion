CC = g++

project = i3-winmotion

src = main.cpp printtree.cpp
obj = $(src:%.cpp=%.o)

CPPFLAGS := -O3 -std=c++17 -I../i3-ipcpp/include
LDFLAGS = -L../i3-ipcpp/build/lib/static/ -lcairo -lX11 -l:libi3-ipc++.a

$(project): $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

all: $(project)

.PHONY: clean

clean:
	rm -f $(obj) $(project)

