#include "mdichild.h"
#include <QTextStream>
#include <QMessageBox>
#include <QFileInfo>
#include <QApplication>
#include <QFileDialog>
#include <QCloseEvent>
#include <QPushButton>

MdiChild::MdiChild(QWidget *parent)
	: QTextEdit(parent)
{
	//设置在子窗口关闭时销毁这个类的对象
	setAttribute(Qt::WA_DeleteOnClose);
	isUntitled=true;//初始化
}

MdiChild::~MdiChild()
{

}

//新建文件操作
void MdiChild::newFile()
{
	//设置窗口编号
	static int sequenceNumber=1;
	//新建文档没有被保存过
	isUntitled=true;
	//将当前文件命名为未命名文档加编号
	curFile=tr("未命名文档%1.txt").arg(sequenceNumber++);
	//设置窗口标题，使用[*]可以在文档被更改后在文件名称后显示*号
	setWindowTitle(curFile+"[*]"+tr("-多文档编辑器"));
	//文档更改时，发射信号
	connect(document(),SIGNAL(contentsChanged()),this,SLOT(documentWasModified()));
}

//documentWasModified()槽的定义
void MdiChild::documentWasModified()
{
	setWindowModified(document()->isModified());
}

//加载文件
bool MdiChild::loadFile(const QString &fileName)
{
	//新建QFile对象
	QFile file(fileName);
	//只读方式打开文件，出错则提示，并返回false
	if (!file.open(QFile::ReadOnly|QFile::Text))
	{
		QMessageBox::warning(this,tr("多文档编辑器"),tr("无法读取文件%1:\n%2.").arg(fileName).arg(file.errorString()));
		return false;
	}

	QTextStream in(&file);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	setPlainText(in.readAll());
	QApplication::restoreOverrideCursor();
	setCurrentFile(fileName);
	connect(document(),SIGNAL(contentsChanged()),this,SLOT(documentWasModified()));
	return true;
}

void MdiChild::setCurrentFile(const QString &fileName)
{
	curFile=QFileInfo(fileName).canonicalFilePath();
	isUntitled=false;
	document()->setModified(false);
	setWindowModified(false);
	setWindowTitle(userFriendlyCurrentFile()+"[*]");
}

QString MdiChild::userFriendlyCurrentFile()
{
	return QFileInfo(curFile).fileName();
}

//保存
bool MdiChild::save()
{
	if(isUntitled)
		return saveAs();
	else
		return saveFile(curFile);
}

//另存为
bool MdiChild::saveAs()
{
	QString fileName=QFileDialog::getSaveFileName(this,tr("另存为"),curFile);//弹出另存为对话框	
	if(fileName.isEmpty())
		return false;
	return saveFile(fileName);
}

bool MdiChild::saveFile(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QFile::WriteOnly|QFile::Text))
	{
		QMessageBox::warning(this,tr("多文档编辑器"),tr("无法写入文件%1:\n%2.").arg(fileName).arg(file.errorString()));
		return false;
	}
	QTextStream out(&file);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	out<<toPlainText();
	QApplication::restoreOverrideCursor();
	setCurrentFile(fileName);
	return true;
}

void MdiChild::closeEvent(QCloseEvent *event)
{
	if(maybeSave())
		event->accept();
	else
		event->ignore();
}

bool MdiChild::maybeSave()
{
	if (document()->isModified())
	{
		QMessageBox box;
		box.setWindowTitle(tr("多文档编辑器"));
		box.setText(tr("是否保存对“%1”的更改").arg(userFriendlyCurrentFile()));
		box.setIcon(QMessageBox::Warning);
		QPushButton *yesBtn=box.addButton(tr("是(&Y)"),QMessageBox::YesRole);
		box.addButton(tr("否(&N)"),QMessageBox::NoRole);
		QPushButton *cancelBtn=box.addButton(tr("取消"),QMessageBox::RejectRole);

		box.exec();

		if(box.clickedButton()==yesBtn)
			return save();
		else if(box.clickedButton()==cancelBtn)
			return false;
	}
	return true;
}