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
	//������������������������ü����
	actionSeparator=new QAction(this);
	actionSeparator->setSeparator(true);
	updateMenus();//���²˵�
	//���л����ʱ���²˵�
	connect(ui.mdiArea,SIGNAL(subWindowActivated(QMdiSubWindow *)),this,SLOT(updateMenus()));
	windowMapper=new QSignalMapper(this);//�����ź�ӳ����
	//ӳ�������·����źţ������ź����ô���
	connect(windowMapper,SIGNAL(mapped(QWidget*)),
		this,SLOT(setActiveSubWindow(QWidget*)));
	//���´��ڲ˵�
	updateWindowMenu();
	connect(ui.menuW,SIGNAL(aboutToShow()),this,SLOT(updateWindowMenu()));
	readSettings();
	initWindow();
}

MainWindow::~MainWindow()
{

}

//���²˵�״̬
void MainWindow::updateMenus()
{
	//�����Ƿ��л���������ø��������Ƿ����
	bool hasMdiChild=(activeMdiChild()!=0);
	ui.action_Save->setEnabled(hasMdiChild);    //���ø��������Ƿ����
	ui.action_SaveAs->setEnabled(hasMdiChild);
	ui.action_Paste->setEnabled(hasMdiChild);
	ui.action_Close->setEnabled(hasMdiChild);
	ui.action_CloseAll->setEnabled(hasMdiChild);
	ui.action_Tile->setEnabled(hasMdiChild);
	ui.action_Cascade->setEnabled(hasMdiChild);
	ui.action_Next->setEnabled(hasMdiChild);
	ui.action_Previous->setEnabled(hasMdiChild);

	actionSeparator->setVisible(hasMdiChild);   //���ü�����Ƿ���ʾ
	bool hasSelection = (activeMdiChild()
		&& activeMdiChild()->textCursor().hasSelection());

	// �л�������б�ѡ����ı������и��Ʋſ���
	ui.action_Cut->setEnabled(hasSelection);
	ui.action_Copy->setEnabled(hasSelection);

	// �л�������ĵ��г�������
	ui.action_Undo->setEnabled(activeMdiChild()
		&& activeMdiChild()->document()->isUndoAvailable());

	// �л�������ĵ��лָ�����
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

//�½��ļ���
void MainWindow::on_action_New_triggered()
{
	MdiChild *child=createMdiChild();
	child->newFile();
	child->show();
}

//���ļ���
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
			ui.statusBar->showMessage(tr("���ļ��ɹ�"),2000);
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

//����ڵ�ʵ��
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

	//����л��������ʾ�����
	QList<QMdiSubWindow *>windows =ui.mdiArea->subWindowList();
	actionSeparator->setVisible(!windows.isEmpty());

	//���������Ӵ���
	for(int i=0;i<windows.size();++i)
	{
		MdiChild *child=qobject_cast<MdiChild *>(windows.at(i)->widget());
		QString text;
		//���������С��9�������ñ��Ϊ��ݼ�
		if (i<9)
		{
			text=tr("&%1 %2").arg(i+1).arg(child->userFriendlyCurrentFile());
		}
		else
		{
			text=tr("%1 %2").arg(i+1).arg(child->userFriendlyCurrentFile());
		}
	

		//��Ӷ������˵�
		QAction *action=ui.menuW->addAction(text);
		action->setCheckable(true);

		//���õ�ǰ����ڶ���Ϊѡ��״̬
		action->setChecked(child==activeMdiChild());
		//���������Ĵ����źŵ��ź�ӳ����map()�ۣ�����۷���maped()�ź�
		connect(action,SIGNAL(triggered()),windowMapper,SLOT(map()));
		windowMapper->setMapping(action,windows.at(i));

	}


}

void MainWindow::on_action_Save_triggered()
{
	if(activeMdiChild()&&activeMdiChild()->save())
		ui.statusBar->showMessage(tr("�ļ�����ɹ�"),2000);
}
//void MainWindow::on_action_SaveAs_triggered()
//{
//	if(activeMdiChild()&&activeMdiChild()->saveAs())
//		ui.statusBar->showMessage(tr("�ļ����Ϊ�ɹ�"),2000);
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
	QMessageBox::about(this,tr("����"),tr("�ų�����"));
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


//д�봰������
void MainWindow::writeSettings()
{
	QSettings settings("zhangchao","myMdi");
	settings.setValue("pos",pos());
	settings.setValue("size",size());
}

//��ȡ��������
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
		ui.statusBar->showMessage(tr("%1 �У�%2 ��").arg(rowNum).arg(colNum),5000);
	}
}

void MainWindow::initWindow()
{
	setWindowTitle(tr("���ĵ��༭��__ZC"));
	//�������һ�ʱ�����Թرչ�����
	ui.mainToolBar->setWindowTitle(tr("������"));
	//����������ݳ�����������󣬳��ֹ�����
	ui.mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui.mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	ui.statusBar->showMessage(tr("��ӭʹ�ö��ĵ��༭��"));
	QLabel *label=new QLabel(this);
	label->setFrameStyle(QFrame::Box|QFrame::Sunken);
	label->setText(tr("�ų�"));
	label->setTextFormat(Qt::RichText);
	ui.statusBar->addPermanentWidget(label);
	ui.action_New->setStatusTip(tr("����һ���ļ�"));

}