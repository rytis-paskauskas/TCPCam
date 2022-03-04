OBJECTS:=TCPCam_client.o lib.o

%.o : %.c
	$(CC) -c $<

TCPCam_client: $(OBJECTS)
	$(CC) -o $@ $?

.PHONY = clean

clean:
	@rm -f *~ *.o
