#ifndef METATAB_H
#define METATAB_H

#include <QWidget>
#include "FilesTab.h"

class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QToolButton;

class MetaTab : public QWidget
{
    Q_OBJECT
public:
	explicit MetaTab(QWidget *parent, FilesTab *tabFiles);

public:
	QLineEdit *editName, *editVersion, *editForb1gMail, *editVendor, *editVendorURL, *editVendorEMail;
	QListWidget *listClasses;

public:
	QListWidgetItem *addClass(QString className);

private:
	QToolButton *buttonAddClass, *buttonRemoveClass, *buttonDetectClass;
	FilesTab *tabFiles;

signals:
	void dataChanged();

public slots:
	void resetTab();
	void addClass();
	void removeClass();
	void detectClass();
	void listSelectionChanged();
	void textChanged();
};

#endif // METATAB_H
