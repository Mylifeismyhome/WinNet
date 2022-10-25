"use strict";
var Socket;

function Connect(connect)
{
    Socket = new WebSocket(connect);
	Socket.binaryType = "blob";
    Socket.onopen = OnOpen;
    Socket.onerror = OnClose;
    Socket.onmessage = OnMessage;
    Socket.onclose = OnClose;
}

function Disconnect() 
{
    Socket.close();
}

function OnOpen(e)
{
    OnOpenCallback();
}

function OnError(e)
{
	console.error("WebSocket error observed:", event);
    Socket.close();
    OnErrorCallback();
}

function OnMessage(e)
{
    Receive(e);
}

function OnClose(e)
{
    OnCloseCallback();
}

function Send(id, content) 
{
    var out = 
    {
        ID: parseInt(id),
        CONTENT: content
    };

    var Json = JSON.stringify(out);
    Socket.send(Json);
}

function Receive(event) 
{
	if(typeof event.data == 'object')
	{
		event.data.arrayBuffer().then(buffer => {
			const view = new DataView(buffer);
			const id = view.getUint32(0);
			window["On" + GetEnumName(id)](new Uint8Array(buffer, 4));
		});
	}
	else
	{
		var begin = event.data.indexOf("{");
		var end = event.data.lastIndexOf("}") + 1;
		var json = JSON.parse(event.data.substr(begin, end));
		window["On" + GetEnumName(json.ID)](json.CONTENT);
	}
}

function LOG(event) 
{
    console.log(event);
}

function GetEnumName(value) 
{
	for (var i in PKG)
	{
		if (PKG[i] == value) 
		{
			return i;
		}
	}
}