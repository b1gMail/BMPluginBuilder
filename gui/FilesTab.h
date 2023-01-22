#ifndef FILESTAB_H
#define FILESTAB_H

#include <QWidget>
#include <QTreeWidget>

class QToolButton;

class FilesTreeWidget : public QTreeWidget
{
	Q_OBJECT

public:
	explicit FilesTreeWidget(QWidget *parent = 0);

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);

public:
	void addFiles(QStringList fileList);
	void resetFiles();
	QStringList fileList();

public:
	QTreeWidgetItem *itemPlugins, *itemTemplates, *itemImages, *itemCSS, *itemJS;

signals:
	void dataChanged();
};

class FilesTab : public QWidget
{
	Q_OBJECT
public:
    explicit FilesTab(QWidget *parent = 0);

public:
	FilesTreeWidget *treeFiles;

private:
	QToolButton *buttonAddFile, *buttonRemoveFile;

signals:
	void dataChanged();

public slots:
	void resetTab();
	void treeSelectionChanged();
	void addFile();
	void removeFile();
	void filesChanged();
};

#endif // FILESTAB_H
