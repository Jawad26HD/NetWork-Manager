#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <sqlite3.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <iphlpapi.h>
    #include <stdio.h>
    #pragma comment(lib, "iphlpapi.lib")
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <ifaddrs.h>
    #include <net/if.h>
#endif

using namespace std;

// هيكل بيانات لمعلومات الجهاز
struct Device {
    string mac;
    string ip;
    string hostname;
    long long downloaded;
    long long uploaded;
    time_t lastSeen;
    long long limit;
    string limitType;
    bool alertEnabled;
};

// فئة قاعدة البيانات
class Database {
private:
    sqlite3* db;
    mutex dbMutex;

public:
    Database(const string& filename) {
        int rc = sqlite3_open(filename.c_str(), &db);
        if (rc) {
            cerr << "❌ خطأ في فتح قاعدة البيانات: " << sqlite3_errmsg(db) << endl;
            exit(1);
        }
        initializeTables();
    }

    ~Database() {
        if (db) {
            sqlite3_close(db);
        }
    }

    void initializeTables() {
        const char* sql = R"(
            CREATE TABLE IF NOT EXISTS devices (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                mac TEXT UNIQUE NOT NULL,
                ip TEXT,
                hostname TEXT,
                first_seen DATETIME DEFAULT CURRENT_TIMESTAMP,
                last_seen DATETIME DEFAULT CURRENT_TIMESTAMP
            );

            CREATE TABLE IF NOT EXISTS statistics (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                device_id INTEGER NOT NULL,
                download_bytes INTEGER DEFAULT 0,
                upload_bytes INTEGER DEFAULT 0,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY(device_id) REFERENCES devices(id) ON DELETE CASCADE
            );

            CREATE TABLE IF NOT EXISTS limits (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                device_id INTEGER NOT NULL UNIQUE,
                limit_bytes INTEGER NOT NULL,
                limit_type TEXT CHECK(limit_type IN ('daily', 'monthly')),
                alert_enabled BOOLEAN DEFAULT 1,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            );

            CREATE TABLE IF NOT EXISTS alerts (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                device_id INTEGER NOT NULL,
                alert_type TEXT NOT NULL,
                message TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY(device_id) REFERENCES devices(id) ON DELETE CASCADE
            );

            CREATE INDEX IF NOT EXISTS idx_devices_mac ON devices(mac);
            CREATE INDEX IF NOT EXISTS idx_devices_ip ON devices(ip);
            CREATE INDEX IF NOT EXISTS idx_statistics_device_id ON statistics(device_id);
            CREATE INDEX IF NOT EXISTS idx_statistics_timestamp ON statistics(timestamp);
        )";

        char* errMsg = nullptr;
        int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        
        if (rc != SQLITE_OK) {
            cerr << "❌ خطأ في إنشاء الجداول: " << errMsg << endl;
            sqlite3_free(errMsg);
        } else {
            cout << "✅ جداول قاعدة البيانات جاهزة" << endl;
        }
    }

    void saveDeviceStats(const string& mac, const string& ip, long long download, long long upload) {
        lock_guard<mutex> lock(dbMutex);
        
        sqlite3_stmt* stmt;
        const char* sql = R"(
            INSERT OR REPLACE INTO devices (mac, ip, last_seen) VALUES (?, ?, CURRENT_TIMESTAMP);
            INSERT INTO statistics (device_id, download_bytes, upload_bytes) 
            SELECT id, ?, ? FROM devices WHERE mac = ?;
        )";

        // أولاً: إدراج أو تحديث الجهاز
        const char* insertDevice = "INSERT OR REPLACE INTO devices (mac, ip, last_seen) VALUES (?, ?, CURRENT_TIMESTAMP)";
        int rc = sqlite3_prepare_v2(db, insertDevice, -1, &stmt, nullptr);
        
        if (rc == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, mac.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, ip.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }

        // ثانياً: إدراج الإحصائيات
        const char* insertStats = "INSERT INTO statistics (device_id, download_bytes, upload_bytes) SELECT id, ?, ? FROM devices WHERE mac = ?";
        rc = sqlite3_prepare_v2(db, insertStats, -1, &stmt, nullptr);
        
        if (rc == SQLITE_OK) {
            sqlite3_bind_int64(stmt, 1, download);
            sqlite3_bind_int64(stmt, 2, upload);
            sqlite3_bind_text(stmt, 3, mac.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }

    vector<Device> getAllDevices() {
        lock_guard<mutex> lock(dbMutex);
        vector<Device> devices;

        const char* sql = R"(
            SELECT d.mac, d.ip, d.hostname, 
                   COALESCE(SUM(s.download_bytes), 0),
                   COALESCE(SUM(s.upload_bytes), 0),
                   COALESCE(l.limit_bytes, 0),
                   COALESCE(l.limit_type, ''),
                   COALESCE(l.alert_enabled, 0)
            FROM devices d
            LEFT JOIN statistics s ON d.id = s.device_id
            LEFT JOIN limits l ON d.id = l.device_id
            GROUP BY d.id
            ORDER BY d.last_seen DESC
        )";

        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

        if (rc == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                Device device;
                device.mac = (const char*)sqlite3_column_text(stmt, 0);
                device.ip = (const char*)sqlite3_column_text(stmt, 1);
                device.hostname = (const char*)sqlite3_column_text(stmt, 2);
                device.downloaded = sqlite3_column_int64(stmt, 3);
                device.uploaded = sqlite3_column_int64(stmt, 4);
                device.limit = sqlite3_column_int64(stmt, 5);
                device.limitType = (const char*)sqlite3_column_text(stmt, 6);
                device.alertEnabled = sqlite3_column_int(stmt, 7);
                device.lastSeen = time(nullptr);

                devices.push_back(device);
            }
            sqlite3_finalize(stmt);
        }

        return devices;
    }

    void setDeviceLimit(const string& mac, long long limit, const string& limitType) {
        lock_guard<mutex> lock(dbMutex);

        sqlite3_stmt* stmt;
        const char* sql = R"(
            INSERT OR REPLACE INTO limits (device_id, limit_bytes, limit_type, alert_enabled)
            SELECT id, ?, ?, 1 FROM devices WHERE mac = ?
        )";

        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc == SQLITE_OK) {
            sqlite3_bind_int64(stmt, 1, limit);
            sqlite3_bind_text(stmt, 2, limitType.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, mac.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }

    void logAlert(const string& mac, const string& alertType, const string& message) {
        lock_guard<mutex> lock(dbMutex);

        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO alerts (device_id, alert_type, message) SELECT id, ?, ? FROM devices WHERE mac = ?";

        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, alertType.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, message.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, mac.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }
};

// فئة مراقب الشبكة
class NetworkMonitor {
private:
    Database& db;
    map<string, Device> devices;
    mutex devicesMutex;
    bool running;

public:
    NetworkMonitor(Database& database) : db(database), running(true) {}

    void start() {
        cout << "🌐 بدء مراقبة الشبكة..." << endl;
        
        // محاكاة جمع بيانات الشبكة كل 5 ثواني
        while (running) {
            updateNetworkStats();
            checkLimits();
            this_thread::sleep_for(chrono::seconds(5));
        }
    }

    void stop() {
        running = false;
        cout << "🛑 إيقاف مراقبة الشبكة" << endl;
    }

    void updateNetworkStats() {
        // في تطبيق حقيقي، هنا نستخدم APIs نظام التشغيل
        // للحصول على بيانات الشبكة من كل جهاز
        
#ifdef _WIN32
        updateWindowsStats();
#else
        updateLinuxStats();
#endif
    }

#ifdef _WIN32
    void updateWindowsStats() {
        // استخدام Windows APIs مثل GetIfTable, GetTcpTable, إلخ
        // هذا كود مثال مبسط
        cout << "📊 تحديث إحصائيات Windows" << endl;
    }
#else
    void updateLinuxStats() {
        // استخدام /proc/net/dev و ARP table
        cout << "📊 تحديث إحصائيات Linux" << endl;
    }
#endif

    void checkLimits() {
        lock_guard<mutex> lock(devicesMutex);
        
        for (auto& [mac, device] : devices) {
            if (device.limit > 0 && device.alertEnabled) {
                long long totalUsage = device.downloaded + device.uploaded;
                
                if (totalUsage >= device.limit) {
                    string message = "تجاوز الحد الأقصى: " + 
                                   to_string(totalUsage / (1024 * 1024)) + " MB";
                    db.logAlert(mac, "limit_exceeded", message);
                    cout << "⚠️  تنبيه: " << message << endl;
                }
            }
        }
    }

    void addDevice(const string& mac, const string& ip, const string& hostname = "") {
        lock_guard<mutex> lock(devicesMutex);
        
        Device device;
        device.mac = mac;
        device.ip = ip;
        device.hostname = hostname.empty() ? ip : hostname;
        device.downloaded = 0;
        device.uploaded = 0;
        device.lastSeen = time(nullptr);
        device.limit = 0;
        device.limitType = "";
        device.alertEnabled = false;
        
        devices[mac] = device;
        db.saveDeviceStats(mac, ip, 0, 0);
    }

    vector<Device> getDevices() {
        vector<Device> result = db.getAllDevices();
        return result;
    }

    void setLimit(const string& mac, long long limit, const string& limitType) {
        db.setDeviceLimit(mac, limit, limitType);
        cout << "✅ تم تعيين حد: " << mac << " - " << limit / (1024 * 1024) << " MB" << endl;
    }

    string formatBytes(long long bytes) {
        const char* sizes[] = {"B", "KB", "MB", "GB", "TB"};
        double len = bytes;
        int order = 0;
        
        while (len >= 1024 && order < 4) {
            order++;
            len = len / 1024;
        }
        
        stringstream ss;
        ss << fixed << setprecision(2) << len << " " << sizes[order];
        return ss.str();
    }

    void printDevices() {
        vector<Device> devices = getDevices();
        
        cout << "\n════════════════════════════════════════════════════════════════" << endl;
        cout << "📱 قائمة الأجهزة المتصلة (" << devices.size() << " أجهزة)" << endl;
        cout << "════════════════════════════════════════════════════════════════" << endl;
        
        for (size_t i = 0; i < devices.size(); i++) {
            cout << "\n[" << (i + 1) << "] " << devices[i].hostname << endl;
            cout << "   📍 IP: " << devices[i].ip << endl;
            cout << "   🔗 MAC: " << devices[i].mac << endl;
            cout << "   ⬇️  التحميل: " << formatBytes(devices[i].downloaded) << endl;
            cout << "   ⬆️  الرفع: " << formatBytes(devices[i].uploaded) << endl;
            cout << "   📊 الإجمالي: " << formatBytes(devices[i].downloaded + devices[i].uploaded) << endl;
            
            if (devices[i].limit > 0) {
                long long usage = devices[i].downloaded + devices[i].uploaded;
                int percent = (usage * 100) / devices[i].limit;
                cout << "   ⚙️  الحد: " << formatBytes(devices[i].limit) << " (" << percent << "%)" << endl;
            }
        }
        
        cout << "\n════════════════════════════════════════════════════════════════" << endl;
    }
};

// البرنامج الرئيسي
int main() {
    cout << "🚀 NetWork Manager - مراقب الشبكة (C++)" << endl;
    cout << "========================================" << endl;

    // تهيئة قاعدة البيانات
    Database db("network_monitor.db");
    cout << "✅ قاعدة البيانات جاهزة" << endl;

    // تهيئة مراقب الشبكة
    NetworkMonitor monitor(db);
    cout << "✅ مراقب الشبكة جاهز" << endl;

    // إضافة أجهزة للاختبار
    monitor.addDevice("A1:B2:C3:D4:E5:F6", "192.168.1.5", "Samsung TV");
    monitor.addDevice("11:22:33:44:55:66", "192.168.1.10", "iPhone");
    monitor.addDevice("77:88:99:AA:BB:CC", "192.168.1.15", "Laptop");

    // تعيين حدود
    monitor.setLimit("A1:B2:C3:D4:E5:F6", 5000 * 1024 * 1024, "monthly");
    monitor.setLimit("11:22:33:44:55:66", 3000 * 1024 * 1024, "monthly");

    // طباعة قائمة الأجهزة
    monitor.printDevices();

    // قائمة تفاعلية
    string choice;
    while (true) {
        cout << "\n📋 الخيارات:" << endl;
        cout << "1. عرض الأجهزة" << endl;
        cout << "2. تعيين حد جديد" << endl;
        cout << "3. خروج" << endl;
        cout << "اختر: ";
        cin >> choice;

        if (choice == "1") {
            monitor.printDevices();
        } else if (choice == "2") {
            string mac;
            long long limitMB;
            cout << "أدخل عنوان MAC: ";
            cin >> mac;
            cout << "أدخل الحد بالميجابايت: ";
            cin >> limitMB;
            monitor.setLimit(mac, limitMB * 1024 * 1024, "monthly");
        } else if (choice == "3") {
            cout << "👋 وداعاً!" << endl;
            break;
        } else {
            cout << "❌ خيار غير صحيح" << endl;
        }
    }

    return 0;
}
