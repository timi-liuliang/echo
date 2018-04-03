#====================================
# define
#====================================
COMPILE_OPTIONS =  -I.//../  -O0 -g3 -Wall -c -fmessage-length=0 

OBJECT_FILE_LIST = \
./make/assert.o \
./make/backtrace.o \
./make/buffer.o \
./make/directory.o \
./make/file.o \
./make/file_reader.o \
./make/frame_lock.o \
./make/ini_file.o \
./make/log.o \
./make/log_file.o \
./make/math.o \
./make/matrix.o \
./make/pstring.o \
./make/pwstring.o \
./make/quaternion.o \
./make/rect.o \
./make/ring_buffer.o \
./make/stringdef.o \
./make/task_thread.o \
./make/task_thread_pool.o \
./make/thread.o \
./make/thread_log.o \
./make/time.o \
./make/path.o \
./make/vector.o 

#====================================
# target file
#====================================
./make/libscl.a : prepare $(OBJECT_FILE_LIST) 
	ar -rc ./make/libscl.a $(OBJECT_FILE_LIST);\
#rm -f ./../lib/libscl.a;\
#cp ./make/libscl.a ./../lib/libscl.a 


#====================================
# object files
#====================================
./make/assert.o : assert.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/assert.o" "assert.cpp" 

./make/backtrace.o : backtrace.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/backtrace.o" "backtrace.cpp" 

./make/buffer.o : buffer.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/buffer.o" "buffer.cpp" 

./make/directory.o : directory.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/directory.o" "directory.cpp" 

./make/file.o : file.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/file.o" "file.cpp" 

./make/file_reader.o : file_reader.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/file_reader.o" "file_reader.cpp" 

./make/frame_lock.o : frame_lock.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/frame_lock.o" "frame_lock.cpp" 

./make/ini_file.o : ini_file.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/ini_file.o" "ini_file.cpp" 

./make/log.o : log.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/log.o" "log.cpp" 

./make/log_file.o : log_file.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/log_file.o" "log_file.cpp" 

./make/math.o : math.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/math.o" "math.cpp" 

./make/matrix.o : matrix.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/matrix.o" "matrix.cpp" 

./make/pstring.o : pstring.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/pstring.o" "pstring.cpp" 

./make/pwstring.o : pwstring.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/pwstring.o" "pwstring.cpp" 

./make/quaternion.o : quaternion.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/quaternion.o" "quaternion.cpp" 

./make/rect.o : rect.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/rect.o" "rect.cpp" 

./make/ring_buffer.o : ring_buffer.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/ring_buffer.o" "ring_buffer.cpp" 

./make/stringdef.o : stringdef.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/stringdef.o" "stringdef.cpp" 

./make/task_thread.o : task_thread.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/task_thread.o" "task_thread.cpp" 

./make/task_thread_pool.o : task_thread_pool.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/task_thread_pool.o" "task_thread_pool.cpp" 

./make/thread.o : thread.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/thread.o" "thread.cpp" 

./make/thread_log.o : thread_log.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/thread_log.o" "thread_log.cpp" 

./make/time.o : time.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/time.o" "time.cpp" 

./make/path.o : path.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/path.o" "path.cpp" 

./make/vector.o : vector.cpp
	g++ $(COMPILE_OPTIONS) -o"./make/vector.o" "vector.cpp" 


#====================================
# prepare
#====================================
prepare : 
	rm -r -f ./make;mkdir ./make

#====================================
# clean
#====================================
clean : 
	rm -r -f ./make; \
	rm -f ./../lib/libscl.a;
