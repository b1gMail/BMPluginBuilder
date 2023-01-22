#ifndef FORMMAIN_H
#define FORMMAIN_H

#include <QtGui/QMainWindow>

class QTabWidget;
class MetaTab;
class FilesTab;

#define MAX_RECENT_FILES	8

class FormMain : public QMainWindow
{
    Q_OBJECT

public:
	FormMain(QWidget *parent = 0);
	~FormMain();

public slots:
	void newProject();
	void openProject();
	bool saveProject();
	bool saveProjectAs();
	void buildPackage();
	void setModified(bool modified = true);
	void aboutBox();
	void showHelp();
	void openRecentFile();

public:
	void openProject(QString fileName);
	bool saveQuestion();

private:
	void createActions();
	void createMenu();
	void createToolBar();
	void createTabWidget();
	bool checkRequiredData();
	int buildPackage(QString fileName, QString &hash);
	void setCurrentFile(QString fileName);
	bool saveProject(QString fileName);
	void updateRecentFileActions();

protected:
	void closeEvent(QCloseEvent *event);

private:
	QMenu *menuRecent;
	QAction *actionNewProject, *actionOpenProject, *actionSaveProject, *actionSaveProjectAs, *actionBuild,
			*actionQuit, *actionAbout, *actionHelp;
	QTabWidget *tabWidget;
	QWidget *widgetMain;
	MetaTab *tabMeta;
	FilesTab *tabFiles;
	QString curFile;
	QString lastBuildDest;
	bool modified;
	QAction *recentFileActions[MAX_RECENT_FILES];
};

#endif // FORMMAIN_H
