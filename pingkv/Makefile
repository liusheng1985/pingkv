SRC := src
TOOLDIR := ./src/tool
DBFDIR := ./src/dbf
IDXDIR := ./src/index
CHACHEDIR := ./src/cache
SERVERDIR := ./src/server
BIN := bin

OO     := ./bin/data_file.o ./bin/data_file_block.o ./bin/data_file_row.o ./bin/hash.o ./bin/container_default_funcs.o ./bin/kv_logger.o ./bin/tool.o ./bin/kv_index.o ./bin/kv_lru.o ./bin/llist.o ./bin/kv_base.o ./bin/kv_work_queue.o ./bin/kv_server.o ./bin/kv_poller.o ./bin/kv_worker.o ./bin/kv_io_buf.o ./bin/kv_work.o ./bin/kv_conf.o ./bin/kv_sender.o

COO    := ./bin/pingkv.o ./bin/kv_client.o ./bin/kv_route.o ./bin/kv_work.o ./bin/tool.o ./bin/hash.o ./bin/kv_conf.o ./bin/kv_logger.o

all: ./bin/main.o $(OO) ./bin/pingkv.o ./bin/kv_client.o ./bin/kv_route.o
	g++ -g -o ./bin/pingkv_server ./bin/main.o $(OO) -lrt -std=c++0x -lboost_regex -lboost_thread-mt
	g++ -g -o ./bin/pingkv $(COO) -std=c++0x -lboost_regex -lrt 

./bin/kv_work.o: ./src/server/kv_work.cc ./src/server/kv_work.h ./src/conf/kv_conf.h
	g++ -g -c -o ./bin/kv_work.o ./src/server/kv_work.cc

./bin/kv_sender.o: ./src/server/kv_sender.cc ./src/server/kv_sender.h ./src/server/kv_work_queue.h
	g++ -g -c -o ./bin/kv_sender.o ./src/server/kv_sender.cc

./bin/kv_conf.o: ./src/conf/kv_conf.cc ./src/conf/kv_conf.h 
	g++ -g -c -o ./bin/kv_conf.o ./src/conf/kv_conf.cc

./bin/kv_worker.o: ./src/server/kv_worker.cc ./src/server/kv_worker.h ./src/tool/kv_base.h ./src/index/kv_index.h ./src/server/kv_work_queue.h
	g++ -g -c -o ./bin/kv_worker.o ./src/server/kv_worker.cc

./bin/kv_poller.o: ./src/tool/kv_poller.cc ./src/tool/kv_poller.h
	g++ -g -c -o ./bin/kv_poller.o ./src/tool/kv_poller.cc

./bin/kv_io_buf.o: ./src/server/kv_io_buf.cc ./src/server/kv_io_buf.h
	g++ -g -c -o ./bin/kv_io_buf.o ./src/server/kv_io_buf.cc

./bin/kv_server.o: ./src/server/kv_server.cc ./src/server/kv_server.h ./src/conf/kv_conf.h ./src/tool/kv_base.h ./src/tool/kv_poller.h ./src/dbf/data_file_tool.h ./src/server/kv_sender.h
	g++ -g -c -o ./bin/kv_server.o ./src/server/kv_server.cc

./bin/llist.o: ./src/tool/llist.cc ./src/tool/llist.h
	g++ -g -c -o ./bin/llist.o ./src/tool/llist.cc

./bin/hash.o: ./src/tool/hash.cc ./src/tool/hash.h
	g++ -g -c -o ./bin/hash.o ./src/tool/hash.cc

./bin/kv_base.o: ./src/tool/kv_base.cc ./src/tool/kv_base.h ./src/err/kv_logger.h
	g++ -g -c -o ./bin/kv_base.o ./src/tool/kv_base.cc 

./bin/tool.o: ./src/tool/tool.cc ./src/tool/tool.h 
	g++ -g -c -o ./bin/tool.o ./src/tool/tool.cc 

./bin/container_default_funcs.o: ./src/tool/container_default_funcs.cc ./src/tool/container_default_funcs.h
	g++ -g -c -o ./bin/container_default_funcs.o ./src/tool/container_default_funcs.cc

./bin/kv_lru.o: ./src/cache/kv_lru.cc ./src/cache/kv_lru.h ./src/tool/llist.h ./src/tool/hashtable.h ./src/tool/hash.h ./src/tool/kv_base.h ./src/conf/kv_conf.h 
	g++ -g -c -o ./bin/kv_lru.o ./src/cache/kv_lru.cc 

./bin/kv_index.o: ./src/index/kv_index.h ./src/index/kv_index.cc ./src/dbf/data_file.h ./src/cache/kv_lru.h ./src/tool/kv_base.h ./src/conf/kv_conf.h 
	g++ -g -c -o ./bin/kv_index.o ./src/index/kv_index.cc

./bin/data_file.o: ./src/dbf/data_file.h ./src/dbf/data_file_conf.h ./src/dbf/data_file_tool.h ./src/dbf/data_file.cc ./src/tool/hashtable.h ./src/tool/kv_base.h ./src/err/kv_logger.h ./src/conf/kv_conf.h ./src/dbf/data_file_tool.h ./src/dbf/data_file_conf.h 
	g++ -g -c -o ./bin/data_file.o ./src/dbf/data_file.cc

./bin/data_file_block.o: ./src/dbf/data_file_block.cc ./src/tool/hashtable.h ./src/tool/kv_base.h ./src/err/kv_logger.h ./src/conf/kv_conf.h ./src/dbf/data_file_tool.h ./src/dbf/data_file_conf.h
	g++ -g -c -o ./bin/data_file_block.o ./src/dbf/data_file_block.cc

./bin/data_file_row.o: ./src/dbf/data_file_row.cc ./src/tool/hashtable.h ./src/tool/kv_base.h ./src/err/kv_logger.h ./src/conf/kv_conf.h ./src/dbf/data_file_tool.h ./src/dbf/data_file_conf.h
	g++ -g -c -o ./bin/data_file_row.o ./src/dbf/data_file_row.cc

./bin/kv_logger.o: ./src/err/kv_logger.cc ./src/err/kv_logger.h 
	g++ -g -c -o ./bin/kv_logger.o ./src/err/kv_logger.cc 

./bin/kv_work_queue.o: ./src/server/kv_work_queue.cc ./src/server/kv_work_queue.h ./src/tool/kv_base.h
	g++ -g -c -o ./bin/kv_work_queue.o ./src/server/kv_work_queue.cc 

./bin/kv_client.o: ./src/client/kv_client.cc ./src/client/kv_client.h ./src/conf/kv_conf.h ./src/client/kv_route.h
	g++ -g -c -o ./bin/kv_client.o ./src/client/kv_client.cc 

./bin/kv_route.o: ./src/client/kv_route.cc ./src/client/kv_route.h
	g++ -g -c -o ./bin/kv_route.o ./src/client/kv_route.cc 

./bin/main.o: ./src/main.cc ./src/main.h $(OO)
	g++ -g -c -o ./bin/main.o ./src/main.cc 

./bin/pingkv.o: ./src/client/pingkv.cc 
	g++ -g -c -o ./bin/pingkv.o ./src/client/pingkv.cc 

clean:
	mkdir -p ./bin
	rm -f ./bin/*
