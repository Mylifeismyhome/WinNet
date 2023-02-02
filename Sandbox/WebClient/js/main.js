$('#input_button').click(function()
{
    var text = $('#input_text').val();
  
    DoSend(ws, PacketID.ReplyMessage, {'text' : text });
});

function OnConnected()
{
    $('#input_button').removeAttr("disabled");
}

function OnDisconnected()
{
    $('#input_button').attr("disabled", true);
}