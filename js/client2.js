// CLIENT

const dgram = require('dgram')
const socket = dgram.createSocket('udp4')
const HOST = 'localhost'
const PORT = 3333

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
    }
    else {
        max = delta * (0.05) + max * 0.95
    }
})

setInterval(() => {
    console.clear()
    console.log(Math.floor(smoothed*10)/10 +"\n"+ Math.floor(max*10)/10)
}, 100)