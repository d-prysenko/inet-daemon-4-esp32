#include "HTTPtoWSPacket.h"

//char HTTPtoWSPacket::GUID[37] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

HTTPtoWSPacket::HTTPtoWSPacket()
{
}

HTTPtoWSPacket::HTTPtoWSPacket(const char* content)
{
	Parse(content);
}

HTTPtoWSPacket::~HTTPtoWSPacket()
{
}

/*
https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers

	An HTTP header consists of its case-insensitive name followed by a colon (:), then by its value.
	Whitespace before the value is ignored

rfc2616

	Both types of message consist of a start-line, zero
	or more header fields (also known as "headers"), an empty line (i.e.,
	a line with nothing preceding the CRLF) indicating the end of the
	header fields, and possibly a message-body.

	start-line      = Request-Line | Status-Line

*/

std::istream& getlineCRLF(std::istream& is, std::string& t)
{
	t.clear();

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	for (;;) {
		int c = sb->sbumpc();
		switch (c) {
		case '\n':
			return is;
		case '\r':
			if (sb->sgetc() == '\n')
				sb->sbumpc();
			return is;
		case std::streambuf::traits_type::eof():
			// Also handle the case when the last line has no line ending
			if (t.empty())
				is.setstate(std::ios::eofbit);
			return is;
		default:
			t += (char)c;
		}
	}
}


void HTTPtoWSPacket::Parse(const char* source)
{
	std::string temp;
	std::stringstream content;
	content << source;

	getlineCRLF(content, temp); // start-line
	while (getlineCRLF(content, temp) && temp != "")
	{
		size_t delim = temp.find(':');
		if (delim != std::string::npos && delim != temp.length() - 1)
		{
			std::string field = temp.substr(0, delim); trim(field); to_lower(field);
			std::string value = temp.substr(delim + 1, std::string::npos); trim(value);

			/*
			
				The |Sec-WebSocket-Key| header field in the client's handshake
				includes a base64-encoded value that, if decoded, is 16 bytes
				in length.  This (encoded) value is used in the creation of
				the server's handshake to indicate an acceptance of the
				connection.  It is not necessary for the server to base64-
				decode the |Sec-WebSocket-Key| value.
			
			*/

			
			if (field == "sec-websocket-key")
			{
				strncpy_s(Sec_WebSocket_Key, sizeof(Sec_WebSocket_Key), value.c_str(), _TRUNCATE);
				isKeySet = true;
			}
		}
	}

}

std::string HTTPtoWSPacket::getWebSocketAccept()
{
	uint8_t hash[20];
	std::string buf = Sec_WebSocket_Key;
	buf += HTTPtoWSPacket::GUID;
	get_sha1(buf, hash);

	
	char output[30];
	uint8_t end = boost::beast::detail::base64::encode(output, hash, 20);
	output[end] = '\0';

	return std::string(output);
}


// dest length MUST BE minimum 20 bytes
void HTTPtoWSPacket::get_sha1(const std::string& p_arg, uint8_t* dest)
{
	boost::uuids::detail::sha1 sha1;
	sha1.process_bytes(p_arg.data(), p_arg.size());
	unsigned hash[5] = { 0 };
	sha1.get_digest(hash);

	uint8_t* ptr = (uint8_t*)hash;
	for (int i = 0; i < 5; i++)
	{
		for (int j = 3; j >= 0; j--)
			(*dest++) = ptr[j];
		ptr = ptr + 4;
	}

}