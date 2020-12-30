const portAudio = require('naudiodon')

// const [mic] = portAudio.getDevices()

// Create an instance of AudioIO with inOptions (defaults are as below), which will return a ReadableStream
var ai = new portAudio.AudioIO({
  inOptions: {
    channelCount: 2,
    sampleFormat: portAudio.SampleFormat16Bit,
    sampleRate: 44100,
    deviceId: -1, // Use -1 or omit the deviceId to select the default device
    closeOnError: true // Close the stream if an audio error is detected, if set false then just log the error
  }
})

ai.on('data', buf => {
    console.log(buf.timestamp)
})

ai.start()