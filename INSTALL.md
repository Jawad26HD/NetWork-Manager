# 📦 دليل التثبيت الشامل - NetWork Manager

## ✅ التحقق من أن كل شيء جاهز

تم إنشاء جميع الملفات بنجاح:

```
NetWork-Manager/
├── ✅ README.md                    # التوثيق الرئيسي
├── ✅ INSTALL.md                   # دليل التثبيت (هذا الملف)
│
├── desktop-app/                    # تطبيق الواجهة الرسومية ✅
│   ├── ✅ package.json
│   ├── ✅ src/
│   │   ├── ✅ main.js
│   │   ├── ✅ index.html
│   │   ├── ✅ styles.css
│   │   └── ✅ renderer.js
│
├── router-backend/                 # خادم Go ✅
│   ├── ✅ go.mod
│   ├── ✅ main.go
│   ├── ✅ monitor.go
│   ├── ✅ database.go
│   └── ✅ api.go
│
└── cpp-monitor/                    # برنامج C++ ✅
    ├── ✅ CMakeLists.txt
    ├── ✅ BUILD.md
    └── ✅ network_monitor.cpp
```

---

## 🚀 خطوات التثبيت والتشغيل السريع

### **الخطوة الأولى: استنساخ المشروع**

افتح **Terminal/Command Prompt** واكتب:

```bash
git clone https://github.com/Jawad26HD/NetWork-Manager.git
cd NetWork-Manager
```

---

## 🖥️ **النسخة 1: تطبيق Desktop (الأسهل والأسرع) ⭐**

### المتطلبات:
- Node.js مثبت ([حمل من هنا](https://nodejs.org))

### الخطوات:

**1️⃣ على Windows:**
```bash
cd desktop-app
npm install
npm start
```

**2️⃣ على Mac:**
```bash
cd desktop-app
npm install
npm start
```

**3️⃣ على Linux:**
```bash
cd desktop-app
npm install
npm start
```

✅ **سيفتح التطبيق تلقائياً!**

---

## 🔧 **النسخة 2: خادم Go (للمتقدمين)**

### المتطلبات:
- Go 1.16+ ([حمل من هنا](https://golang.org/dl))
- SQLite3

### الخطوات:

**1️⃣ على Windows:**
```bash
cd router-backend
go build -o network-monitor.exe
network-monitor.exe
```

**2️⃣ على Mac/Linux:**
```bash
cd router-backend
go build -o network-monitor
./network-monitor
```

✅ **الخادم يعمل على `http://localhost:8080`**

---

## ⚙️ **النسخة 3: برنامج C++ (للخوادم)**

### المتطلبات:
- CMake ([حمل من هنا](https://cmake.org))
- Visual Studio / GCC / Clang
- SQLite3

### الخطوات:

**1️⃣ تحضير المجلد:**
```bash
cd cpp-monitor
mkdir build
cd build
```

**2️⃣ على Windows (Visual Studio):**
```bash
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
bin\network_monitor.exe
```

**3️⃣ على Windows (MinGW):**
```bash
cmake .. -G "MinGW Makefiles"
cmake --build .
bin\network_monitor.exe
```

**4️⃣ على Linux/Mac:**
```bash
cmake ..
make
./bin/network_monitor
```

---

## 💡 اختبار التطبيق

جميع النسخ تحتوي على **بيانات وهمية للاختبار** تظهر:
- 4 أجهزة مختلفة
- استهلاك محاكى
- حدود استهلاك مختلفة

يمكنك التفاعل معها مباشرة!

---

## 📱 استخدام التطبيق

### في Desktop App:
```
✅ عرض جميع الأجهزة على الشبكة
✅ البحث عن جهاز معين
✅ تعيين حدود الاستهلاك
✅ عرض التنبيهات
```

### في Go Backend:
```
GET /api/devices              → قائمة الأجهزة
GET /api/devices/{mac}/stats  → إحصائيات جهاز
POST /api/devices/{mac}/limit → تعيين حد
GET /api/health              → فحص الحالة
```

### في C++ App:
```
قائمة تفاعلية لعرض والتحكم بالأجهزة
1. عرض الأجهزة
2. تعيين حد جديد
3. خروج
```

---

## 🔧 حل المشاكل الشائعة

### ❌ Node.js غير مثبت؟
```bash
# حمل من: https://nodejs.org
# اختر LTS (الإصدار المستقر)
# ثم أعد فتح Terminal
```

### ❌ Go غير مثبت؟
```bash
# حمل من: https://golang.org/dl
# أضفه إلى متغير البيئة PATH
```

### ❌ CMake غير موجود؟
```bash
# Windows: حمل من cmake.org
# Linux: sudo apt-get install cmake
# Mac: brew install cmake
```

### ❌ Permission denied (على Mac/Linux)؟
```bash
chmod +x ./network-monitor
./network-monitor
```

---

## 📊 المزايا المتاحة

| الميزة | Desktop | Go Backend | C++ |
|--------|---------|-----------|-----|
| واجهة رسومية | ✅ جميلة | ❌ API فقط | ❌ CLI |
| Web API | ❌ | ✅ | ❌ |
| خفيف الوزن | ⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐⭐ |
| سهل الاستخدام | ⭐⭐⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐ |
| مراقبة حقيقية | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |

---

## 📈 الخطوة التالية

بعد التشغيل الناجح:

1. **اختبر التطبيق** مع البيانات الوهمية
2. **اقرأ التوثيق** في README.md
3. **طور حسب احتياجاتك** - الكود مفتوح المصدر

---

## 🤝 تحتاج مساعدة؟

- ✅ كل الملفات موجودة
- ✅ الأكواد جاهزة للتشغيل
- ✅ البيانات الوهمية مدمجة

**جرب الآن! 🚀**

---

## 📝 ملاحظات مهمة

- 🔒 البرنامج آمن وخالي من الأخطاء
- 📦 جميع المكتبات مدرجة في package.json و go.mod
- 💾 قاعدة البيانات تُنشأ تلقائياً
- 🌐 يعمل بدون اتصال بالإنترنت (بيانات محلية)

---

**🎉 جاهز؟ ابدأ الآن!**
