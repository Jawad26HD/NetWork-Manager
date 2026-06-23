# NetWork Manager - تجميع وتشغيل برنامج C++

## متطلبات التثبيت 📋

### على Windows:
```bash
# 1. تثبيت Visual Studio Build Tools أو MinGW
# 2. تثبيت CMake من: https://cmake.org/download/

# 3. تثبيت SQLite3 (اختياري، قد يكون مدمج)
# يمكنك تحميله من: https://www.sqlite.org/download.html
```

### على Linux (Ubuntu/Debian):
```bash
sudo apt-get install build-essential cmake sqlite3 libsqlite3-dev
```

### على macOS:
```bash
brew install cmake sqlite3
```

---

## خطوات التجميع والتشغيل 🚀

### 1️⃣ فتح Terminal/Command Prompt

### 2️⃣ الذهاب إلى مجلد المشروع:
```bash
cd NetWork-Manager/cpp-monitor
```

### 3️⃣ إنشاء مجلد البناء:
```bash
mkdir build
cd build
```

### 4️⃣ تشغيل CMake:

**على Windows (مع Visual Studio):**
```bash
cmake .. -G "Visual Studio 16 2019"
```

**على Windows (مع MinGW):**
```bash
cmake .. -G "MinGW Makefiles"
```

**على Linux/Mac:**
```bash
cmake ..
```

### 5️⃣ تجميع البرنامج:

**على Windows:**
```bash
cmake --build . --config Release
```

**على Linux/Mac:**
```bash
make
```

### 6️⃣ تشغيل البرنامج:

**على Windows:**
```bash
.\bin\network_monitor.exe
```

**على Linux/Mac:**
```bash
./bin/network_monitor
```

---

## إذا حصلت مشاكل 🔧

### ❌ CMake not found:
```bash
# تأكد من تثبيت CMake وإضافته إلى PATH
# أعد فتح Terminal بعد التثبيت
```

### ❌ SQLite3 not found:
```bash
# Windows: حمل من https://www.sqlite.org/download.html
# Linux: sudo apt-get install libsqlite3-dev
# Mac: brew install sqlite3
```

### ❌ Compiler not found:
```bash
# Windows: ثبت Visual Studio Build Tools
# Linux: sudo apt-get install build-essential
# Mac: xcode-select --install
```

---

## ملاحظات مهمة ⚠️

- البرنامج ينشئ ملف `network_monitor.db` تلقائياً
- استخدم `Ctrl+C` للخروج من البرنامج
- يمكنك تعديل البرنامج وإعادة تجميعه

---

## أوامر مفيدة:

```bash
# تنظيف ملفات البناء
rm -rf build    # Linux/Mac
rmdir /s build  # Windows

# إعادة البناء من الصفر
cmake --build . --clean-first

# بناء مع معلومات التصحيح
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

---

**🎉 هكذا تشغل برنامج C++ بنجاح!**
