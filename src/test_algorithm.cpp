#include "database.h"
#include "scheduler.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "=== 实验室安排系统 - 算法测试 ===" << std::endl;
    
    // 创建数据库
    Database db("test_lab_schedule.db");
    if (!db.initialize()) {
        std::cerr << "数据库初始化失败!" << std::endl;
        return 1;
    }
    
    // 清空旧数据
    db.clearAllData();
    
    // 1. 添加实验室
    std::cout << "\n[1] 添加实验室..." << std::endl;
    db.addLaboratory("实验楼A301", 40);
    db.addLaboratory("实验楼A302", 40);
    db.addLaboratory("实验楼B201", 50);
    
    auto labs = db.getAllLaboratories();
    std::cout << "已添加 " << labs.size() << " 个实验室" << std::endl;
    
    // 2. 添加申请(参考题目示例)
    std::cout << "\n[2] 添加申请..." << std::endl;
    
    // B210307 - 朱洁 (第9周)
    LabRequest req1;
    req1.classId = "B210307";
    req1.studentCount = 33;
    req1.teacher = "朱洁";
    req1.priority = 1;
    req1.preferredSlots = {
        {9, 1, 0}, {9, 2, 0}, {9, 3, 0}, {9, 4, 0}, {9, 5, 0}  // 第9周周一至周五上午
    };
    req1.excludedSlots = {
        {9, 1, 1}, {9, 3, 1}, {9, 4, 1}  // 第9周部分下午不可用
    };
    db.addRequest(req1);
    
    // B210308 - 胡惠娟 (第9周)
    LabRequest req2;
    req2.classId = "B210308";
    req2.studentCount = 36;
    req2.teacher = "胡惠娟";
    req2.priority = 2;
    req2.preferredSlots = {
        {9, 1, 0}, {9, 2, 0}  // 第9周周一、周二上午
    };
    req2.excludedSlots = {
        {9, 3, 0}, {9, 5, 0}, {9, 5, 1}  // 部分时间不可用
    };
    db.addRequest(req2);
    
    // B210309 - 戴华 (第9周)
    LabRequest req3;
    req3.classId = "B210309";
    req3.studentCount = 33;
    req3.teacher = "戴华";
    req3.priority = 3;
    req3.preferredSlots = {
        {9, 2, 0}, {9, 3, 0}  // 第9周周二、周三上午
    };
    req3.excludedSlots = {
        {9, 1, 0}, {9, 1, 1}, {9, 4, 1}
    };
    db.addRequest(req3);
    
    // B210310 - 徐鹤 (第9周)
    LabRequest req4;
    req4.classId = "B210310";
    req4.studentCount = 33;
    req4.teacher = "徐鹤";
    req4.priority = 4;
    req4.preferredSlots = {
        {9, 2, 0}, {9, 3, 0}, {9, 5, 0}  // 第9周部分上午
    };
    req4.excludedSlots = {
        {9, 1, 1}, {9, 2, 1}
    };
    db.addRequest(req4);
    
    auto requests = db.getAllRequests();
    std::cout << "已添加 " << requests.size() << " 个申请" << std::endl;
    
    // 3. 生成课表
    std::cout << "\n[3] 生成课表..." << std::endl;
    Scheduler scheduler(&db);
    int successCount = scheduler.generateSchedule();
    
    // 4. 显示统计
    std::cout << "\n[4] 调度统计:" << std::endl;
    auto stats = scheduler.getScheduleStats();
    std::cout << "总申请数: " << stats.totalRequests << std::endl;
    std::cout << "成功分配: " << stats.successfulRequests << std::endl;
    std::cout << "失败数量: " << stats.failedRequests << std::endl;
    std::cout << "成功率: " << stats.successRate << "%" << std::endl;
    
    if (!stats.failedClasses.empty()) {
        std::cout << "\n未能分配的班级:" << std::endl;
        for (const auto& failedClass : stats.failedClasses) {
            std::cout << "  - " << failedClass << std::endl;
        }
    }
    
    // 5. 查询课表
    std::cout << "\n[5] 查询课表:" << std::endl;
    auto schedules = db.getAllSchedules();
    
    const char* days[] = {"周一", "周二", "周三", "周四", "周五"};
    const char* periods[] = {"上午(2-5节)", "下午(6-9节)"};
    
    std::cout << "\n完整课表:" << std::endl;
    std::cout << "------------------------------------------------------" << std::endl;
    for (const auto& sch : schedules) {
        auto req = db.getRequest(sch.requestId);
        auto lab = db.getLaboratory(sch.labId);
        
        std::cout << "班级: " << req.classId 
                  << " | 教师: " << req.teacher
                  << " | 实验室: " << lab.location
                  << " | 第" << sch.timeSlot.week << "周 "
                  << days[sch.timeSlot.day - 1] << " "
                  << periods[sch.timeSlot.period] << std::endl;
    }
    std::cout << "------------------------------------------------------" << std::endl;
    
    // 6. 按班级查询
    std::cout << "\n[6] 查询B210307班级的课表:" << std::endl;
    auto classSchedules = db.getSchedulesByClass("B210307");
    for (const auto& sch : classSchedules) {
        auto lab = db.getLaboratory(sch.labId);
        std::cout << "  第" << sch.timeSlot.week << "周 "
                  << days[sch.timeSlot.day - 1] << " "
                  << periods[sch.timeSlot.period]
                  << " - " << lab.location << std::endl;
    }
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
    return 0;
}
