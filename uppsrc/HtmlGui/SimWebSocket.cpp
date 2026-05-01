#include "SimWebSocket.h"

NAMESPACE_UPP

SimWebSocket::SimWebSocket()
{
}

SimWebSocket::~SimWebSocket()
{
}

void SimWebSocket::SendText(const String& data)
{
	WhenSend(data);
}

void SimWebSocket::SendBinary(const String& data)
{
	// For now, same as text
	WhenSend(data);
}

String SimWebSocket::Receive()
{
	return ""; // SimWebSocket usually works via WhenMessage
}

void SimWebSocket::Close(const String& msg)
{
	open = false;
}

END_UPP_NAMESPACE
