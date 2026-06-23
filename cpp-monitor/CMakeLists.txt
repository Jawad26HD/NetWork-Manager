cmake_minimum_required(VERSION 3.10)
project(NetworkMonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# البحث عن SQLite3
find_package(SQLite3 REQUIRED)

# إضافة ملفات المصدر
add_executable(network_monitor network_monitor.cpp)

# ربط المكتبات
target_link_libraries(network_monitor PRIVATE SQLite3::SQLite3)

# إضافة include directories
target_include_directories(network_monitor PRIVATE ${SQLITE3_INCLUDE_DIRS})

# إضافة خيارات المترجم
if(MSVC)
    target_compile_options(network_monitor PRIVATE /W4 /utf-8)
else()
    target_compile_options(network_monitor PRIVATE -Wall -Wextra -Wpedantic)
endif()

# تعيين مسار الإخراج
set_target_properties(network_monitor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# رسالة عند الانتهاء
message(STATUS "✅ المشروع جاهز للتجميع")
message(STATUS "💾 البرنامج: ${CMAKE_BINARY_DIR}/bin/network_monitor")
