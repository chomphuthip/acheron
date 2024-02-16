# acheron
A fileless malware server.

Acheron is a fileless malware server, designed to supply payloads to [Charon](https://github.com/chomphuthip/charon) instances. In ancient Greek mythology, the Acheron river is the entrances to the underworld with Charon as it's sole ferryman. 

Acheron retrieves a file handle using ```fopen_s``` uses it to determine the size of the file, allocates a buffer big enough for the file, then loads it. Once the file is loaded, a TCP socket is bound to a port (default: 7777). When a connection to a Charon instance is established, the size of the payload in the form of a 64 bit unsigned long (size_t) is sent. Once the size is sent back from the client, the payload will be sent and presumably executed.

Improvements that could be made include:
1. Multithreaded responses, with a listening socket passing responses to different threads.
2. Payload encryption using a DH exchange with a static keypair for Acheron instances and a keypair created at runtime for Charon instances.
3. A magic header to identify friendly Charon instances

Shellcode is handwritten and assembled with [NASM](https://www.nasm.us/) with the ```-f bin``` arguments. Shellcode has to be statically compiled and/or be able to run without calling external DLLs directly as module offsets will differ between instances. The third option is to walk the pointers from the ```TEB``` in the ```FS``` register to the location of the desired module and the function contained within. 

Usage: ```acheron shell.bin```
