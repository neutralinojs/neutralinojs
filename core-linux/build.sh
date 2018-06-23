echo "Neutralino is being built.."
if [ -e bin/neutranino.out ]; then 
    rm bin/neutralino.out
fi

g++ -std=c++11 src/Buffer.cpp src/EventLoop.cpp src/EventLoopThreadPool.cpp src/Handler.cpp src/Parser.cpp src/Socket.cpp src/Epoll.cpp src/EventLoopThread.cpp src/functions.cpp src/main.cpp src/router.cpp src/Thread.cpp src/core/filesystem.cpp src/settings.cpp src/core/os.cpp src/core/computer.cpp -pthread -o bin/neutralino 