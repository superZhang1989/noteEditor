#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include "ui_mainwindow.h"
class MdiChild;
class QMdiSubWindow;
class QSignalMapper;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~MainWindow();

private slots:
	void updateMenus();//���²˵�
	MdiChild * createMdiChild();//�����Ӵ���
	void setActiveSubWindow(QWidget *window);//���û�Ӵ���
	void updateWindowMenu();//���´��ڲ˵�

	void on_action_New_triggered();//�½��ļ�
	void on_action_Open_triggered();//���ļ�
	void on_action_Save_triggered();//����
	//void on_action_SaveAs_triggered()��//���Ϊ
	void on_action_Exit_triggered();//�˳�

	void on_action_Undo_triggered();//����
	void on_action_Redo_triggered();//�ָ�
	void on_action_Copy_triggered();//����
	void on_action_Cut_triggered();//����
	void on_action_Paste_triggered();//ճ��

	void on_action_Close_triggered();//�ر�
	void on_action_CloseAll_triggered();
	void on_action_Tile_triggered();//ƽ��
	void on_action_Cascade_triggered();//���
	void on_action_Next_triggered();//��һ��
	void on_action_Previous_triggered();//��һ��

	void on_action_About_triggered();
	void on_action_AboutQt_triggered();

	void showTextRowAndCol();//��ʾ�ı����к���

private:
	Ui::MainWindowClass ui;
	QAction *actionSeparator;//�����
	MdiChild *activeMdiChild();//�����
	QMdiSubWindow *findMdiChild(const QString &fileName);//�����Ӵ���
	QSignalMapper *windowMapper;//�ź�ӳ���� 
	void readSettings();//��ȡ��������
	void writeSettings();//д�봰������
	void initWindow();//��ʼ������

protected:
	void closeEvent(QCloseEvent *event);//�ر��¼�
};

#endif // MAINWINDOW_H
