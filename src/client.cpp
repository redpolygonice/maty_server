#include "client.h"

Client::Client(int id, QWebSocket *socket)
	: id_(id)
	, socket_(socket)
{
}
