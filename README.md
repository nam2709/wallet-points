## 🧑‍🤝‍🧑 **PHÂN CÔNG CÔNG VIỆC**

### 1️⃣ Nguyễn Đăng Quang (Trưởng nhóm)
- Khởi tạo, xây dựng cấu trúc ban đầu cho toàn bộ dự án.  
- Lên kế hoạch, phân công nhiệm vụ và theo sát tiến độ của nhóm.  
- Viết tài liệu hướng dẫn cài đặt, giúp các thành viên dễ dàng nắm bắt và triển khai.  
- Trực tiếp phụ trách chính phần code **tính năng Đăng nhập**, **Ví**, **Giao dịch**, và **các chức năng của người dùng** – nền tảng quan trọng của hệ thống.  

---

### 2️⃣ Ngô Hà Nam
- Đảm nhận phần code **tính năng Đăng ký người dùng mới** và các chức năng liên quan đến User.  
- Phụ trách chính việc phát triển **chức năng phần Ví, Giao dịch, Admin** – những thành phần trọng tâm trong ứng dụng.  
- Phụ trách chính **kiểm tra toàn bộ các chức năng của module**.  
- Tạo **GitHub** và chịu trách nhiệm **push code** lên GitHub.  

---

### 3️⃣ Vũ Việt Anh
- Chịu trách nhiệm xây dựng **chức năng tải dữ liệu hệ thống** và **hiển thị danh sách menu**.  
- Thiết kế, phân chia **menu riêng biệt** cho **Người quản lý (Admin)** và **Người dùng (User)**, đồng thời xây dựng **Lịch sử giao dịch** đảm bảo tính rõ ràng và thuận tiện.  
- Phụ trách phần **tính năng Đăng nhập** và các chức năng của **Admin** như **sửa thông tin người dùng**.  

---

### 4️⃣ Hoàng Dương Nam
- **Kiểm tra các chức năng của module**.  
- Khi phát hiện **bug hoặc lỗi**, tham gia cùng các thành viên khác để **chỉnh sửa và hoàn thiện code**.  

<br><br>

# 🗂️ **Tổng quan về chương trình**

Sau khi tiến hành chạy chương trình, sẽ hiện ra các module như sau:

---

## 1️⃣ Register New User
Truy cập module này có thể tạo tài khoản cho người dùng bình thường, người đăng kí sẽ phải nhập **tên đăng nhập**, **mật khẩu**, và **khai báo tên người dùng**.

---

## 2️⃣ Register New Admin
Truy cập module này để có thể tạo tài khoản cho **người quản trị (admin)** cho hệ thống.  
Người đăng kí sẽ phải nhập **tên đăng nhập**, **mật khẩu**, và **khai báo tên người quản trị**.

---

## 3️⃣ Login to System
Module này dùng để đăng nhập vào hệ thống, yêu cầu nhập **tên đăng nhập** và **mật khẩu** để có thể truy cập vào hệ thống.  

**Lưu ý**:  
- Khi đăng nhập sẽ được chia thành **2 luồng**:  
  - **Người dùng**: sẽ đi theo 1 luồng với các module chức năng riêng.  
  - **Người quản trị**: sẽ có các module chức năng khác nhau.

---

### 👤 USER Modules

a) **View Profile Information**: cho phép người dùng kiểm tra thông tin tài khoản của mình  

b) **Change Password**: đổi mật khẩu của tài khoản, lưu ý có 1 bước check mật khẩu cũ cần phải nhập đúng mới có thể nhập mật khẩu mới cho tài khoản  

c) **Update Personal Information**: thay đổi thông tin cá nhân của mình (tên người dùng)  

d) **View Wallet & Transaction History**: Kiểm tra số dư ví và lịch sử giao dịch điểm của ví  

e) **Transfer Points to Another Wallet**: chuyển điểm sang ví khác (có yêu cầu OTP)  

f) **Request Top-up from Central Wallet**: Yêu cầu nạp điểm từ ví chính của hệ thống. Người dùng sẽ yêu cầu nạp 1 số điểm nhất định vào ví của mình. Tuy nhiên sẽ phải đợi Tài khoản quản trị chấp nhận yêu cầu này. Sau khi người quản trị chấp nhận yêu cầu, số điểm mới được chuyển tới ví người dùng  

g) **Process Admin Update Notifications**: Kiểm tra các tiến trình người quản trị thực thi lên tài khoản của người dùng. (Bao gồm chấp thuận việc người quản trị chỉnh sửa thông tin cá nhân)  

h) **Logout**: Đăng xuất tài khoản  

---

### 🛠️ ADMIN Modules

a) **List All Users**: Liệt kê các tài khoản có trong hệ thống  

b) **Create New User**: Tạo tài khoản mới cho hệ thống  

c) **Modify User Information**: Thay đổi thông tin của người dùng (Sau khi thay đổi cần được người dùng chấp thuận ở phần g(user))  

d) **View Central Wallet Balance**: Kiểm tra số dư ví tổng  

e) **Top-up User Wallet**: Nạp điểm cho ví của người dung  

f) **Approve Top-up Requests**: Danh sách các yêu cầu từ phía tài khoản người dùng và dùng để chấp thuận các yêu cầu chuyển điểm (yêu cầu từ phần f(user))  

g) **Logout**: Đăng xuất tài khoản  

---

## 4️⃣ Exit System
Chọn module này để có thể thoát chương trình hệ thống ví, điểm.