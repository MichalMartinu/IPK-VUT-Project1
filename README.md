# IPK project Client/Server application
Implementation of Client/Server program to gain information of users from /etc/passw.
School project for *IPK - Computer Communications and Networks*

## Run program
Server
```
./ipk-server -p port
```
Client
```
./ipk-client -h host -p port [-n|-f|-l] login
```
### Client arguments
```
host  -- Identification of server
port  -- Number of destination port.
-n    -- Full user name will be returned, including any additional information of input login
-f    -- User home directory will be returned of input login
-l    -- List of all users on separated line will be returned
login -- Specifies user login name for above operations 
```


### Compiling

Program is UNIX based an you can compile it by command:

```
make             -- compile ipk-server.c and ipk-client.c
make ipk-server  -- compile ipk-server.c
make ipk-client  -- compile ipk-client.c
```

