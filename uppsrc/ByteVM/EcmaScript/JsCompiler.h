#ifndef _ByteVM_EcmaScript_JsCompiler_h_
#define _ByteVM_EcmaScript_JsCompiler_h_

#include "JsValue.h"
#include <ByteVM/Python/PyCompiler.h>

NAMESPACE_UPP

class JsCompiler : public PyCompiler {
protected:
	virtual void ParseBlock() override;
	virtual void Statement() override;
	virtual void Expression() override;
	
	virtual void EmitConst(const PyValue& v) override;
	virtual void EmitName(int code, const String& name) override;

	void JsEmit(int code);
	void JsEmit(int code, int iarg);

public:
	JsCompiler(const Vector<Token>& tokens, String file = String()) : PyCompiler(tokens, file) {}
	
	void Compile(Vector<PyIR>& out);
};

END_UPP_NAMESPACE

#endif
