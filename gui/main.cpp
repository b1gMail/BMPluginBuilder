#include <QtGui/QApplication>
#include <QFileOpenEvent>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include "FormMain.h"

class App : public QApplication
{
public:
	App(int &argc, char *argv[]) : QApplication(argc, argv)
	{
		setApplicationName("BMPluginBuilder");
		setApplicationVersion("2.0.0");
		setOrganizationName("B1G Software");
		setOrganizationDomain("b1g.de");

		qtTranslator.load("qt_" + QLocale::system().name(),
				QLibraryInfo::location(QLibraryInfo::TranslationsPath));
		installTranslator(&qtTranslator);

		myappTranslator.load(":/trans/bmpluginbuilder_" + QLocale::system().name());
		installTranslator(&myappTranslator);

		w = new FormMain;
		w->show();
	}

	~App()
	{
		delete w;
	}

protected:
	bool event(QEvent *event)
	{
		if(event->type() == QEvent::FileOpen)
		{
			QString fileName = static_cast<QFileOpenEvent *>(event)->file();

			if(w != NULL && w->saveQuestion())
				w->openProject(fileName);

			return(true);
		}
		else
			return QApplication::event(event);
	}

public:
	FormMain *w;
	QTranslator qtTranslator;
	QTranslator myappTranslator;
};

int main(int argc, char *argv[])
{
	App a(argc, argv);
    return a.exec();
}
