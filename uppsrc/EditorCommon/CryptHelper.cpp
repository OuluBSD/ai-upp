#include <EditorCommon/CryptHelper.h>

NAMESPACE_UPP

String HashBuf::ToString() const
{
	return HexString(GetData(), GetDataSize());
}

bool HashBuf::FromString(const String &text)
{
	if (text.GetLength() != GetDataSize() * 2)
		return false;
	String b = ScanHexString(text);
	if (b.GetLength() != GetDataSize())
		return false;
	memcpy(GetData(), b.Begin(), GetDataSize());
	return true;
}

bool HashBuf::IsZero() const
{
	const byte *data = GetData();
	int size = GetDataSize();
	for (int i = 0; i < size; ++i) {
		if (data[i] != 0)
			return false;
	}
	return true;
}

bool HashBuf::operator==(const HashBuf &other) const
{
	if (GetDataSize() != other.GetDataSize())
		return false;
	return memcmp(GetData(), other.GetData(), GetDataSize()) == 0;
}

bool HashBuf::operator<(const HashBuf &other) const
{
	if (GetDataSize() != other.GetDataSize())
		return GetDataSize() < other.GetDataSize();
	return memcmp(GetData(), other.GetData(), GetDataSize()) < 0;
}

MD5Buf::MD5Buf()
{
	memset(m_data, 0, MD5_DATA_SIZE);
}

SHA1Buf::SHA1Buf()
{
	memset(m_data, 0, SHA1_DATA_SIZE);
}

// CryptHelper implementations will need OpenSSL or U++ Core/SSL.
// For now I'll stub them or use U++ equivalents if possible.
// U++ has Md5Stream, Sha1Stream etc.

END_UPP_NAMESPACE
