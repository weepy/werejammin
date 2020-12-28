const dgram = require('dgram')
const socket = dgram.createSocket('udp4')
const HOST = 'localhost'
// const HOST = 'b33p.live'
const PORT = 12345

setInterval(() => {

    const t = new Uint32Array(128)
    const time = process.hrtime()
    t[0] = time[0]
    t[1] = time[1]
    

    socket.send(Buffer.from(t.buffer), PORT, HOST)
    
}, 1)

function hrtimeToMs([s, ns]) {
    return s*1000 + ns/1000000
}

let smoothed = 0
let max = 0

socket.on('message', (msg, remote) => {

    const arr = new Uint32Array(msg.buffer)
    const now = process.hrtime()


    const delta = hrtimeToMs(now) - hrtimeToMs(arr)

    smoothed = delta * 0.1 + smoothed * 0.9

    if(delta > max) {
        max = delta
        // console.log("MAX", Math.floor(max*100)|0)
    }
    else {
        max = delta * (0.05) + max * 0.95
        // console.log(Math.floor(smoothed*100)|0)
    }

    

    // console.log('Data received from client : ' + msg.toString())
    // console.log('Received %d bytes from %s:%d\n',msg.length, remote.address, remote.port)


    // socket.send(new Buffer(message), remote.port, remote.address, function(err, bytes) {
    //     if (err) throw err;
    //     console.log(`UDP message sent to ${remote.address}:${remote.port}`);
    // });


})

setInterval(() => {
    console.clear()
    console.log(Math.floor(smoothed*10)/10 +"\n"+ Math.floor(max*10)/10)
}, 100)
