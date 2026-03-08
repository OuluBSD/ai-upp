#ifndef _CardEngine_CryptHelper_h_
#define _CardEngine_CryptHelper_h_

#include <Core/Core.h>

NAMESPACE_UPP

#define MD5_DATA_SIZE		16
#define SHA1_DATA_SIZE		20

#define AES_BLOCK_SIZE		16
#define ADD_PADDING(x) ((((x) + 15) >> 4) << 4)

class HashBuf
{
public:
	virtual ~HashBuf() {}

	String ToString() const;
	bool FromString(const String &text);
	bool IsZero() const;

	bool operator==(const HashBuf &other) const;
	bool operator<(const HashBuf &other) const;

	virtual byte *GetData() = 0;
	virtual const byte *GetData() const = 0;
	virtual int GetDataSize() const = 0;

	void Serialize(Stream& s) {
		int size = GetDataSize();
		if (s.IsLoading()) {
			s.Get(GetData(), size);
		} else {
			s.Put(GetData(), size);
		}
	}
};

class MD5Buf : public HashBuf
{
public:
	MD5Buf();

	virtual byte *GetData() override { return m_data; }
	virtual const byte *GetData() const override { return m_data; }
	virtual int GetDataSize() const override { return MD5_DATA_SIZE; }

private:
	byte m_data[MD5_DATA_SIZE];
};

class SHA1Buf : public HashBuf
{
public:
	SHA1Buf();

	virtual byte *GetData() override { return m_data; }
	virtual const byte *GetData() const override { return m_data; }
	virtual int GetDataSize() const override { return SHA1_DATA_SIZE; }

	byte m_data[SHA1_DATA_SIZE];
};

class CryptHelper
{
public:
	static bool MD5Sum(const String &fileName, MD5Buf &buf);
	static bool SHA1Hash(const byte *data, unsigned dataSize, SHA1Buf &buf);
	static bool HMACSha1(const byte *keyData, unsigned keySize, const byte *plainData, unsigned plainSize, SHA1Buf &buf);
	static bool AES128Encrypt(const byte *keyData, unsigned keySize, const String &plainStr, Vector<byte> &outCipher);
	static bool AES128Decrypt(const byte *keyData, unsigned keySize, const byte *cipher, unsigned cipherSize, String &outPlain);

private:
	static void BytesToKey(const byte *keyData, unsigned keySize, byte *key, byte *iv);
};

END_UPP_NAMESPACE

#endif
