#include "toolbar.h"
#include <QAction>
#include <QIcon>
#include <QDebug>

ToolBar::ToolBar(QWidget* parent)
	: QToolBar("Main Toolbar", parent)
{
	setMovable(true);

	newAction = new QAction(QIcon::fromTheme("document-new"), "New", this);
	openAction = new QAction(QIcon::fromTheme("document-open"), "Open", this);
	saveAction = new QAction(QIcon::fromTheme("document-save"), "Save", this);

	connect(newAction, &QAction::triggered, this, []() { qDebug() << "New clicked"; });
	connect(openAction, &QAction::triggered, this, []() { qDebug() << "Open clicked"; });
	connect(saveAction, &QAction::triggered, this, []() { qDebug() << "Save clicked"; });

	addAction(newAction);
	addAction(openAction);
	addAction(saveAction);
}