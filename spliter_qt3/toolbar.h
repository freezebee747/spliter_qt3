#pragma once
#include <QToolBar>

class ToolBar : public QToolBar {
	Q_OBJECT
public:
	QAction* newAction;
	QAction* openAction;
	QAction* saveAction;
	explicit ToolBar(QWidget* parent = nullptr);
};