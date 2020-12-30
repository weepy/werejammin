const dgram = require('dgram')
const { Buffer } = require('buffer')
const socket = dgram.createSocket('udp4')

socket.on('message', (msg, remote) => {
    


    const arr = new Uint32Array(msg.buffer)
    console.log(arr.length, arr[0], arr[1])

    // console.log('Data received from client : ' + msg.toJSON())
    console.log('Received %d bytes from %s:%d\n',msg.length, remote.address, remote.port)


    socket.send(new Buffer(msg), remote.port, remote.address, function(err, bytes) {
        if (err) throw err;
        console.log(`UDP message sent to ${remote.address}:${remote.port}`);
    });


})
socket.bind(8080)