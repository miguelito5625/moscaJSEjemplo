var mqtt = require('mqtt');
var client = mqtt.connect('mqtt://broker.emqx.io');

client.on('connect', function () {
    // client.subscribe('#'); //Para escuchar todos los topics
    client.subscribe('mike/5625/#');
});

client.on('message', function (topic, message) {
    context = message.toString();
    // console.log(context)
    console.log(`topic = ${topic}; message: ${context}`);
});