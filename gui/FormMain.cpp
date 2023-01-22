#include "FormMain.h"
#include "MetaTab.h"
#include "FilesTab.h"
#include "PluginFactory.h"
#include <QtGui>

FormMain::FormMain(QWidget *parent) : QMainWindow(parent)
{
	this->setWindowTitle(tr("BMPluginBuilder"));
	this->setWindowIcon(QIcon(":/icons/app.png"));
	this->setUnifiedTitleAndToolBarOnMac(true);

	this->createActions();
	this->createMenu();
	this->createToolBar();
	this->statusBar();
	this->createTabWidget();

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget, 1);
	widgetMain->setLayout(mainLayout);
	this->setCentralWidget(widgetMain);

	this->resize(QSize(700, 640));
	this->modified = false;

	QSettings settings;
	this->move(settings.value("x", x()).toInt(), settings.value("y", y()).toInt());

	if(QApplication::argc() > 1
	   && QFileInfo(QApplication::argv()[1]).exists())
	{
		this->openProject(QApplication::argv()[1]);
	}
	else
	{
		this->newProject();
	}
}

FormMain::~FormMain()
{
}

bool FormMain::saveQuestion()
{
	if(!modified)
		return(true);

	int res = QMessageBox::question(this, tr("Unsaved changed"), tr("Do you want to save the changes you made to the project?"), tr("Yes"), tr("No"), tr("Cancel"), 0, 2);

	if((res == 0 && !this->saveProject()) || res == 2)
		return(false);

	return(true);
}

void FormMain::closeEvent(QCloseEvent *event)
{
	if(!saveQuestion())
	{
		event->ignore();
	}
	else
	{
		QSettings settings;
		settings.setValue("x", x());
		settings.setValue("y", y());
		settings.setValue("w", width());
		settings.setValue("h", height());
	}
}

void FormMain::setModified(bool modified)
{
	this->modified = modified;
	this->setCurrentFile(curFile);
	setWindowModified(modified);
}

void FormMain::openProject()
{
	if(!saveQuestion())
		return;

	QString fileName = QFileDialog::getOpenFileName(this, tr("Open project"), "", tr("BMPluginBuilder projects (*.bmproj)"));
	if(fileName.isEmpty())
		return;

	this->openProject(fileName);
}

void FormMain::openRecentFile()
{
	if(!saveQuestion())
		return;

	QAction *action = qobject_cast<QAction *>(sender());
	if(action != NULL)
		this->openProject(action->data().toString());
}

void FormMain::updateRecentFileActions()
{
	QSettings settings;
	QStringList files = settings.value("recentFileList").toStringList();

	int numRecentFiles = qMin(files.size(), MAX_RECENT_FILES), n = 0;

	for(int i=0; i<numRecentFiles; i++)
	{
		if(QFileInfo(files.at(i)).exists())
		{
			QString text = tr("&%1 %2").arg(n+1).arg(QFileInfo(files.at(i)).fileName());
			recentFileActions[n]->setText(text);
			recentFileActions[n]->setData(files.at(i));
			recentFileActions[n]->setVisible(true);
			n++;
		}
	}
	for(int j=n; j<MAX_RECENT_FILES; j++)
		recentFileActions[j]->setVisible(false);

	menuRecent->menuAction()->setVisible(n > 0);
}

void FormMain::openProject(QString fileName)
{
	QSettings s(fileName, QSettings::IniFormat);

	s.beginGroup("bmproj");
	if(s.value("type", "") != "BMPluginBuilder project")
	{
		QMessageBox::critical(this, tr("Error"), tr("This file is not a valid BMPluginBuilder project."));
		return;
	}
	this->lastBuildDest = s.value("last_build_dest", "").toString();
	s.endGroup();

	this->setCurrentFile(fileName);
	this->tabMeta->resetTab();
	this->tabFiles->resetTab();
	this->tabWidget->setCurrentIndex(0);

	s.beginGroup("meta");
	tabMeta->editName->setText(s.value("name", "").toString());
	tabMeta->editVersion->setText(s.value("version", "").toString());
	tabMeta->editForb1gMail->setText(s.value("forb1gmail", "").toString());
	tabMeta->editVendor->setText(s.value("vendor", "").toString());
	tabMeta->editVendorURL->setText(s.value("vendor_url", "").toString());
	tabMeta->editVendorEMail->setText(s.value("vendor_email", "").toString());
	s.endGroup();

	int size = s.beginReadArray("classes");
	for(int i=0; i<size; i++)
	{
		s.setArrayIndex(i);
		tabMeta->addClass(s.value("name").toString());
	}
	s.endArray();

	QStringList fileList;
	size = s.beginReadArray("files");
	for(int i=0; i<size; i++)
	{
		s.setArrayIndex(i);
		fileList.append(s.value("path").toString());
	}
	s.endArray();

	tabFiles->treeFiles->addFiles(fileList);

	this->setModified(false);
}

bool FormMain::saveProject(QString fileName)
{
	if(QFileInfo(fileName).exists())
	{
		QFile file(fileName);
		if(!file.open(QIODevice::WriteOnly) || !file.resize(0))
		{
			QMessageBox::critical(this, tr("Error"), tr("Failed to write to the specified file."));
			return(false);
		}
		file.close();
	}

	QSettings s(fileName, QSettings::IniFormat);
	s.beginGroup("bmproj");
	s.setValue("type", "BMPluginBuilder project");
	s.setValue("last_build_dest", lastBuildDest);
	s.endGroup();

	s.beginGroup("meta");
	s.setValue("name", tabMeta->editName->text());
	s.setValue("version", tabMeta->editVersion->text());
	s.setValue("forb1gmail", tabMeta->editForb1gMail->text());
	s.setValue("vendor", tabMeta->editVendor->text());
	s.setValue("vendor_url", tabMeta->editVendorURL->text());
	s.setValue("vendor_email", tabMeta->editVendorEMail->text());
	s.endGroup();

	s.beginWriteArray("classes");
	for(int i=0; i<tabMeta->listClasses->count(); i++)
	{
		s.setArrayIndex(i);
		s.setValue("name", tabMeta->listClasses->item(i)->text());
	}
	s.endArray();

	QStringList fileList = tabFiles->treeFiles->fileList();
	s.beginWriteArray("files");
	for(int i=0; i<fileList.count(); i++)
	{
		s.setArrayIndex(i);
		s.setValue("path", fileList.at(i));
	}
	s.endArray();

	s.sync();

	this->setCurrentFile(fileName);
	this->setModified(false);

	return(true);
}

bool FormMain::saveProject()
{
	if(this->curFile.isEmpty() || !QFileInfo(this->curFile).exists())
		return this->saveProjectAs();
	else
		return this->saveProject(this->curFile);
}

bool FormMain::saveProjectAs()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save project"), this->curFile, tr("BMPluginBuilder projects (*.bmproj)"));
	if(fileName.isEmpty())
		return(false);

	return this->saveProject(fileName);
}

void FormMain::newProject()
{
	if(!saveQuestion())
		return;

	this->lastBuildDest = "";
	this->setCurrentFile("");
	this->setModified(false);
	this->tabMeta->resetTab();
	this->tabFiles->resetTab();
	this->tabWidget->setCurrentIndex(0);
}

void FormMain::setCurrentFile(QString fileName)
{
	if(curFile != fileName && fileName != "" && QFile(fileName).exists())
	{
		QSettings settings;
		QStringList files = settings.value("recentFileList").toStringList();
		files.removeAll(fileName);
		files.prepend(fileName);
		if(files.size() > MAX_RECENT_FILES)
			files.removeLast();
		settings.setValue("recentFileList", files);
		updateRecentFileActions();
	}

	curFile = fileName;

	QFileInfo theFile(fileName);
	QString displayName = theFile.baseName();

	if(displayName.isEmpty())
		displayName = tr("NewProject");

	setWindowTitle(tr("%1%2 - %3").arg(displayName).arg(modified?"[*]":"").arg(QApplication::applicationName()));
	setWindowFilePath(fileName);
}

void FormMain::buildPackage()
{
	if(!this->checkRequiredData())
		return;

	QString fileName = lastBuildDest + tr("%1-%2.bmplugin").arg(tabMeta->listClasses->item(0)->text()).arg(tabMeta->editVersion->text());
	fileName = QFileDialog::getSaveFileName(this, tr("Select destination"), fileName, tr("b1gMail plugin packages (*.bmplugin)"));

	if(fileName.isEmpty())
		return;

	if(lastBuildDest != QFileInfo(fileName).path()+"/")
	{
		lastBuildDest = QFileInfo(fileName).path()+"/";
		this->setModified(true);
	}

	QString hash;
	int size;
	if((size = buildPackage(fileName, hash)) <= 0)
	{
		QMessageBox::critical(this, tr("Error"), tr("An error occured during the build process."));
	}
	else
	{
		QString message = tr("Package built (size: %1 bytes, checksum: %2)").arg(size).arg(hash);
		statusBar()->showMessage(message);
	}
}

int FormMain::buildPackage(QString fileName, QString &hash)
{
	PluginFactory f;
	f.name = this->tabMeta->editName->text();
	f.version  = this->tabMeta->editVersion->text();
	f.forb1gMail = this->tabMeta->editForb1gMail->text();
	f.vendor = this->tabMeta->editVendor->text();
	f.vendorURL = this->tabMeta->editVendorURL->text();
	f.vendorEMail = this->tabMeta->editVendorEMail->text();
	f.tag = QApplication::applicationName() + "/" + QApplication::applicationVersion();

	for(int i=0; i<this->tabMeta->listClasses->count(); i++)
		f.classNames.append(this->tabMeta->listClasses->item(i)->text());

	for(int i=0; i<this->tabFiles->treeFiles->itemPlugins->childCount(); i++)
		f.pluginFiles.append(this->tabFiles->treeFiles->itemPlugins->child(i)->text(1));
	for(int i=0; i<this->tabFiles->treeFiles->itemTemplates->childCount(); i++)
		f.templateFiles.append(this->tabFiles->treeFiles->itemTemplates->child(i)->text(1));
	for(int i=0; i<this->tabFiles->treeFiles->itemImages->childCount(); i++)
		f.imageFiles.append(this->tabFiles->treeFiles->itemImages->child(i)->text(1));
	for(int i=0; i<this->tabFiles->treeFiles->itemCSS->childCount(); i++)
		f.cssFiles.append(this->tabFiles->treeFiles->itemCSS->child(i)->text(1));
	for(int i=0; i<this->tabFiles->treeFiles->itemJS->childCount(); i++)
		f.jsFiles.append(this->tabFiles->treeFiles->itemJS->child(i)->text(1));

	int pluginSize = f.Build(fileName);

	hash.clear();
	hash.append(f.checkSum);

	return(pluginSize);
}

bool FormMain::checkRequiredData()
{
	if(this->tabMeta->listClasses->count() == 0)
	{
		QMessageBox::critical(this, tr("Missing class name"), tr("At least one plugin class name is required. Please add a plugin class name and try again."));
		return(false);
	}

	if(this->tabFiles->treeFiles->itemPlugins->childCount() == 0)
	{
		QMessageBox::critical(this, tr("Missing plugin file"), tr("You need to add at least one PHP plugin file. Please add a PHP plugin file and try again."));
		return(false);
	}

	QString missingFiles = "";
	QStringList fileNames = this->tabFiles->treeFiles->fileList();
	for(int i=0; i<fileNames.count(); i++)
	{
		QFileInfo theFile(fileNames.at(i));
		if(!theFile.exists())
		{
			if(missingFiles.isEmpty())
				missingFiles.append("- ");
			else
				missingFiles.append("\n- ");
			missingFiles.append(fileNames.at(i));
		}
	}

	if(!missingFiles.isEmpty())
	{
		QMessageBox::critical(this, tr("Missing files"), tr("The following files do not exist anymore.\n\n%1\n\nPlease restore the files or remove them from the project and try again.").arg(missingFiles));
		return(false);
	}

	return(true);
}

void FormMain::createTabWidget()
{
	widgetMain = new QWidget(this);

	tabFiles = new FilesTab(this);
	QObject::connect(tabFiles,
					 SIGNAL(dataChanged()),
					 this,
					 SLOT(setModified()));

	tabMeta = new MetaTab(this, tabFiles);
	QObject::connect(tabMeta,
					 SIGNAL(dataChanged()),
					 this,
					 SLOT(setModified()));

	tabWidget = new QTabWidget(widgetMain);
	tabWidget->addTab(tabMeta, QIcon(":/icons/meta16.png"), tr("Meta information"));
	tabWidget->addTab(tabFiles, QIcon(":/icons/files16.png"), tr("Files"));
}

void FormMain::createActions()
{
	actionNewProject = new QAction(QIcon(":/icons/new.png"), tr("&New"), this);
	actionNewProject->setShortcuts(QKeySequence::New);
	actionNewProject->setStatusTip(tr("Create a new project"));
	QObject::connect(actionNewProject,
					 SIGNAL(triggered()),
					 this,
					 SLOT(newProject()));

	actionOpenProject = new QAction(QIcon(":/icons/open.png"), tr("&Open..."), this);
	actionOpenProject->setShortcuts(QKeySequence::Open);
	actionOpenProject->setStatusTip(tr("Open an existing project"));
	QObject::connect(actionOpenProject,
					 SIGNAL(triggered()),
					 this,
					 SLOT(openProject()));

	actionSaveProject = new QAction(QIcon(":/icons/save.png"), tr("&Save"), this);
	actionSaveProject->setShortcuts(QKeySequence::Save);
	actionSaveProject->setStatusTip(tr("Save the project"));
	QObject::connect(actionSaveProject,
					 SIGNAL(triggered()),
					 this,
					 SLOT(saveProject()));

	actionSaveProjectAs = new QAction(QIcon(":/icons/saveas.png"), tr("Save &As..."), this);
	actionSaveProjectAs->setShortcuts(QKeySequence::SaveAs);
	actionSaveProjectAs->setStatusTip(tr("Save the project under a new name"));
	QObject::connect(actionSaveProjectAs,
					 SIGNAL(triggered()),
					 this,
					 SLOT(saveProjectAs()));

	actionBuild = new QAction(QIcon(":/icons/build.png"), tr("&Build..."), this);
	actionBuild->setShortcut(QKeySequence("F5"));
	actionBuild->setStatusTip(tr("Export the project to a .bmplugin package"));
	QObject::connect(actionBuild,
					 SIGNAL(triggered()),
					 this,
					 SLOT(buildPackage()));

	actionHelp = new QAction(QIcon(":/icons/help.png"), tr("&Contents"), this);
	QObject::connect(actionHelp,
					 SIGNAL(triggered()),
					 this,
					 SLOT(showHelp()));

	actionAbout = new QAction(tr("&About"), this);
	QObject::connect(actionAbout,
					 SIGNAL(triggered()),
					 this,
					 SLOT(aboutBox()));

	actionQuit = new QAction(QIcon(":/icons/quit.png"), tr("E&xit"), this);

	QObject::connect(actionQuit,
					 SIGNAL(triggered()),
					 this,
					 SLOT(close()));

	for(int i=0; i<MAX_RECENT_FILES; i++)
	{
		recentFileActions[i] = new QAction(this);
		recentFileActions[i]->setVisible(false);
		QObject::connect(recentFileActions[i],
						 SIGNAL(triggered()),
						 this,
						 SLOT(openRecentFile()));
	}
}

void FormMain::createMenu()
{
	QMenu *menuProject = this->menuBar()->addMenu(tr("&Project"));
	menuProject->addAction(actionNewProject);
	menuProject->addAction(actionOpenProject);
	menuRecent = menuProject->addMenu(tr("Recent projects"));
	menuProject->addSeparator();
	menuProject->addAction(actionSaveProject);
	menuProject->addAction(actionSaveProjectAs);
	menuProject->addSeparator();
	menuProject->addAction(actionQuit);

	for(int i=0; i<MAX_RECENT_FILES; i++)
		menuRecent->addAction(recentFileActions[i]);

	QMenu *menuPluginPackage = this->menuBar()->addMenu(tr("P&lugin package"));
	menuPluginPackage->addAction(actionBuild);

	QMenu *menuHelp = this->menuBar()->addMenu(tr("&Help"));
	menuHelp->addAction(actionHelp);
	menuHelp->addSeparator();
	menuHelp->addAction(actionAbout);

	this->updateRecentFileActions();
}

void FormMain::createToolBar()
{
	QToolBar *tb = this->addToolBar(tr("Toolbar"));
	tb->setFloatable(false);
	tb->setMovable(false);
	tb->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	tb->addAction(actionNewProject);
	tb->addAction(actionOpenProject);
	tb->addAction(actionSaveProject);
	tb->addSeparator();
	tb->addAction(actionBuild);
}

void FormMain::showHelp()
{
	QDesktopServices::openUrl(QUrl(tr("http://help.b1gmail.com/jump.php?lang=en&file=bmpluginbuilder")));
}

void FormMain::aboutBox()
{
	QMessageBox::about(this, tr("About"), tr("BMPluginBuilder %1\n\n(c) 2002-2010 B1G Software").arg(QApplication::applicationVersion()));
}
