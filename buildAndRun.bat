g++ -g src/*.cpp src/*.c -static -Iinclude -iquote include -Llib -lopengl32 -lglfw3 -lgdi32 -o splines -mincoming-stack-boundary=2
splines.exe