#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>

// 实验室信息
struct Laboratory {
    int id;
    std::string location;
    int capacity;
};

// 时间槽定义 (周次, 星期, 时段: 0-上午, 1-下午)
struct TimeSlot {
    int week;      // 周次 (9 或 10)
    int day;       // 星期 (1-5 对应周一到周五)
    int period;    // 时段 (0-上午, 1-下午)
    
    bool operator==(const TimeSlot& other) const {
        return week == other.week && day == other.day && period == other.period;
    }
    
    bool operator<(const TimeSlot& other) const {
        if (week != other.week) return week < other.week;
        if (day != other.day) return day < other.day;
        return period < other.period;
    }
};

// 实验申请
struct LabRequest {
    int id;
    std::string classId;
    int studentCount;
    std::string teacher;
    std::vector<TimeSlot> preferredSlots;  // 期望时间段 (√)
    std::vector<TimeSlot> excludedSlots;   // 不期望时间段 (×)
    int priority;  // 优先级 (基于申请时间)
};

// 课程安排结果
struct Schedule {
    int id;
    int requestId;
    int labId;
    TimeSlot timeSlot;
};

class Database {
public:
    Database(const std::string& dbPath);
    ~Database();
    
    bool initialize();
    bool isOpen() const { return db != nullptr; }
    
    // 实验室管理
    bool addLaboratory(const std::string& location, int capacity);
    bool deleteLaboratory(int id);
    std::vector<Laboratory> getAllLaboratories();
    Laboratory getLaboratory(int id);
    
    // 申请管理
    bool addRequest(const LabRequest& request);
    bool deleteRequest(int id);
    std::vector<LabRequest> getAllRequests();
    LabRequest getRequest(int id);
    
    // 课程安排管理
    bool clearSchedules();
    bool addSchedule(const Schedule& schedule);
    std::vector<Schedule> getAllSchedules();
    std::vector<Schedule> getSchedulesByLab(int labId);
    std::vector<Schedule> getSchedulesByClass(const std::string& classId);
    
    // 清空所有数据
    bool clearAllData();
    
private:
    sqlite3* db;
    std::string dbPath;
    
    bool executeSQL(const std::string& sql);
    std::string serializeTimeSlots(const std::vector<TimeSlot>& slots);
    std::vector<TimeSlot> deserializeTimeSlots(const std::string& data);
};

#endif // DATABASE_H
