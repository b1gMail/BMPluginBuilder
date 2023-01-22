#include "FilesTab.h"
#include <QtGui>

FilesTreeWidget::FilesTreeWidget(QWidget *parent) : QTreeWidget(parent)
{
	setColumnCount(2);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setAcceptDrops(true);

	QStringList l;
	l.append(tr("Filename"));
	l.append(tr("Path"));
	setHeaderLabels(l);
	header()->resizeSection(0, 400);

	resetFiles();
}

void FilesTreeWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if(event->mimeData()->hasFormat("text/uri-list"))
	{
		event->setDropAction(Qt::LinkAction);
		event->acceptProposedAction();
	}
}

void FilesTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

void FilesTreeWidget::dropEvent(QDropEvent *event)
{
	QStringList files;
	QList<QUrl> urls = event->mimeData()->urls();
	if(urls.isEmpty())
		return;

	for(int i=0; i<urls.count(); i++)
	{
		QString fileName = urls.at(i).toLocalFile();
		if(fileName.isEmpty())
			continue;
		files.append(fileName);
	}

	addFiles(files);

	event->accept();
}

void FilesTreeWidget::addFiles(QStringList fileList)
{
	bool duplicates = false, changed = false;

	for(int i=0; i<fileList.count(); i++)
	{
		QFileInfo theFile(fileList.at(i));
		if(!theFile.exists())
			continue;

		QString fileName = theFile.fileName(), ext = theFile.suffix();
		QStringList treeItem;
		treeItem.append(fileName);
		treeItem.append(fileList.at(i));

		ext = ext.toLower();

		QTreeWidgetItem *parent = NULL;
		if(ext == "php")
			parent = itemPlugins;
		else if(ext == "tpl")
			parent = itemTemplates;
		else if(ext == "gif" || ext == "png" || ext == "jpg")
			parent = itemImages;
		else if(ext == "css")
			parent = itemCSS;
		else if(ext == "js")
			parent = itemJS;

		if(parent != NULL)
		{
			bool exists = false;

			for(int j=0; j<parent->childCount(); j++)
			{
				if(parent->child(j)->text(0) == fileName)
				{
					exists = duplicates = true;
				}
			}

			if(exists)
				continue;

			(new QTreeWidgetItem(parent, treeItem))->setIcon(0, QIcon(":/icons/file16.png"));
			changed = true;
		}
	}

	if(changed)
		emit dataChanged();

	if(duplicates)
	{
		QMessageBox::warning(this, tr("Duplicate file(s)"), tr("One or more files have not been added because a file with a same name already exists."));
	}
}

void FilesTreeWidget::resetFiles()
{
	clear();

	itemPlugins = new QTreeWidgetItem(this, QStringList(tr("Plugins")));
	itemPlugins->setIcon(0, QIcon(":/icons/folder16.png"));
	itemPlugins->setFlags(itemPlugins->flags() & ~Qt::ItemIsSelectable);

	itemTemplates = new QTreeWidgetItem(this, QStringList(tr("Templates")));
	itemTemplates->setIcon(0, QIcon(":/icons/folder16.png"));
	itemTemplates->setFlags(itemPlugins->flags() & ~Qt::ItemIsSelectable);

	itemImages = new QTreeWidgetItem(this, QStringList(tr("Images")));
	itemImages->setIcon(0, QIcon(":/icons/folder16.png"));
	itemImages->setFlags(itemPlugins->flags() & ~Qt::ItemIsSelectable);

	itemCSS = new QTreeWidgetItem(this, QStringList(tr("Stylesheets")));
	itemCSS->setIcon(0, QIcon(":/icons/folder16.png"));
	itemCSS->setFlags(itemPlugins->flags() & ~Qt::ItemIsSelectable);

	itemJS = new QTreeWidgetItem(this, QStringList(tr("JavasScript")));
	itemJS->setIcon(0, QIcon(":/icons/folder16.png"));
	itemJS->setFlags(itemPlugins->flags() & ~Qt::ItemIsSelectable);

	expandAll();
}

QStringList FilesTreeWidget::fileList()
{
	QStringList l;

	for(int i=0; i<itemPlugins->childCount(); i++)
		l.append(itemPlugins->child(i)->text(1));
	for(int i=0; i<itemTemplates->childCount(); i++)
		l.append(itemTemplates->child(i)->text(1));
	for(int i=0; i<itemImages->childCount(); i++)
		l.append(itemImages->child(i)->text(1));
	for(int i=0; i<itemCSS->childCount(); i++)
		l.append(itemCSS->child(i)->text(1));
	for(int i=0; i<itemJS->childCount(); i++)
		l.append(itemJS->child(i)->text(1));

	return(l);
}

FilesTab::FilesTab(QWidget *parent) : QWidget(parent)
{
	treeFiles = new FilesTreeWidget(this);
	QObject::connect(treeFiles,
					 SIGNAL(itemSelectionChanged()),
					 this,
					 SLOT(treeSelectionChanged()));
	QObject::connect(treeFiles,
					 SIGNAL(dataChanged()),
					 this,
					 SLOT(filesChanged()));

	buttonAddFile = new QToolButton;
	buttonAddFile->setIcon(QIcon(":/icons/add16.png"));
	buttonAddFile->setToolTip(tr("Add"));
	QObject::connect(buttonAddFile,
					 SIGNAL(clicked()),
					 this,
					 SLOT(addFile()));

	buttonRemoveFile = new QToolButton;
	buttonRemoveFile->setIcon(QIcon(":/icons/remove16.png"));
	buttonRemoveFile->setToolTip(tr("Remove"));
	QObject::connect(buttonRemoveFile,
					 SIGNAL(clicked()),
					 this,
					 SLOT(removeFile()));

	QVBoxLayout *buttonsLayout = new QVBoxLayout;
	buttonsLayout->addWidget(buttonAddFile);
	buttonsLayout->addWidget(buttonRemoveFile);
	buttonsLayout->addStretch();

	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(treeFiles, 1);
	layout->addLayout(buttonsLayout);
	setLayout(layout);

	this->resetTab();
}

void FilesTab::resetTab()
{
	treeFiles->resetFiles();
	buttonRemoveFile->setDisabled(true);
}

void FilesTab::treeSelectionChanged()
{
	buttonRemoveFile->setDisabled(treeFiles->selectedItems().count() == 0);
}

void FilesTab::addFile()
{
	QStringList fileList = QFileDialog::getOpenFileNames(this, tr("Select files"), "", tr("Supported files (*.php *.tpl *.gif *.png *.jpg *.css *.js)"));
	treeFiles->addFiles(fileList);
}

void FilesTab::removeFile()
{
	qDeleteAll(treeFiles->selectedItems());
}

void FilesTab::filesChanged()
{
	emit dataChanged();
}
