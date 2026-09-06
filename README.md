# RemoteMonitor

MVP cho đề tài PBL4 509: giám sát và điều khiển máy tính Windows trong mạng LAN.

## Đặc điểm

- `monitor_manager`: Qt GUI, TCP server, SQLite và danh sách agent.
- `monitor_agent`: Qt GUI, kết nối chủ động tới manager, heartbeat, thông tin hệ thống,
  screenshot và quyền điều khiển có xác nhận.

## Screen viewer performance

The Manager screen viewer uses a back-pressure pipeline: it asks an Agent for a
new frame only after the previous one arrives. This prevents the TCP write queue
from growing until the UI lags or becomes unstable.

- **30 FPS - HD** (default): JPEG up to 1280x720, quality 55.
- **60 FPS - HD performance**: JPEG up to 1280x720, quality 45.

These are target rates. The achieved rate depends on the Agent CPU, Wi-Fi/LAN
bandwidth and display resolution. Select the profile in the Manager dashboard.
- `monitor_common`: JSON message protocol và TCP length-prefixed framing.
- Không có keylogger ẩn, không lưu nội dung phím và không chạy shell tùy ý.

## Build trên Windows

Cần Qt 6 (Core, Gui, Widgets, Network, Sql, Test), CMake 3.21+ và compiler C++20.

Nếu dùng MSYS2 UCRT64, cài dependency bằng:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-qt6-base
```

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;$env:PATH"
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="C:\Qt\6.x.x\msvc2022_64"
cmake --build build
ctest --test-dir build --output-on-failure
```

Với Qt cài từ MSYS2, có thể bỏ `CMAKE_PREFIX_PATH` vì CMake sẽ tìm Qt trong
`C:\msys64\ucrt64`.

Chạy manager trước, đăng nhập mặc định trong bản demo là `admin` / `admin`, sau đó mở
agent và nhập địa chỉ manager. Port mặc định là TCP `45454`.

## Lưu ý bảo mật

MVP hiện dùng TCP thuần cho lab LAN. Không mở port ra Internet. Trước khi triển khai
thực tế cần thay socket bằng TLS, dùng password hashing chuyên dụng và bổ sung chính
xác sách firewall/triển khai.
