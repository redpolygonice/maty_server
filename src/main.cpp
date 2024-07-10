#include <QCoreApplication>
#include "dispatcher.h"
#include "settings.h"
#include "log.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	GetSettings()->load();
	Log::create();
	if (!GetDispatcher()->start())
		return -1;
	LOG("WebSocket server started!");
	return a.exec();
}
