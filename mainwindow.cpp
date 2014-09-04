#include "mainwindow.h"
#include "mdichild.h"
#include <QMdiSubWindow>
#include <QFileDialog>
#include <QSignalMapper>
#include <QSettings>
#include <QCloseEvent>
#include <QLabel>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	ui.action_New->setShortcut(QKeySequence("Ctrl+N"));
	//创建间隔器动作并在其中设置间隔器
	actionSeparator=new QAction(this);
	actionSeparator->setSeparator(true);
	updateMenus();//更新菜单
	//当有活动窗口时更新菜单
	connect(ui.mdiArea,SIGNAL(subWindowActivated(QMdiSubWindow *)),this,SLOT(updateMenus()));
	windowMapper=new QSignalMapper(this);//创建信号映射器
	//映射器重新发射信号，根据信号设置窗口
	connect(windowMapper,SIGNAL(mapped(QWidget*)),
		this,SLOT(setActiveSubWindow(QWidget*)));
	//更新窗口菜单
	updateWindowMenu();
	connect(ui.menuW,SIGNAL(aboutToShow()),this,SLOT(updateWindowMenu()));
	readSettings();
	initWindow();
}

MainWindow::~MainWindow()
{

}

//更新菜单状态
void MainWindow::updateMenus()
{
	//根据是否有活动窗口来设置各个动作是否可用
	bool hasMdiChild=(activeMdiChild()!=0);
	ui.action_Save->setEnabled(hasMdiChild);    //设置各个动作是否可用
	ui.action_SaveAs->setEnabled(hasMdiChild);
	ui.action_Paste->setEnabled(hasMdiChild);
	ui.action_Close->setEnabled(hasMdiChild);
	ui.action_CloseAll->setEnabled(hasMdiChild);
	ui.action_Tile->setEnabled(hasMdiChild);
	ui.action_Cascade->setEnabled(hasMdiChild);
	ui.action_Next->setEnabled(hasMdiChild);
	ui.action_Previous->setEnabled(hasMdiChild);

	actionSeparator->setVisible(hasMdiChild);   //设置间隔器是否显示
	bool hasSelection = (activeMdiChild()
		&& activeMdiChild()->textCursor().hasSelection());

	// 有活动窗口且有被选择的文本，剪切复制才可用
	ui.action_Cut->setEnabled(hasSelection);
	ui.action_Copy->setEnabled(hasSelection);

	// 有活动窗口且文档有撤销操作
	ui.action_Undo->setEnabled(activeMdiChild()
		&& activeMdiChild()->document()->isUndoAvailable());

	// 有活动窗口且文档有恢复操作
	ui.action_Redo->setEnabled(activeMdiChild()
		&& activeMdiChild()->document()->isRedoAvailable());
}

MdiChild * MainWindow::activeMdiChild()
{
	if(QMdiSubWindow * activeSubWindow=ui.mdiArea->activeSubWindow())
		return qobject_cast<MdiChild *>(activeSubWindow->widget());
	return 0;
}

MdiChild * MainWindow::createMdiChild()
{
	MdiChild * child=new MdiChild;
	ui.mdiArea->addSubWindow(child);
	connect(child,SIGNAL(copyAvailable(bool)),ui.action_Cut,SLOT(setEnabled(bool)));
	connect(child,SIGNAL(copyAvailable(bool)),ui.action_Copy,SLOT(setEnabled(bool)));
	connect(child->document(),SIGNAL(undoAvailable(bool)),ui.action_Undo,SLOT(setEnabled(bool)));
	connect(child->document(),SIGNAL(redoAvailable(bool)),ui.action_Redo,SLOT(setEnabled(bool)));
	connect(child,SIGNAL(cursorPositionChanged()),this,SLOT(showTextRowAndCol()));
	return child;
}

//新建文件槽
void MainWindow::on_action_New_triggered()
{
	MdiChild *child=createMdiChild();
	child->newFile();
	child->show();
}

//打开文件槽
void MainWindow::on_action_Open_triggered()
{
	QString fileName=QFileDialog::getOpenFileName(this);
	if(!fileName.isEmpty())
	{
		QMdiSubWindow *existing=findMdiChild(fileName);
		if (existing)
		{
			ui.mdiArea->setActiveSubWindow(existing);
			return;
		}
		MdiChild *child=createMdiChild();
		if (child->loadFile(fileName))
		{
			ui.statusBar->showMessage(tr("打开文件成功"),2000);
			child->show();
		}
		else
			child->close();
	}
}

QMdiSubWindow *MainWindow::findMdiChild(const QString &fileName)
{
	QString canonicalFilePath=QFileInfo(fileName).canonicalFilePath();
	foreach(QMdiSubWindow *window,ui.mdiArea->subWindowList())
	{
		MdiChild *mdiChild=qobject_cast<MdiChild *>(window->widget());
		if(mdiChild->currentFile()==canonicalFilePath)
			return window;
	}
	return 0;
}

//活动窗口的实现
void MainWindow::setActiveSubWindow(QWidget *window)
{
	if(!window)
		return;
	ui.mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

void MainWindow::updateWindowMenu()
{
	ui.menuW->clear();
	ui.menuW->addAction(ui.action_Close);
	ui.menuW->addAction(ui.action_CloseAll);
	ui.menuW->addSeparator();
	ui.menuW->addAction(ui.action_Tile);
	ui.menuW->addAction(ui.action_Cascade);
	ui.menuW->addSeparator();
	ui.menuW->addAction(ui.action_Next);
	ui.menuW->addAction(ui.action_Previous);
	ui.menuW->addAction(actionSeparator);

	//如果有活动窗口则显示间隔器
	QList<QMdiSubWindow *>windows =ui.mdiArea->subWindowList();
	actionSeparator->setVisible(!windows.isEmpty());

	//遍历各个子窗口
	for(int i=0;i<windows.size();++i)
	{
		MdiChild *child=qobject_cast<MdiChild *>(windows.at(i)->widget());
		QString text;
		//如果窗口数小于9，则设置编号为快捷键
		if (i<9)
		{
			text=tr("&%1 %2").arg(i+1).arg(child->userFriendlyCurrentFile());
		}
		else
		{
			text=tr("%1 %2").arg(i+1).arg(child->userFriendlyCurrentFile());
		}
	

		//添加动作到菜单
		QAction *action=ui.menuW->addAction(text);
		action->setCheckable(true);

		//设置当前活动窗口动作为选中状态
		action->setChecked(child==activeMdiChild());
		//关联动作的触发信号到信号映射器map()槽，这个槽发射maped()信号
		connect(action,SIGNAL(triggered()),windowMapper,SLOT(map()));
		windowMapper->setMapping(action,windows.at(i));

	}


}

void MainWindow::on_action_Save_triggered()
{
	if(activeMdiChild()&&activeMdiChild()->save())
		ui.statusBar->showMessage(tr("文件保存成功"),2000);
}
//void MainWindow::on_action_SaveAs_triggered()
//{
//	if(activeMdiChild()&&activeMdiChild()->saveAs())
//		ui.statusBar->showMessage(tr("文件另存为成功"),2000);
//}

void MainWindow::on_action_Exit_triggered()
{
	qApp->closeAllWindows();
}

void MainWindow::on_action_Undo_triggered()
{
	if(activeMdiChild())
		activeMdiChild()->undo();
}

void MainWindow::on_action_Redo_triggered()
{
	if(activeMdiChild())
		activeMdiChild()->redo();
}

void MainWindow::on_action_Copy_triggered()
{
	if(activeMdiChild())
		activeMdiChild()->copy();
}

void MainWindow::on_action_Cut_triggered()
{
	if(activeMdiChild())
		activeMdiChild()->cut();
}

void MainWindow::on_action_Paste_triggered()
{
	if(activeMdiChild())
		activeMdiChild()->paste();
}

void MainWindow::on_action_Close_triggered()
{
	ui.mdiArea->closeActiveSubWindow();
}

void MainWindow::on_action_CloseAll_triggered()
{
	ui.mdiArea->closeAllSubWindows();
}

void MainWindow::on_action_Tile_triggered()
{
	ui.mdiArea->tileSubWindows();
}

void MainWindow::on_action_Cascade_triggered()
{
	ui.mdiArea->cascadeSubWindows();
}

void MainWindow::on_action_Next_triggered()
{
	ui.mdiArea->activateNextSubWindow();
}

void MainWindow::on_action_Previous_triggered()
{
	ui.mdiArea->activatePreviousSubWindow();
}

void MainWindow::on_action_About_triggered()
{
	QMessageBox::about(this,tr("关于"),tr("张超制作"));
}

void MainWindow::on_action_AboutQt_triggered()
{
	qApp->aboutQt();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	ui.mdiArea->closeAllSubWindows();
	if(ui.mdiArea->currentSubWindow())
		event->ignore();
	else
	{
		writeSettings();
		event->accept();
	}
}


//写入窗口设置
void MainWindow::writeSettings()
{
	QSettings settings("zhangchao","myMdi");
	settings.setValue("pos",pos());
	settings.setValue("size",size());
}

//读取窗口设置
void MainWindow::readSettings()
{
	QSettings settings("zhangchao","myMdi");
	QPoint pos=settings.value("pos",QPoint(200,200)).toPoint();
	QSize size=settings.value("size",QSize(800,600)).toSize();
	move(pos);
	resize(size);
}

void MainWindow::showTextRowAndCol()
{
	if (activeMdiChild())
	{
		int rowNum=activeMdiChild()->textCursor().blockNumber()+1;
		int colNum=activeMdiChild()->textCursor().columnNumber()+1;
		ui.statusBar->showMessage(tr("%1 行，%2 列").arg(rowNum).arg(colNum),5000);
	}
}

void MainWindow::initWindow()
{
	setWindowTitle(tr("多文档编辑器__ZC"));
	//工具栏右击时，可以关闭工具栏
	ui.mainToolBar->setWindowTitle(tr("工具栏"));
	//当区域的内容超出可视区域后，出现滚动条
	ui.mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui.mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	ui.statusBar->showMessage(tr("欢迎使用多文档编辑器"));
	QLabel *label=new QLabel(this);
	label->setFrameStyle(QFrame::Box|QFrame::Sunken);
	label->setText(tr("张超"));
	label->setTextFormat(Qt::RichText);
	ui.statusBar->addPermanentWidget(label);
	ui.action_New->setStatusTip(tr("创建一个文件"));

}