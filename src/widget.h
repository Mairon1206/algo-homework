#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include "database.h"
#include "scheduler.h"

class Widget : public QWidget {
    Q_OBJECT

public:
    Widget(QWidget* parent = nullptr);
    ~Widget();

private slots:
    // 实验室管理
    void addLaboratory();
    void deleteLaboratory();
    void refreshLabTable();
    
    // 申请管理
    void addRequest();
    void deleteRequest();
    void refreshRequestTable();
    void updateTimeSlotSelection();
    
    // 课表生成
    void generateSchedule();
    
    // 课表查询
    void queryByLab();
    void queryByClass();
protected:
    // 添加事件过滤器声明
    bool eventFilter(QObject *obj, QEvent *event) override;
private:
    // 数据库和调度器
    Database* database;
    Scheduler* scheduler;
    
    // UI组件
    QTabWidget* tabWidget;
    
    // 实验室管理标签页
    QWidget* labTab;
    QLineEdit* labLocationEdit;
    QSpinBox* labCapacitySpinBox;
    QPushButton* addLabButton;
    QPushButton* deleteLabButton;
    QTableWidget* labTable;
    
    // 申请管理标签页
    QWidget* requestTab;
    QLineEdit* classIdEdit;
    QSpinBox* studentCountSpinBox;
    QLineEdit* teacherEdit;
    QSpinBox* prioritySpinBox;
    QGroupBox* timeSlotGroup;
    QCheckBox* timeSlotChecks[2][5][2];  // [周次][星期][时段]
    QPushButton* addRequestButton;
    QPushButton* deleteRequestButton;
    QTableWidget* requestTable;
    
    // 课表生成标签页
    QWidget* scheduleTab;
    QPushButton* generateButton;
    QTextEdit* scheduleResultText;
    
    // 课表查询标签页
    QWidget* queryTab;
    QComboBox* queryLabCombo;
    QLineEdit* queryClassEdit;
    QPushButton* queryLabButton;
    QPushButton* queryClassButton;
    QTableWidget* queryResultTable;
    
    // 初始化UI
    void setupUI();
    void setupLabTab();
    void setupRequestTab();
    void setupScheduleTab();
    void setupQueryTab();
    
    // 辅助函数
    QString timeSlotToString(const TimeSlot& slot);
    QString dayToString(int day);
    QString periodToString(int period);
};

#endif // WIDGET_H
