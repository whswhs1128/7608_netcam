
CFLAGS= -I/
all:
	arm-goke-linux-uclibcgnueabi-gcc -o test  hemu_openapi.cpp RTPHandle.cpp app_hemu.cpp hemu_protobuf.cpp test_hemu_openapi.c -I../lib/inc/ -I../inc -lpthread -lstdc++ \
	../lib/lib/libprotobuf-lite.a ../lib/lib/libcurl.a ../lib/lib/libssl.a ../lib/lib/libcrypto.a  ../lib/lib/libjsoncpp.a
	 cp test /opt/goke/rootfs_uClibc/
clean:
	rm test 
