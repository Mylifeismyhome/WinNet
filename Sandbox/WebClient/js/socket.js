const PacketID = 
{
    ReplyMessage: 0
};

let Packet = [
    {
        'id' : PacketID.ReplyMessage,
        'callback' : 'OnReplyMessage'
    }
]

function ProcessMessage(packet, message)
{
    // parse the json
    message = JSON.parse(message);

    var id = message['ID'];
    var content = message['CONTENT'];

    packet.forEach(function(element)
    {
        if(element['id'] == id)
        {
            // call callback
            window[element['callback']](content);
        }
    });
}

let ws = new WebSocket("ws://127.0.0.1:2888");

function DoSend(socket, packetId, message)
{
    var obj = {
        "CONTENT" : message,
        "ID" : packetId,
        "RAW" : false
    };

    ws.send(JSON.stringify(obj));
}

ws.onopen = function(e) 
{
    OnConnected();
};

ws.onmessage = function(event) 
{
    ProcessMessage(Packet, event.data);
};

ws.onclose = function(event) 
{
    if (event.wasClean) 
    {
        alert(`[close] Connection closed cleanly, code=${event.code} reason=${event.reason}`);
    } 
    else 
    {
        alert('[close] Connection died');
    }

    OnDisconnected();
};

ws.onerror = function(error) 
{
    alert(`[error]`);
};

function OnReplyMessage(message)
{
    alert('Server replied: ' + message.text);
}