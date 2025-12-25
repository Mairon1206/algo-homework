#include "widget.h"
#include <QMessageBox>
#include <QHeaderView>
#include <sstream>
#include <QMouseEvent> 

Widget::Widget(QWidget* parent)
    : QWidget(parent) {
    
    // 初始化数据库
    database = new Database("lab_schedule.db");
    if (!database->initialize()) {
        QMessageBox::critical(this, "错误", "数据库初始化失败!");
        return;
    }
    
    // 初始化调度器
    scheduler = new Scheduler(database);
    
    // 设置UI
    setupUI();
    
    // 刷新表格
    refreshLabTable();
    refreshRequestTable();
    
    setWindowTitle("实验室安排系统");
    resize(1000, 700);
}

Widget::~Widget() {
    delete scheduler;
    delete database;
}

void Widget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);
    
    setupLabTab();
    setupRequestTab();
    setupScheduleTab();
    setupQueryTab();
}

void Widget::setupLabTab() {
    labTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(labTab);
    
    // 输入区域
    QGroupBox* inputGroup = new QGroupBox("实验室信息录入");
    QGridLayout* inputLayout = new QGridLayout(inputGroup);
    
    inputLayout->addWidget(new QLabel("实验室地址:"), 0, 0);
    labLocationEdit = new QLineEdit();
    labLocationEdit->setPlaceholderText("例如: 实验楼A301");
    inputLayout->addWidget(labLocationEdit, 0, 1);
    
    inputLayout->addWidget(new QLabel("容纳人数:"), 1, 0);
    labCapacitySpinBox = new QSpinBox();
    labCapacitySpinBox->setRange(1, 200);
    labCapacitySpinBox->setValue(40);
    inputLayout->addWidget(labCapacitySpinBox, 1, 1);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    addLabButton = new QPushButton("添加实验室");
    deleteLabButton = new QPushButton("删除选中实验室");
    buttonLayout->addWidget(addLabButton);
    buttonLayout->addWidget(deleteLabButton);
    buttonLayout->addStretch();
    inputLayout->addLayout(buttonLayout, 2, 0, 1, 2);
    
    layout->addWidget(inputGroup);
    
    // 表格区域
    labTable = new QTableWidget();
    labTable->setColumnCount(3);
    labTable->setHorizontalHeaderLabels({"ID", "实验室地址", "容纳人数"});
    labTable->horizontalHeader()->setStretchLastSection(true);
    labTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    labTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(labTable);
    
    // 连接信号
    connect(addLabButton, &QPushButton::clicked, this, &Widget::addLaboratory);
    connect(deleteLabButton, &QPushButton::clicked, this, &Widget::deleteLaboratory);
    
    tabWidget->addTab(labTab, "实验室管理");
}

void Widget::setupRequestTab() {
    requestTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(requestTab);
    
    // 基本信息输入
    QGroupBox* basicGroup = new QGroupBox("基本信息");
    QGridLayout* basicLayout = new QGridLayout(basicGroup);
    
    basicLayout->addWidget(new QLabel("班级ID:"), 0, 0);
    classIdEdit = new QLineEdit();
    classIdEdit->setPlaceholderText("例如: B210307");
    basicLayout->addWidget(classIdEdit, 0, 1);
    
    basicLayout->addWidget(new QLabel("学生人数:"), 0, 2);
    studentCountSpinBox = new QSpinBox();
    studentCountSpinBox->setRange(1, 200);
    studentCountSpinBox->setValue(33);
    basicLayout->addWidget(studentCountSpinBox, 0, 3);
    
    basicLayout->addWidget(new QLabel("指导教师:"), 1, 0);
    teacherEdit = new QLineEdit();
    teacherEdit->setPlaceholderText("例如: 朱洁");
    basicLayout->addWidget(teacherEdit, 1, 1);
    
    basicLayout->addWidget(new QLabel("优先级:"), 1, 2);
    prioritySpinBox = new QSpinBox();
    prioritySpinBox->setRange(1, 100);
    prioritySpinBox->setValue(1);
    prioritySpinBox->setToolTip("数字越小优先级越高");
    basicLayout->addWidget(prioritySpinBox, 1, 3);
    
    layout->addWidget(basicGroup);
    
    // 时间段选择
    timeSlotGroup = new QGroupBox("时间段选择 (蓝色=期望, 红色=不可用)");
    QGridLayout* timeLayout = new QGridLayout(timeSlotGroup);
    
    QString days[] = {"周一", "周二", "周三", "周四", "周五"};
    QString periods[] = {"上午", "下午"};
    QString weeks[] = {"第9周", "第10周"};
    
    for (int w = 0; w < 2; w++) {
        timeLayout->addWidget(new QLabel("<b>" + weeks[w] + "</b>"), w * 6, 0);
        
        for (int p = 0; p < 2; p++) {
            timeLayout->addWidget(new QLabel(periods[p]), w * 6 + p * 3 + 1, 0);
            
            for (int d = 0; d < 5; d++) {
                if (w == 0 && p == 0 && d == 0) {
                    timeLayout->addWidget(new QLabel(days[d]), 0, d + 1);
                }
                
                timeSlotChecks[w][d][p] = new QCheckBox();
                timeSlotChecks[w][d][p]->setProperty("week", w);
                timeSlotChecks[w][d][p]->setProperty("day", d);
                timeSlotChecks[w][d][p]->setProperty("period", p);
                // 安装事件过滤器，让Widget能够捕获复选框的鼠标事件
                timeSlotChecks[w][d][p]->installEventFilter(this);
                connect(timeSlotChecks[w][d][p], &QCheckBox::stateChanged, 
                        this, &Widget::updateTimeSlotSelection);
                timeLayout->addWidget(timeSlotChecks[w][d][p], w * 6 + p * 3 + 1, d + 1);
            }
        }
    }
    
    // 添加说明标签
    QHBoxLayout* legendLayout = new QHBoxLayout();
    QLabel* preferredLabel = new QLabel("左键点击: 期望时间段");
    preferredLabel->setStyleSheet("QLabel { background-color: lightblue; padding: 5px; }");
    QLabel* excludedLabel = new QLabel("右键点击: 不可用时间段");
    excludedLabel->setStyleSheet("QLabel { background-color: lightcoral; padding: 5px; }");
    legendLayout->addWidget(preferredLabel);
    legendLayout->addWidget(excludedLabel);
    legendLayout->addStretch();
    timeLayout->addLayout(legendLayout, 12, 0, 1, 6);
    
    layout->addWidget(timeSlotGroup);
    
    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    addRequestButton = new QPushButton("添加申请");
    deleteRequestButton = new QPushButton("删除选中申请");
    buttonLayout->addWidget(addRequestButton);
    buttonLayout->addWidget(deleteRequestButton);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);
    
    // 表格
    requestTable = new QTableWidget();
    requestTable->setColumnCount(5);
    requestTable->setHorizontalHeaderLabels({"ID", "班级", "人数", "教师", "优先级"});
    requestTable->horizontalHeader()->setStretchLastSection(true);
    requestTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    requestTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(requestTable);
    
    connect(addRequestButton, &QPushButton::clicked, this, &Widget::addRequest);
    connect(deleteRequestButton, &QPushButton::clicked, this, &Widget::deleteRequest);
    
    tabWidget->addTab(requestTab, "申请管理");
}

void Widget::setupScheduleTab() {
    scheduleTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(scheduleTab);
    
    QLabel* infoLabel = new QLabel(
        "<h3>课表生成</h3>"
        "<p>点击下方按钮生成课程安排。算法将:</p>"
        "<ul>"
        "<li>按优先级顺序处理申请(数字越小优先级越高)</li>"
        "<li>优先满足教师期望的时间段</li>"
        "<li>确保实验室容量满足需求</li>"
        "<li>避免时间冲突</li>"
        "</ul>"
    );
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);
    
    generateButton = new QPushButton("生成课程安排");
    generateButton->setMinimumHeight(40);
    QFont buttonFont = generateButton->font();
    buttonFont.setPointSize(12);
    buttonFont.setBold(true);
    generateButton->setFont(buttonFont);
    layout->addWidget(generateButton);
    
    scheduleResultText = new QTextEdit();
    scheduleResultText->setReadOnly(true);
    layout->addWidget(scheduleResultText);
    
    connect(generateButton, &QPushButton::clicked, this, &Widget::generateSchedule);
    
    tabWidget->addTab(scheduleTab, "课表生成");
}

void Widget::setupQueryTab() {
    queryTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(queryTab);
    
    // 查询选项
    QGroupBox* queryGroup = new QGroupBox("查询选项");
    QGridLayout* queryLayout = new QGridLayout(queryGroup);
    
    queryLayout->addWidget(new QLabel("按实验室查询:"), 0, 0);
    queryLabCombo = new QComboBox();
    queryLayout->addWidget(queryLabCombo, 0, 1);
    queryLabButton = new QPushButton("查询");
    queryLayout->addWidget(queryLabButton, 0, 2);
    
    queryLayout->addWidget(new QLabel("按班级查询:"), 1, 0);
    queryClassEdit = new QLineEdit();
    queryClassEdit->setPlaceholderText("输入班级ID");
    queryLayout->addWidget(queryClassEdit, 1, 1);
    queryClassButton = new QPushButton("查询");
    queryLayout->addWidget(queryClassButton, 1, 2);
    
    layout->addWidget(queryGroup);
    
    // 结果表格
    queryResultTable = new QTableWidget();
    queryResultTable->setColumnCount(6);
    queryResultTable->setHorizontalHeaderLabels({
        "班级", "教师", "实验室", "周次", "星期", "时段"
    });
    queryResultTable->horizontalHeader()->setStretchLastSection(true);
    queryResultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(queryResultTable);
    
    connect(queryLabButton, &QPushButton::clicked, this, &Widget::queryByLab);
    connect(queryClassButton, &QPushButton::clicked, this, &Widget::queryByClass);
    
    tabWidget->addTab(queryTab, "课表查询");
}

// 实验室管理实现
void Widget::addLaboratory() {
    QString location = labLocationEdit->text().trimmed();
    if (location.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入实验室地址!");
        return;
    }
    
    int capacity = labCapacitySpinBox->value();
    
    if (database->addLaboratory(location.toStdString(), capacity)) {
        QMessageBox::information(this, "成功", "实验室添加成功!");
        labLocationEdit->clear();
        refreshLabTable();
        
        // 更新查询下拉框
        queryLabCombo->clear();
        auto labs = database->getAllLaboratories();
        for (const auto& lab : labs) {
            queryLabCombo->addItem(
                QString::fromStdString(lab.location), 
                lab.id
            );
        }
    } else {
        QMessageBox::critical(this, "错误", "实验室添加失败!");
    }
}

void Widget::deleteLaboratory() {
    int row = labTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "警告", "请先选择要删除的实验室!");
        return;
    }
    
    int id = labTable->item(row, 0)->text().toInt();
    
    if (database->deleteLaboratory(id)) {
        QMessageBox::information(this, "成功", "实验室删除成功!");
        refreshLabTable();
    } else {
        QMessageBox::critical(this, "错误", "实验室删除失败!");
    }
}

void Widget::refreshLabTable() {
    auto labs = database->getAllLaboratories();
    labTable->setRowCount(labs.size());
    
    for (size_t i = 0; i < labs.size(); i++) {
        labTable->setItem(i, 0, new QTableWidgetItem(QString::number(labs[i].id)));
        labTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(labs[i].location)));
        labTable->setItem(i, 2, new QTableWidgetItem(QString::number(labs[i].capacity)));
    }
}

// 申请管理实现
void Widget::updateTimeSlotSelection() {
    QCheckBox* checkbox = qobject_cast<QCheckBox*>(sender());
    if (!checkbox) return;
    
    if (checkbox->checkState() == Qt::Checked) {
        checkbox->setStyleSheet("QCheckBox::indicator { background-color: lightblue; }");
    } else if (checkbox->checkState() == Qt::PartiallyChecked) {
        checkbox->setStyleSheet("QCheckBox::indicator { background-color: lightcoral; }");
    } else {
        checkbox->setStyleSheet("");
    }
}

void Widget::addRequest() {
    QString classId = classIdEdit->text().trimmed();
    QString teacher = teacherEdit->text().trimmed();
    
    if (classId.isEmpty() || teacher.isEmpty()) {
        QMessageBox::warning(this, "警告", "请填写班级ID和教师姓名!");
        return;
    }
    
    LabRequest request;
    request.classId = classId.toStdString();
    request.studentCount = studentCountSpinBox->value();
    request.teacher = teacher.toStdString();
    request.priority = prioritySpinBox->value();
    
    // 收集时间段选择
    for (int w = 0; w < 2; w++) {
        for (int d = 0; d < 5; d++) {
            for (int p = 0; p < 2; p++) {
                QCheckBox* cb = timeSlotChecks[w][d][p];
                TimeSlot slot = {w + 9, d, p};  // 周次从9开始
                
                if (cb->checkState() == Qt::Checked) {
                    request.preferredSlots.push_back(slot);
                } else if (cb->checkState() == Qt::PartiallyChecked) {
                    request.excludedSlots.push_back(slot);
                }
            }
        }
    }
    
    if (request.preferredSlots.empty()) {
        QMessageBox::warning(this, "警告", "请至少选择一个期望时间段!");
        return;
    }
    
    if (database->addRequest(request)) {
        QMessageBox::information(this, "成功", "申请添加成功!");
        classIdEdit->clear();
        teacherEdit->clear();
        
        // 清除复选框
        for (int w = 0; w < 2; w++) {
            for (int d = 0; d < 5; d++) {
                for (int p = 0; p < 2; p++) {
                    timeSlotChecks[w][d][p]->setCheckState(Qt::Unchecked);
                }
            }
        }
        
        refreshRequestTable();
    } else {
        QMessageBox::critical(this, "错误", "申请添加失败!");
    }
}

void Widget::deleteRequest() {
    int row = requestTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "警告", "请先选择要删除的申请!");
        return;
    }
    
    int id = requestTable->item(row, 0)->text().toInt();
    
    if (database->deleteRequest(id)) {
        QMessageBox::information(this, "成功", "申请删除成功!");
        refreshRequestTable();
    } else {
        QMessageBox::critical(this, "错误", "申请删除失败!");
    }
}

void Widget::refreshRequestTable() {
    auto requests = database->getAllRequests();
    requestTable->setRowCount(requests.size());
    
    for (size_t i = 0; i < requests.size(); i++) {
        requestTable->setItem(i, 0, new QTableWidgetItem(QString::number(requests[i].id)));
        requestTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(requests[i].classId)));
        requestTable->setItem(i, 2, new QTableWidgetItem(QString::number(requests[i].studentCount)));
        requestTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(requests[i].teacher)));
        requestTable->setItem(i, 4, new QTableWidgetItem(QString::number(requests[i].priority)));
    }
}

// 课表生成实现
void Widget::generateSchedule() {
    auto labs = database->getAllLaboratories();
    auto requests = database->getAllRequests();
    
    if (labs.empty()) {
        QMessageBox::warning(this, "警告", "请先添加实验室!");
        return;
    }
    
    if (requests.empty()) {
        QMessageBox::warning(this, "警告", "请先添加申请!");
        return;
    }
    
    scheduleResultText->clear();
    scheduleResultText->append("正在生成课程安排...\n");
    
    int successCount = scheduler->generateSchedule();
    
    auto stats = scheduler->getScheduleStats();
    
    scheduleResultText->append("\n========== 调度结果统计 ==========");
    scheduleResultText->append(QString("总申请数: %1").arg(stats.totalRequests));
    scheduleResultText->append(QString("成功分配: %1").arg(stats.successfulRequests));
    scheduleResultText->append(QString("失败数量: %1").arg(stats.failedRequests));
    scheduleResultText->append(QString("成功率: %1%").arg(stats.successRate, 0, 'f', 2));
    
    if (!stats.failedClasses.empty()) {
        scheduleResultText->append("\n未能分配的班级:");
        for (const auto& failedClass : stats.failedClasses) {
            scheduleResultText->append("  - " + QString::fromStdString(failedClass));
        }
    }
    
    scheduleResultText->append("\n课程安排已保存到数据库!");
    scheduleResultText->append("请前往\"课表查询\"页面查看详细安排。");
    
    if (successCount > 0) {
        QMessageBox::information(this, "成功", 
            QString("课程安排生成完成!\n成功分配: %1 / %2")
            .arg(successCount).arg(stats.totalRequests));
    }
}

// 课表查询实现
void Widget::queryByLab() {
    if (queryLabCombo->count() == 0) {
        QMessageBox::warning(this, "警告", "没有可查询的实验室!");
        return;
    }
    
    int labId = queryLabCombo->currentData().toInt();
    auto schedules = database->getSchedulesByLab(labId);
    
    queryResultTable->setRowCount(schedules.size());
    
    for (size_t i = 0; i < schedules.size(); i++) {
        auto request = database->getRequest(schedules[i].requestId);
        auto lab = database->getLaboratory(schedules[i].labId);
        
        queryResultTable->setItem(i, 0, new QTableWidgetItem(
            QString::fromStdString(request.classId)));
        queryResultTable->setItem(i, 1, new QTableWidgetItem(
            QString::fromStdString(request.teacher)));
        queryResultTable->setItem(i, 2, new QTableWidgetItem(
            QString::fromStdString(lab.location)));
        queryResultTable->setItem(i, 3, new QTableWidgetItem(
            QString("第%1周").arg(schedules[i].timeSlot.week)));
        queryResultTable->setItem(i, 4, new QTableWidgetItem(
            dayToString(schedules[i].timeSlot.day)));
        queryResultTable->setItem(i, 5, new QTableWidgetItem(
            periodToString(schedules[i].timeSlot.period)));
    }
}

void Widget::queryByClass() {
    QString classId = queryClassEdit->text().trimmed();
    if (classId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入班级ID!");
        return;
    }
    
    auto schedules = database->getSchedulesByClass(classId.toStdString());
    
    if (schedules.empty()) {
        QMessageBox::information(this, "提示", "未找到该班级的课程安排!");
        return;
    }
    
    queryResultTable->setRowCount(schedules.size());
    
    for (size_t i = 0; i < schedules.size(); i++) {
        auto request = database->getRequest(schedules[i].requestId);
        auto lab = database->getLaboratory(schedules[i].labId);
        
        queryResultTable->setItem(i, 0, new QTableWidgetItem(
            QString::fromStdString(request.classId)));
        queryResultTable->setItem(i, 1, new QTableWidgetItem(
            QString::fromStdString(request.teacher)));
        queryResultTable->setItem(i, 2, new QTableWidgetItem(
            QString::fromStdString(lab.location)));
        queryResultTable->setItem(i, 3, new QTableWidgetItem(
            QString("第%1周").arg(schedules[i].timeSlot.week)));
        queryResultTable->setItem(i, 4, new QTableWidgetItem(
            dayToString(schedules[i].timeSlot.day)));
        queryResultTable->setItem(i, 5, new QTableWidgetItem(
            periodToString(schedules[i].timeSlot.period)));
    }
}

bool Widget::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QCheckBox* checkbox = qobject_cast<QCheckBox*>(obj);
        // 确保是时间段复选框（通过检查是否有week属性）
        if (checkbox && checkbox->property("week").isValid()) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::RightButton) {
                // 右键点击逻辑：切换"不可用"状态 (PartiallyChecked)
                // 如果当前是不可用(红)，则取消；否则设为不可用
                if (checkbox->checkState() == Qt::PartiallyChecked) {
                    checkbox->setCheckState(Qt::Unchecked);
                } else {
                    checkbox->setCheckState(Qt::PartiallyChecked);
                }
                return true; // 拦截事件，阻止默认处理
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

// 辅助函数
QString Widget::timeSlotToString(const TimeSlot& slot) {
    return QString("第%1周 %2 %3")
        .arg(slot.week)
        .arg(dayToString(slot.day))
        .arg(periodToString(slot.period));
}

QString Widget::dayToString(int day) {
    const char* days[] = {"周一", "周二", "周三", "周四", "周五"};
    return QString::fromUtf8(days[day]);
}

QString Widget::periodToString(int period) {
    return period == 0 ? "上午(2-5节)" : "下午(6-9节)";
}
