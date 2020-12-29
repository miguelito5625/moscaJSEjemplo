var mqtt = require('mqtt');

var client = mqtt.connect('mqtt://broker.emqx.io');

client.on('connect', function () {
    setInterval(function () {
        let numero = (Math.random()>=0.5)? 1 : 0;
        client.publish('Mike/5625/led', numero.toString());
        console.log('Message Sent:', numero);
    }, 2000);
    // setInterval(function () {
    //     client.publish('mike/5625/luzsala', 'Hola MQTT');
    //     console.log('Message Sent');
    // }, 2000);
    // setInterval(function () {
    //     client.publish('testTopic', 'Hello mqtt testTopic');
    //     console.log('Message Sent');
    // }, 4000);
});