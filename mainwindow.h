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
	void updateMenus();//更新菜单
	MdiChild * createMdiChild();//创建子窗口
	void setActiveSubWindow(QWidget *window);//设置活动子窗口
	void updateWindowMenu();//更新窗口菜单

	void on_action_New_triggered();//新建文件
	void on_action_Open_triggered();//打开文件
	void on_action_Save_triggered();//保存
	//void on_action_SaveAs_triggered()；//另存为
	void on_action_Exit_triggered();//退出

	void on_action_Undo_triggered();//撤销
	void on_action_Redo_triggered();//恢复
	void on_action_Copy_triggered();//复制
	void on_action_Cut_triggered();//剪切
	void on_action_Paste_triggered();//粘贴

	void on_action_Close_triggered();//关闭
	void on_action_CloseAll_triggered();
	void on_action_Tile_triggered();//平铺
	void on_action_Cascade_triggered();//层叠
	void on_action_Next_triggered();//下一个
	void on_action_Previous_triggered();//上一个

	void on_action_About_triggered();
	void on_action_AboutQt_triggered();

	void showTextRowAndCol();//显示文本的行和列

private:
	Ui::MainWindowClass ui;
	QAction *actionSeparator;//间隔器
	MdiChild *activeMdiChild();//活动窗口
	QMdiSubWindow *findMdiChild(const QString &fileName);//查找子窗口
	QSignalMapper *windowMapper;//信号映射器 
	void readSettings();//读取窗口设置
	void writeSettings();//写入窗口设置
	void initWindow();//初始化窗口

protected:
	void closeEvent(QCloseEvent *event);//关闭事件
};

#endif // MAINWINDOW_H
