# CN socket programming project phase 1 README
## Compilation Instruction
### Compile
server: g++ server.cpp -o server
client: g++ client.cpp -o client
## Usage Guide
### Execute
- `server: ./server <port>`
- `client: ./client 127.0.0.1 <port>`
### Command Types And Forms
- `register: register <id> <password>`
- `login: login <id> <password> <port>`
- `logout`
- `quit`
If you type command in wrong form, client will ignore it and say it's not correct.
## Any Additional Information
I implement some error-detection in my code, including but not only:
- `Repeated registration/ login`
- `Wrong password`
- `Non-existent user`
- `Try to logout without login`
- `Use same port with same ip`  
I forgot to show `repeated login` in my vedio, TAs can try my code to verify it.
