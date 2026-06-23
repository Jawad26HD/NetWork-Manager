package main

import (
	"encoding/json"
	"fmt"
	"net/http"
	"strconv"
	"strings"

	"github.com/gorilla/mux"
)

// APIServer خادم API
type APIServer struct {
	db      *DB
	monitor *NetworkMonitor
	router  *mux.Router
}

// NewAPIServer ينشئ خادم API جديد
func NewAPIServer(db *DB, monitor *NetworkMonitor) *APIServer {
	server := &APIServer{
		db:      db,
		monitor: monitor,
		router:  mux.NewRouter(),
	}

	// تسجيل المسارات
	server.registerRoutes()

	return server
}

// registerRoutes تسجيل جميع مسارات API
func (s *APIServer) registerRoutes() {
	// الأجهزة
	s.router.HandleFunc("/api/devices", s.handleGetDevices).Methods("GET")
	s.router.HandleFunc("/api/devices/{mac}/stats", s.handleGetDeviceStats).Methods("GET")
	s.router.HandleFunc("/api/devices/{mac}/limit", s.handleSetLimit).Methods("POST")
	s.router.HandleFunc("/api/devices/{mac}/alerts", s.handleGetAlerts).Methods("GET")

	// الحالة
	s.router.HandleFunc("/api/health", s.handleHealth).Methods("GET")
	s.router.HandleFunc("/api/stats", s.handleGetStats).Methods("GET")

	// CORS
	s.router.Use(corsMiddleware)
}

// corsMiddleware لتفعيل CORS
func corsMiddleware(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Access-Control-Allow-Origin", "*")
		w.Header().Set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
		w.Header().Set("Access-Control-Allow-Headers", "Content-Type, Authorization")
		w.Header().Set("Content-Type", "application/json")

		if r.Method == "OPTIONS" {
			w.WriteHeader(http.StatusOK)
			return
		}

		next.ServeHTTP(w, r)
	})
}

// Response هيكل الاستجابة العامة
type Response struct {
	Success bool        `json:"success"`
	Message string      `json:"message,omitempty"`
	Data    interface{} `json:"data,omitempty"`
	Error   string      `json:"error,omitempty"`
}

// DeviceInfo معلومات الجهاز للاستجابة
type DeviceInfo struct {
	MAC          string `json:"mac"`
	IP           string `json:"ip"`
	Hostname     string `json:"hostname"`
	Downloaded   string `json:"downloaded"`   // بصيغة إنسانية
	Uploaded     string `json:"uploaded"`     // بصيغة إنسانية
	Total        string `json:"total"`        // إجمالي
	Limit        string `json:"limit"`        // الحد الأقصى
	LimitType    string `json:"limit_type"`
	AlertEnabled bool   `json:"alert_enabled"`
	Usage        int    `json:"usage_percent"` // نسبة الاستخدام
}

// handleGetDevices جلب قائمة الأجهزة
func (s *APIServer) handleGetDevices(w http.ResponseWriter, r *http.Request) {
	devices := s.monitor.GetDevices()

	deviceList := make([]DeviceInfo, 0, len(devices))
	for _, device := range devices {
		download, upload, _ := s.db.GetDeviceStats(device.MAC)
		
		deviceInfo := DeviceInfo{
			MAC:          device.MAC,
			IP:           device.IP,
			Hostname:     device.Hostname,
			Downloaded:   formatBytes(download),
			Uploaded:     formatBytes(upload),
			Total:        formatBytes(download + upload),
			Limit:        formatBytes(device.Limit),
			LimitType:    device.LimitType,
			AlertEnabled: device.AlertEnabled,
		}

		// حساب نسبة الاستخدام
		if device.Limit > 0 {
			deviceInfo.Usage = int((download + upload) * 100 / device.Limit)
		}

		deviceList = append(deviceList, deviceInfo)
	}

	sendJSON(w, http.StatusOK, Response{
		Success: true,
		Message: fmt.Sprintf("Found %d devices", len(deviceList)),
		Data:    deviceList,
	})
}

// handleGetDeviceStats جلب إحصائيات جهاز معين
func (s *APIServer) handleGetDeviceStats(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	mac := vars["mac"]

	download, upload, err := s.db.GetDeviceStats(mac)
	if err != nil {
		sendJSON(w, http.StatusInternalServerError, Response{
			Success: false,
			Error:   "Failed to fetch stats",
		})
		return
	}

	stats := map[string]interface{}{
		"mac":        mac,
		"download":  formatBytes(download),
		"download_bytes": download,
		"upload":    formatBytes(upload),
		"upload_bytes": upload,
		"total":     formatBytes(download + upload),
		"total_bytes": download + upload,
	}

	sendJSON(w, http.StatusOK, Response{
		Success: true,
		Data:    stats,
	})
}

// handleSetLimit تعيين حد الاستهلاك
func (s *APIServer) handleSetLimit(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	mac := vars["mac"]

	var req struct {
		Limit     int64  `json:"limit"`      // بالميجابايت
		LimitType string `json:"limit_type"` // "daily" أو "monthly"
	}

	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		sendJSON(w, http.StatusBadRequest, Response{
			Success: false,
			Error:   "Invalid request body",
		})
		return
	}

	// تحويل من ميجابايت إلى بايت
	limitBytes := req.Limit * 1024 * 1024

	if err := s.monitor.SetLimit(mac, limitBytes, req.LimitType); err != nil {
		sendJSON(w, http.StatusBadRequest, Response{
			Success: false,
			Error:   err.Error(),
		})
		return
	}

	sendJSON(w, http.StatusOK, Response{
		Success: true,
		Message: fmt.Sprintf("Limit set to %s (%s)", formatBytes(limitBytes), req.LimitType),
	})
}

// handleGetAlerts جلب الإنذارات
func (s *APIServer) handleGetAlerts(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	mac := vars["mac"]

	alerts, err := s.db.GetAlertsForDevice(mac)
	if err != nil {
		sendJSON(w, http.StatusInternalServerError, Response{
			Success: false,
			Error:   "Failed to fetch alerts",
		})
		return
	}

	sendJSON(w, http.StatusOK, Response{
		Success: true,
		Data: map[string]interface{}{
			"mac":    mac,
			"alerts": alerts,
			"count":  len(alerts),
		},
	})
}

// handleHealth فحص صحة الخدمة
func (s *APIServer) handleHealth(w http.ResponseWriter, r *http.Request) {
	sendJSON(w, http.StatusOK, Response{
		Success: true,
		Message: "Router monitor is healthy",
		Data: map[string]interface{}{
			"status":  "running",
			"devices": len(s.monitor.GetDevices()),
		},
	})
}

// handleGetStats جلب الإحصائيات العامة
func (s *APIServer) handleGetStats(w http.ResponseWriter, r *http.Request) {
	devices := s.monitor.GetDevices()

	var totalDownload, totalUpload int64
	for _, device := range devices {
		download, upload, _ := s.db.GetDeviceStats(device.MAC)
		totalDownload += download
		totalUpload += upload
	}

	sendJSON(w, http.StatusOK, Response{
		Success: true,
		Data: map[string]interface{}{
			"total_devices":   len(devices),
			"total_download":  formatBytes(totalDownload),
			"total_upload":    formatBytes(totalUpload),
			"total_usage":     formatBytes(totalDownload + totalUpload),
			"download_bytes":  totalDownload,
			"upload_bytes":    totalUpload,
		},
	})
}

// Router ترجع جهاز التوجيه
func (s *APIServer) Router() *mux.Router {
	return s.router
}

// Helper Functions

// formatBytes تحويل البايتات إلى صيغة إنسانية
func formatBytes(bytes int64) string {
	units := []string{"B", "KB", "MB", "GB", "TB"}
	value := float64(bytes)

	for _, unit := range units {
		if value < 1024 {
			return fmt.Sprintf("%.2f %s", value, unit)
		}
		value /= 1024
	}

	return fmt.Sprintf("%.2f PB", value)
}

// sendJSON إرسال استجابة JSON
func sendJSON(w http.ResponseWriter, statusCode int, data interface{}) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(statusCode)
	json.NewEncoder(w).Encode(data)
}

// parseLimit تحليل قيمة الحد من نص
func parseLimit(limitStr string) (int64, error) {
	// تحليل قيمة مثل "100MB" أو "1GB"
	limitStr = strings.TrimSpace(limitStr)
	
	multipliers := map[string]int64{
		"B":  1,
		"KB": 1024,
		"MB": 1024 * 1024,
		"GB": 1024 * 1024 * 1024,
		"TB": 1024 * 1024 * 1024 * 1024,
	}

	for unit, multiplier := range multipliers {
		if strings.HasSuffix(strings.ToUpper(limitStr), unit) {
			numStr := strings.TrimSuffix(strings.TrimSpace(limitStr[:len(limitStr)-len(unit)]), " ")
			num, err := strconv.ParseFloat(numStr, 64)
			if err != nil {
				return 0, err
			}
			return int64(num * float64(multiplier)), nil
		}
	}

	// محاولة تحليل كرقم مباشر (بالبايتات)
	num, err := strconv.ParseInt(limitStr, 10, 64)
	return num, err
}
