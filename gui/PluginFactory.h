#ifndef PLUGINFACTORY_H
#define PLUGINFACTORY_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QFile>

class PluginFactory : public QObject
{
    Q_OBJECT
public:
	explicit PluginFactory(QObject *parent = 0);

public:
	int Build(QString fileName);

private:
	void writeHeader();
	void computeChecksum();
	void generateFileArray(QStringList &v, QTextStream &sArray);
	void generateArray();

public:
	QString name, version, forb1gMail, vendor, vendorURL, vendorEMail, tag;
	QStringList classNames, pluginFiles, templateFiles, imageFiles, cssFiles, jsFiles;
	char checkSum[33];

private:
	QFile *fp;
};

#endif // PLUGINFACTORY_H
