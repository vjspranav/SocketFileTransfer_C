# SocketFileTransfer_C
A simple C code to transfer files through a client server connection.
## How to run
As this program is spread over Server and client, we'll need two terminals.  
One in Server Folder and one in Client Folder.  
> Make sure the files you want to download exist in Server folder.  
**Server**
In terminal open in Server Folder
```
gcc Server.c -o Server
./Server
```
This should Open server in listen mode and should output something like  
```
[vjspranav@archlinux Server]$ ./Server 
Listening for Clients
```
Perfect, now time to start a Client.  
  
**Client**
In terminal open in Client Folder
```
gcc Client.c -o Client
./Client
```
This should connect to server and give following out put in
* Server
```
[vjspranav@archlinux Server]$ ./Server 
Listening for Clients
Connection Successfull
```
* Client
```
[vjspranav@archlinux Client]$ ./Client 
** Connection Successfull **
Welcome to inter device file transfer mechanism
1. ls to view available files
2. get <filename> to download file from the dir of server
3. exit to exit
```

and that's it.
