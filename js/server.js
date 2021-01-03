const udp = require('dgram')

// --------------------creating a udp server --------------------

// creating a udp server
const server = udp.createSocket('udp4')

// emits when any error occurs
server.on('error', (error) => {
  console.log('Error: ' + error)
  server.close()
})

// emits on new datagram msg
server.on('message', (msg,info) => {
  // console.log('Data received from client : ' + msg.toString())
  console.log('Received %d bytes from %s:%d\n',msg.length, info.address, info.port)

  //sending msg
  server.send(msg,info.port,info.address,function(error){
    if(error){
      client.close()
    }
    else {
      console.log('Data sent !!!')
    }
  })

})

//emits when socket is ready and listening for datagram msgs
server.on('listening', () => {
  const address = server.address()
  const { port, family, ipaddr } = address
  console.log(`Server is listening at port ${port}
Server ip : ${ipaddr}
Server is IP4/IP6 : ${family}`)

})

//emits after the socket is closed using socket.close()
server.on('close', () => {
  console.log('Socket is closed !')
})

server.bind(3333,'localhost')



