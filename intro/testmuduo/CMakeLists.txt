#   cmake .. 自动生成MakeFile   ..表示CMakeLists.txt文件所在的目录，一般在build目录下使用此命令，这样生成的乱七八糟的中间文件都在build目录下
#   然后make即可生成目标可执行文件

#   配置头文件搜索路径
include_directories(/usr/muduo-master)
#   配置库文件搜索路径
link_directories(/usr/build/release-install-cpp11/lib)

#设置需要编译的源文件列表
set(SRC_LIST muduoserver.cpp)

#   把.指定路径下的所有源文件名字放入变量名SRC_LIST里面
#   aux_source_directory(. SRC_LIST)

#   设定编译生成的可执行文件的存储位置  (不能直接在可执行文件名中写路径，会出错)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)   #项目根目录为CHATSERVER

#   表示生成可执行文件server   源文件为SRC_LIST变量定义的列表
add_executable(server ${SRC_LIST})
#   表示server这个目标程序，需要链接的库
target_link_libraries(server muduo_net muduo_base pthread)