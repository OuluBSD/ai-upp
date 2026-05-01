#ifndef _HtmlGui_SimWebSocket_h_
#define _HtmlGui_SimWebSocket_h_

#include <Core/Core.h>

NAMESPACE_UPP
#include <Core/Inet.h>
END_UPP_NAMESPACE

NAMESPACE_UPP

class SimWebSocket : public IWebSocket, public Pte<SimWebSocket> {
	bool open = true;

public:
	SimWebSocket();
	virtual ~SimWebSocket();

	virtual void   SendText(const String& data) override;
	virtual void   SendBinary(const String& data) override;
	virtual String Receive() override;
	virtual bool   IsOpen() const override { return open; }
	virtual void   Close(const String& msg = Null) override;

	// Bridge link
	Event<const String&> WhenSend;
};

END_UPP_NAMESPACE

#endif
