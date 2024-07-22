#include <QCoreApplication>
#include "dispatcher.h"
#include "settings.h"
#include "log.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	Log::create();
	GetSettings()->load();
	if (!GetDispatcher()->start())
		return -1;
	LOG("WebSocket server started!");
	return a.exec();
}
