A MultiRPC implementation by modification of XML-RPC library.

Executing the program:

To run the code, following steps are to be followed:

Setting up the project:
1. An installed version of XMLRPC-C with libcurl-dev package installed on the system without libwww-dev and libwww-ssl0 packages.
2. Decompress the provided files of the assignment code.
3. Place the folder in the installation base directory for XMLRPC-C.
4. Replace ./src/xmlrpc_client_global.c file in the installation with the corresponding file is the provided code.
 5. Replace ./include/xmlrpc-c/client_global.h file in the installation with the corresponding file is the provided code.
6. cd ./<project folder>

Compiling and running the project:
1. make clean
2. make
3. cd bin/
4. ./service <number of servers to start> start
5. ./client <number of requests> <number of servers for considering responses>
6. ./service stop

