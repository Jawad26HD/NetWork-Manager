// ملفات الإعدادات المحلية
const API_BASE_URL = 'http://localhost:9090/api';
let allDevices = [];
let refreshInterval = null;

// عند فتح الصفحة
document.addEventListener('DOMContentLoaded', () => {
  console.log('🚀 تطبيق مراقبة الإنترنت بدأ التشغيل');
  
  // تهيئة الأحداث
  initializeEventListeners();
  
  // تحديث البيانات كل 3 ثواني
  refreshData();
  refreshInterval = setInterval(refreshData, 3000);
});

// تهيئة الأحداث
function initializeEventListeners() {
  // البحث
  document.getElementById('search-input').addEventListener('input', filterDevices);
  
  // التصفية
  document.getElementById('btn-all').addEventListener('click', () => {
    filterByType('all');
    updateFilterButtons('btn-all');
  });
  
  document.getElementById('btn-alert').addEventListener('click', () => {
    filterByType('alert');
    updateFilterButtons('btn-alert');
  });
  
  document.getElementById('btn-refresh').addEventListener('click', refreshData);
  
  // Modal للحدود
  const modal = document.getElementById('limit-modal');
  const closeBtn = document.querySelector('.close');
  const cancelBtn = document.getElementById('btn-cancel-limit');
  const saveBtn = document.getElementById('btn-save-limit');
  
  closeBtn.addEventListener('click', () => closeModal(modal));
  cancelBtn.addEventListener('click', () => closeModal(modal));
  saveBtn.addEventListener('click', saveLimitSettings);
  
  window.addEventListener('click', (event) => {
    if (event.target === modal) {
      closeModal(modal);
    }
  });
}

// تحديث البيانات
async function refreshData() {
  try {
    // الحصول على الأجهزة
    const response = await fetch(`${API_BASE_URL}/devices`);
    const data = await response.json();
    
    if (data.success) {
      allDevices = data.data || [];
      updateUI();
    }
  } catch (error) {
    console.error('❌ خطأ في جلب البيانات:', error);
    // في حالة الخطأ، سنستخدم بيانات وهمية للاختبار
    if (allDevices.length === 0) {
      loadMockData();
      updateUI();
    }
  }
}

// بيانات وهمية للاختبار
function loadMockData() {
  allDevices = [
    {
      mac: 'A1:B2:C3:D4:E5:F6',
      ip: '192.168.1.5',
      hostname: 'Samsung TV',
      downloaded: '256.50 MB',
      uploaded: '10.20 MB',
      total: '266.70 MB',
      limit: '5000.00 MB',
      limit_type: 'monthly',
      alert_enabled: true,
      usage_percent: 5
    },
    {
      mac: '11:22:33:44:55:66',
      ip: '192.168.1.10',
      hostname: 'iPhone',
      downloaded: '1256.80 MB',
      uploaded: '230.45 MB',
      total: '1487.25 MB',
      limit: '3000.00 MB',
      limit_type: 'monthly',
      alert_enabled: true,
      usage_percent: 50
    },
    {
      mac: '77:88:99:AA:BB:CC',
      ip: '192.168.1.15',
      hostname: 'Laptop',
      downloaded: '4512.30 MB',
      uploaded: '890.10 MB',
      total: '5402.40 MB',
      limit: '10000.00 MB',
      limit_type: 'monthly',
      alert_enabled: true,
      usage_percent: 54
    },
    {
      mac: 'DD:EE:FF:00:11:22',
      ip: '192.168.1.20',
      hostname: 'PlayStation',
      downloaded: '9856.75 MB',
      uploaded: '1245.60 MB',
      total: '11102.35 MB',
      limit: '15000.00 MB',
      limit_type: 'monthly',
      alert_enabled: false,
      usage_percent: 74
    }
  ];
}

// تحديث الواجهة
function updateUI() {
  updateStats();
  renderDevicesTable();
}

// تحديث الإحصائيات
function updateStats() {
  let totalDevices = allDevices.length;
  let totalDownload = 0;
  let totalUpload = 0;
  
  allDevices.forEach(device => {
    // تحليل الأرقام من الصيغة النصية
    totalDownload += parseSize(device.downloaded);
    totalUpload += parseSize(device.uploaded);
  });
  
  document.getElementById('total-devices').textContent = totalDevices;
  document.getElementById('total-download').textContent = formatBytes(totalDownload);
  document.getElementById('total-upload').textContent = formatBytes(totalUpload);
  document.getElementById('total-usage').textContent = formatBytes(totalDownload + totalUpload);
}

// عرض جدول الأجهزة
function renderDevicesTable() {
  const tbody = document.getElementById('devices-tbody');
  tbody.innerHTML = '';
  
  allDevices.forEach((device, index) => {
    const row = document.createElement('tr');
    const usagePercent = device.usage_percent || 0;
    let usageClass = '';
    
    if (usagePercent > 90) {
      usageClass = 'critical';
    } else if (usagePercent > 70) {
      usageClass = 'warning';
    }
    
    row.innerHTML = `
      <td>${index + 1}</td>
      <td><span class="device-name">${device.hostname || 'جهاز غير معروف'}</span></td>
      <td><span class="device-ip">${device.ip}</span></td>
      <td><span class="device-mac">${device.mac}</span></td>
      <td>${device.downloaded}</td>
      <td>${device.uploaded}</td>
      <td><strong>${device.total}</strong></td>
      <td>${device.limit}</td>
      <td>
        <div class="usage-bar">
          <div class="usage-fill ${usageClass}" style="width: ${Math.min(usagePercent, 100)}%">
            ${usagePercent}%
          </div>
        </div>
      </td>
      <td>
        <div class="device-actions">
          <button class="action-btn btn-primary" onclick="openLimitModal('${device.mac}', '${device.hostname}', '${device.ip}')">
            ⚙️ حد
          </button>
          <button class="action-btn btn-secondary" onclick="viewDeviceDetails('${device.mac}')">
            👁️ تفاصيل
          </button>
        </div>
      </td>
    `;
    
    tbody.appendChild(row);
  });
}

// فتح modal لتعيين الحد
function openLimitModal(mac, hostname, ip) {
  const modal = document.getElementById('limit-modal');
  document.getElementById('device-name').value = hostname;
  document.getElementById('device-ip').value = ip;
  document.getElementById('limit-value').value = '';
  document.getElementById('limit-type').value = 'monthly';
  modal.dataset.mac = mac;
  modal.classList.add('show');
}

// إغلاق Modal
function closeModal(modal) {
  modal.classList.remove('show');
}

// حفظ إعدادات الحد
async function saveLimitSettings() {
  const mac = document.getElementById('limit-modal').dataset.mac;
  const limitValue = parseInt(document.getElementById('limit-value').value);
  const limitType = document.getElementById('limit-type').value;
  
  if (!limitValue || limitValue < 1) {
    showNotification('الرجاء إدخال قيمة صحيحة للحد', 'error');
    return;
  }
  
  try {
    const response = await fetch(`${API_BASE_URL}/devices/${mac}/limit`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        limit: limitValue,
        limit_type: limitType
      })
    });
    
    const data = await response.json();
    
    if (data.success) {
      showNotification('✅ تم حفظ الحد بنجاح', 'success');
      closeModal(document.getElementById('limit-modal'));
      refreshData();
    } else {
      showNotification('❌ خطأ: ' + data.error, 'error');
    }
  } catch (error) {
    console.error('خطأ:', error);
    showNotification('❌ خطأ في الاتصال', 'error');
  }
}

// عرض تفاصيل الجهاز
async function viewDeviceDetails(mac) {
  try {
    const response = await fetch(`${API_BASE_URL}/devices/${mac}/alerts`);
    const data = await response.json();
    
    if (data.success) {
      const alerts = data.data.alerts || [];
      let alertsText = alerts.length > 0 
        ? alerts.join('\n')
        : 'لا توجد تنبيهات';
      
      alert(`التنبيهات للجهاز:\n\n${alertsText}`);
    }
  } catch (error) {
    console.error('خطأ:', error);
  }
}

// البحث عن جهاز
function filterDevices() {
  const searchText = document.getElementById('search-input').value.toLowerCase();
  const rows = document.querySelectorAll('#devices-tbody tr');
  
  rows.forEach(row => {
    const hostname = row.querySelector('.device-name').textContent.toLowerCase();
    const ip = row.querySelector('.device-ip').textContent.toLowerCase();
    const mac = row.querySelector('.device-mac').textContent.toLowerCase();
    
    const matches = hostname.includes(searchText) || 
                   ip.includes(searchText) || 
                   mac.includes(searchText);
    
    row.style.display = matches ? '' : 'none';
  });
}

// التصفية حسب النوع
function filterByType(type) {
  const rows = document.querySelectorAll('#devices-tbody tr');
  
  rows.forEach(row => {
    if (type === 'all') {
      row.style.display = '';
    } else if (type === 'alert') {
      // إظهار فقط الأجهزة ذات نسبة استخدام عالية
      const usageText = row.querySelector('.usage-fill').textContent;
      const usage = parseInt(usageText);
      row.style.display = usage > 70 ? '' : 'none';
    }
  });
}

// تحديث أزرار التصفية
function updateFilterButtons(activeBtn) {
  document.querySelectorAll('.filter-btn').forEach(btn => {
    btn.classList.remove('active');
  });
  document.getElementById(activeBtn).classList.add('active');
}

// عرض التنبيهات
function showNotification(message, type = 'success') {
  const notification = document.getElementById('notification');
  notification.textContent = message;
  notification.className = `notification show ${type}`;
  
  setTimeout(() => {
    notification.classList.remove('show');
  }, 3000);
}

// دوال مساعدة
function formatBytes(bytes) {
  if (bytes === 0) return '0 B';
  
  const k = 1024;
  const sizes = ['B', 'KB', 'MB', 'GB', 'TB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  
  return Math.round(bytes / Math.pow(k, i) * 100) / 100 + ' ' + sizes[i];
}

function parseSize(sizeStr) {
  const parts = sizeStr.trim().split(' ');
  const value = parseFloat(parts[0]);
  const unit = parts[1];
  
  const units = {
    'B': 1,
    'KB': 1024,
    'MB': 1024 * 1024,
    'GB': 1024 * 1024 * 1024,
    'TB': 1024 * 1024 * 1024 * 1024
  };
  
  return value * (units[unit] || 1);
}

// تحميل البيانات الوهمية عند البداية
window.addEventListener('load', () => {
  loadMockData();
  updateUI();
  showNotification('📊 تم تحميل البيانات بنجاح', 'success');
});
