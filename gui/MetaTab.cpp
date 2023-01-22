#include "MetaTab.h"
#include <QtGui>

MetaTab::MetaTab(QWidget *parent, FilesTab *tabFiles) : QWidget(parent)
{
	this->tabFiles = tabFiles;

	editName = new QLineEdit;
	editVersion = new QLineEdit;
	editVersion->setFixedWidth(100);
	editForb1gMail = new QLineEdit;
	editForb1gMail->setFixedWidth(100);
	editVendor = new QLineEdit;
	editVendorURL = new QLineEdit;
	editVendorEMail = new QLineEdit;

	QObject::connect(editName,
					 SIGNAL(textEdited(QString)),
					 this,
					 SLOT(textChanged()));
	QObject::connect(editVersion,
					 SIGNAL(textEdited(QString)),
					 this,
					 SLOT(textChanged()));
	QObject::connect(editForb1gMail,
					 SIGNAL(textEdited(QString)),
					 this,
					 SLOT(textChanged()));
	QObject::connect(editVendor,
					 SIGNAL(textEdited(QString)),
					 this,
					 SLOT(textChanged()));
	QObject::connect(editVendorURL,
					 SIGNAL(textEdited(QString)),
					 this,
					 SLOT(textChanged()));
	QObject::connect(editVendorEMail,
					 SIGNAL(textEdited(QString)),
					 this,
					 SLOT(textChanged()));

	listClasses = new QListWidget;
	listClasses->setSelectionMode(QAbstractItemView::ExtendedSelection);
	listClasses->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);
	QObject::connect(listClasses,
					 SIGNAL(itemSelectionChanged()),
					 this,
					 SLOT(listSelectionChanged()));

	buttonAddClass = new QToolButton;
	buttonAddClass->setIcon(QIcon(":/icons/add16.png"));
	buttonAddClass->setToolTip(tr("Add"));
	QObject::connect(buttonAddClass,
					 SIGNAL(clicked()),
					 this,
					 SLOT(addClass()));

	buttonRemoveClass = new QToolButton;
	buttonRemoveClass->setIcon(QIcon(":/icons/remove16.png"));
	buttonRemoveClass->setToolTip(tr("Remove"));
	QObject::connect(buttonRemoveClass,
					 SIGNAL(clicked()),
					 this,
					 SLOT(removeClass()));

	buttonDetectClass = new QToolButton;
	buttonDetectClass->setIcon(QIcon(":/icons/detect16.png"));
	buttonDetectClass->setToolTip(tr("Auto detect class names"));
	QObject::connect(buttonDetectClass,
					 SIGNAL(clicked()),
					 this,
					 SLOT(detectClass()));

	QVBoxLayout *classesButtonsLayout = new QVBoxLayout;
	classesButtonsLayout->addWidget(buttonAddClass);
	classesButtonsLayout->addWidget(buttonRemoveClass);
	classesButtonsLayout->addStretch();
	classesButtonsLayout->addWidget(buttonDetectClass);

	QHBoxLayout *classesLayout = new QHBoxLayout;
	classesLayout->addWidget(listClasses, 1);
	classesLayout->addLayout(classesButtonsLayout);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(new QLabel(tr("Plugin package name:")));
	layout->addWidget(editName);
	layout->addWidget(new QLabel(tr("Plugin package version:")));
	layout->addWidget(editVersion);
	layout->addWidget(new QLabel(tr("Developed for b1gMail version:")));
	layout->addWidget(editForb1gMail);
	layout->addWidget(new QLabel(tr("Vendor name:")));
	layout->addWidget(editVendor);
	layout->addWidget(new QLabel(tr("Vendor website:")));
	layout->addWidget(editVendorURL);
	layout->addWidget(new QLabel(tr("Vendor email:")));
	layout->addWidget(editVendorEMail);
	layout->addWidget(new QLabel(tr("Plugin classes:")));
	layout->addLayout(classesLayout, 1);

	setLayout(layout);

	this->resetTab();
}

void MetaTab::resetTab()
{
	editName->setText(tr("Example plugin package"));
	editVersion->setText(tr("1.0.0"));
	editForb1gMail->setText(tr("7.2.0"));
	editVendor->setText(tr("Example vendor"));
	editVendorURL->setText(tr("http://www.example.com/"));
	editVendorEMail->setText(tr("contact@example.com"));

	listClasses->clear();

	buttonRemoveClass->setDisabled(true);
}

void MetaTab::listSelectionChanged()
{
	buttonRemoveClass->setDisabled(listClasses->selectedItems().count() == 0);
}

void MetaTab::addClass()
{
	listClasses->setFocus();
	listClasses->editItem(addClass("MyPlugin"));

	emit dataChanged();
}

void MetaTab::removeClass()
{
	if(listClasses->selectedItems().count() > 0)
	{
		qDeleteAll(listClasses->selectedItems());
		emit dataChanged();
	}
}

void MetaTab::detectClass()
{
	if(this->tabFiles->treeFiles->itemPlugins->childCount() < 1)
	{
		QMessageBox::warning(this, tr("Error"), tr("In order to auto detect class names, please add at least one PHP plugin file in the \"Files\" tab."));
		return;
	}

	bool changed = false;

	for(int i=0; i<this->tabFiles->treeFiles->itemPlugins->childCount(); i++)
	{
		QString fileName = this->tabFiles->treeFiles->itemPlugins->child(i)->text(1);

		QFile fp(fileName);
		if(fp.open(QFile::ReadOnly | QFile::Text))
		{
			QTextStream s(&fp);
			QString line;

			while(!(line = s.readLine()).isNull())
			{
				int posClass = line.indexOf("class"), posExt = line.indexOf("extends BMPlugin"), posComment = line.indexOf("//");

				if(posClass != -1 && posExt != -1 && posExt > posClass + 2 && (posComment == -1 || posComment > posClass))
				{
					QString className = line.mid(posClass+5, posExt-posClass-5).trimmed();

					if(!className.isEmpty())
					{
						if(this->listClasses->findItems(className, Qt::MatchExactly).count() == 0)
						{
							addClass(className);
							changed = true;
						}
					}
				}
			}

			fp.close();
		}
	}

	if(changed)
		emit dataChanged();
}

QListWidgetItem *MetaTab::addClass(QString className)
{
	QListWidgetItem *newItem = new QListWidgetItem(className, listClasses);
	newItem->setIcon(QIcon(":/icons/class16.png"));
	newItem->setFlags(Qt::ItemIsEditable | newItem->flags());
	return newItem;
}

void MetaTab::textChanged()
{
	emit dataChanged();
}
