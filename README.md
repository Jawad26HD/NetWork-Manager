# NetWork Manager - النسخة النهائية 🌐

نظام شامل لمراقبة استهلاك الإنترنت للأجهزة المتصلة بالشبكة

## المشروع يحتوي على 3 نسخ:

### 1️⃣ **Desktop App (Electron + JavaScript)**
- واجهة رسومية سهلة وجميلة
- تعمل على Windows, Mac, Linux
- بيانات وهمية للاختبار السريع

**التشغيل:**
```bash
cd desktop-app
npm install
npm start
```

---

### 2️⃣ **Backend (Go)**
- خادم REST API
- مراقبة حقيقية للشبكة
- قاعدة بيانات SQLite

**التشغيل:**
```bash
cd router-backend
go build
./network-monitor  # أو ./network-monitor.exe على Windows
```

---

### 3️⃣ **Command Line App (C++)**
- برنامج سطر الأوامر
- سريع وخفيف
- مثالي للخوادم

**التشغيل:**
```bash
cd cpp-monitor
mkdir build && cd build
cmake ..
cmake --build . --config Release
./bin/network_monitor
```

---

## الميزات الرئيسية ✨

✅ مراقبة جميع الأجهزة على الشبكة
✅ قياس استهلاك البيانات (Upload/Download)
✅ تعيين حدود استهلاك
✅ إنذارات تلقائية
✅ تخزين البيانات في قاعدة بيانات
✅ واجهات متعددة (Desktop, CLI, Web API)

---

## المتطلبات 📋

### للـ Desktop App:
- Node.js 14+
- npm

### للـ Backend (Go):
- Go 1.16+
- SQLite3

### للـ C++ App:
- CMake 3.10+
- Compiler (GCC, Clang, MSVC)
- SQLite3 Dev Libraries

---

## البنية المشروع 📁

```
NetWork-Manager/
├── desktop-app/          # تطبيق Electron
│   ├── src/
│   │   ├── main.js
│   │   ├── index.html
│   │   ├── styles.css
│   │   └── renderer.js
│   └── package.json
│
├── router-backend/       # Backend بـ Go
│   ├── main.go
│   ├── monitor.go
│   ├── database.go
│   ├── api.go
│   └── go.mod
│
├── cpp-monitor/          # برنامج C++
│   ├── network_monitor.cpp
│   ├── CMakeLists.txt
│   └── BUILD.md
│
└── README.md             # هذا الملف
```

---

## البدء السريع 🚀

### خطوة 1: استنساخ المشروع
```bash
git clone https://github.com/Jawad26HD/NetWork-Manager.git
cd NetWork-Manager
```

### خطوة 2: اختر النسخة المناسبة

**للمبتدئين (Desktop App):**
```bash
cd desktop-app
npm install
npm start
```

**للمتقدمين (Go Backend):**
```bash
cd router-backend
go build
./network-monitor
```

**لخوادم الإنتاج (C++):**
```bash
cd cpp-monitor
mkdir build && cd build
cmake .. && cmake --build .
./bin/network_monitor
```

---

## API Endpoints (Go Backend) 🔌

```
GET  /api/devices              # جلب قائمة الأجهزة
GET  /api/devices/{mac}/stats  # إحصائيات جهاز معين
POST /api/devices/{mac}/limit  # تعيين حد الاستهلاك
GET  /api/devices/{mac}/alerts # جلب التنبيهات
GET  /api/health               # فحص حالة الخادم
GET  /api/stats                # الإحصائيات العامة
```

---

## الترخيص 📜

MIT License - استخدم حراً!

---

## التطوير المستقبلي 🔮

- [ ] إضافة واجهة ويب (React)
- [ ] تطبيق موبايل (React Native)
- [ ] تكامل مع خدمات السحابة
- [ ] نظام التنبيهات المتقدم
- [ ] تقارير تفصيلية

---

## المساهمة 🤝

Pull Requests مرحب بها!
Issues والاقتراحات يمكنك فتحها في GitHub

---

## التواصل 📧

- GitHub: [@Jawad26HD](https://github.com/Jawad26HD)
- Email: المتوفرة في الملف الشخصي

---

**🎉 شكراً لاستخدامك NetWork Manager!**
