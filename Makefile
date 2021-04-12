CC = g++

project = i3-winmotion
obj_dir = ./build
bin_dir = ./bin

src = main.cpp printtree.cpp
obj = $(src:%.cpp=$(obj_dir)/%.o)

CXXFLAGS := -O3 -Wall -std=c++17 -Ilib/i3-ipcpp/include
LDFLAGS = -Llib/i3-ipcpp/build/lib/static/ -lcairo -lX11 -l:libi3-ipc++.a

i3ipcpp=lib/i3-ipcpp/build/lib/static/libi3-ipc++.a

all: $(bin_dir)/$(project)

$(i3ipcpp):
	git submodule update --init --recursive && cd lib/i3-ipcpp && ./configure && $(MAKE)

$(obj_dir)/%.o : %.cpp $(i3ipcpp)
	@mkdir -p $(obj_dir)
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(bin_dir)/$(project): $(obj)
	@mkdir -p $(bin_dir)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf $(obj_dir) $(project)

