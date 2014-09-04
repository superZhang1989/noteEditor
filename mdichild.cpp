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
	//�������Ӵ��ڹر�ʱ���������Ķ���
	setAttribute(Qt::WA_DeleteOnClose);
	isUntitled=true;//��ʼ��
}

MdiChild::~MdiChild()
{

}

//�½��ļ�����
void MdiChild::newFile()
{
	//���ô��ڱ��
	static int sequenceNumber=1;
	//�½��ĵ�û�б������
	isUntitled=true;
	//����ǰ�ļ�����Ϊδ�����ĵ��ӱ��
	curFile=tr("δ�����ĵ�%1.txt").arg(sequenceNumber++);
	//���ô��ڱ��⣬ʹ��[*]�������ĵ������ĺ����ļ����ƺ���ʾ*��
	setWindowTitle(curFile+"[*]"+tr("-���ĵ��༭��"));
	//�ĵ�����ʱ�������ź�
	connect(document(),SIGNAL(contentsChanged()),this,SLOT(documentWasModified()));
}

//documentWasModified()�۵Ķ���
void MdiChild::documentWasModified()
{
	setWindowModified(document()->isModified());
}

//�����ļ�
bool MdiChild::loadFile(const QString &fileName)
{
	//�½�QFile����
	QFile file(fileName);
	//ֻ����ʽ���ļ�����������ʾ��������false
	if (!file.open(QFile::ReadOnly|QFile::Text))
	{
		QMessageBox::warning(this,tr("���ĵ��༭��"),tr("�޷���ȡ�ļ�%1:\n%2.").arg(fileName).arg(file.errorString()));
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

//����
bool MdiChild::save()
{
	if(isUntitled)
		return saveAs();
	else
		return saveFile(curFile);
}

//���Ϊ
bool MdiChild::saveAs()
{
	QString fileName=QFileDialog::getSaveFileName(this,tr("���Ϊ"),curFile);//�������Ϊ�Ի���	
	if(fileName.isEmpty())
		return false;
	return saveFile(fileName);
}

bool MdiChild::saveFile(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QFile::WriteOnly|QFile::Text))
	{
		QMessageBox::warning(this,tr("���ĵ��༭��"),tr("�޷�д���ļ�%1:\n%2.").arg(fileName).arg(file.errorString()));
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
		box.setWindowTitle(tr("���ĵ��༭��"));
		box.setText(tr("�Ƿ񱣴�ԡ�%1���ĸ���").arg(userFriendlyCurrentFile()));
		box.setIcon(QMessageBox::Warning);
		QPushButton *yesBtn=box.addButton(tr("��(&Y)"),QMessageBox::YesRole);
		box.addButton(tr("��(&N)"),QMessageBox::NoRole);
		QPushButton *cancelBtn=box.addButton(tr("ȡ��"),QMessageBox::RejectRole);

		box.exec();

		if(box.clickedButton()==yesBtn)
			return save();
		else if(box.clickedButton()==cancelBtn)
			return false;
	}
	return true;
}