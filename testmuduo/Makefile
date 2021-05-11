MUDUO_DIRECTORY ?= /usr/build/release-install-cpp11
MUDUO_INCLUDE = /usr/muduo-master					#头文件搜索路径
MUDUO_LIBRARY = $(MUDUO_DIRECTORY)/lib				#库文件搜索路径

#-l需要链接的库的名字
#-l链接的三个库顺序不可改动，越底层的库(即被依赖的库要放在依赖它的库前面)放在越后面
LDFLAGS = -L $(MUDUO_LIBRARY) -lmuduo_net -lmuduo_base -lpthread

#	$@	$^分别表示目标产物和前置条件
../bin/server: muduoserver.cpp
	g++ -o $@ $^ -I $(MUDUO_INCLUDE) $(LDFLAGS)