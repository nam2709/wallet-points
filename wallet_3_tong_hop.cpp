#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <ctime>
#include <random>
#include <functional>
#include <limits>

using namespace std;

struct PendingUpdate {
    string username;
    string fullname;
    string otp;
};

// Simple OTP service with alphanumeric support
class OTPService {
public:
    static string generateOTP(size_t length = 8) {
        static const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        static mt19937 rng(static_cast<unsigned>(time(nullptr)));
        static uniform_int_distribution<> dist(0, sizeof(charset) - 2);
        string otp;
        for (size_t i = 0; i < length; ++i) {
            otp += charset[dist(rng)];
        }
        return otp;
    }
    static bool verifyOTP(const string &sent, const string &input) {
        return sent == input;
    }
};

// User account class
class User {
public:
    string username;
    size_t password_hash;
    string full_name;
    bool is_admin;
    int wallet_id;
    bool must_change_password;

    User() : is_admin(false), wallet_id(0), must_change_password(false) {}
    User(const string &u, const string &pwd, const string &name, bool admin, int wid, bool force_change = false)
        : username(u), full_name(name), is_admin(admin), wallet_id(wid), must_change_password(force_change) {
        password_hash = hash<string>()(pwd);
    }

    bool checkPassword(const string &pwd) const {
        return hash<string>()(pwd) == password_hash;
    }
    void setPassword(const string &pwd) {
        password_hash = hash<string>()(pwd);
    }
};

// Wallet class for points and transaction logging
class Wallet {
public:
    int id;
    long long balance;
    vector<string> history;

    Wallet() : id(0), balance(0) {}
    Wallet(int _id) : id(_id), balance(0) {}

    void log(const string &entry) {
        history.push_back(entry);
        ofstream logf("transaction.db", ios::app);
        if (logf) {
            time_t now = time(nullptr);
            char buf[64];
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
            logf << "[" << buf << "] Wallet " << id << ": " << entry << '\n';
        }
    }
};

// Database with users and wallets
class Database {
public:
    unordered_map<string, User> users;
    unordered_map<int, Wallet> wallets;
    int next_wallet_id;

    void saveUsers() {
        ofstream ofs("users.db", ios::trunc);
        for (auto &p : users) {
            User &u = p.second;
            ofs << u.username << ' ' << u.password_hash << ' ' << u.full_name
                << ' ' << u.is_admin << ' ' << u.wallet_id << ' ' << u.must_change_password << '\n';
        }
    }
    void saveWallets() {
        ofstream ofs("wallets.db", ios::trunc);
        for (auto &p : wallets) {
            Wallet &w = p.second;
            ofs << w.id << ' ' << w.balance << '\n';
        }
    }
    void loadWallets() {
        ifstream ifs("wallets.db");
        if (!ifs) return;
        int id;
        long long bal;
        while (ifs >> id >> bal) {
            wallets[id] = Wallet(id);
            wallets[id].balance = bal;
        }
    }
    void loadUsers() {
        ifstream ifs("users.db");
        if (!ifs) return;
        string uname, fname;
        size_t pwd_hash;
        bool admin, force;
        int wid;
        while (ifs >> uname >> pwd_hash >> fname >> admin >> wid >> force) {
            User u;
            u.username = uname;
            u.full_name = fname;
            u.is_admin = admin;
            u.wallet_id = wid;
            u.must_change_password = force;
            u.password_hash = pwd_hash;
            users[uname] = u;
            next_wallet_id = max(next_wallet_id, wid + 1);
        }
    }

    Database() : next_wallet_id(1) {
        loadUsers();
        loadWallets();
        if (!wallets.count(0)) {
            wallets[0] = Wallet(0);
            wallets[0].balance = 1000000;
        }
    }
    ~Database() {
        backupFiles();
        // saveUsers();
        // saveWallets();
    }

private:
    void backupFiles() {
        time_t now = time(nullptr);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y%m%d%H%M%S", localtime(&now));
        string ts(buf);
        // rename("users.db", ("users.db." + ts).c_str());
        // rename("wallets.db", ("wallets.db." + ts).c_str());
        rename("users.db", "users_backup.db");
        rename("wallets.db", "wallets_backup.db");
    }
};

Database db;

// Authentication
User* login() {
    db.loadUsers();
    cout << "Username: ";
    string u, p;
    cin >> u;
    cout << "Password: ";
    cin >> p;
    auto it = db.users.find(u);
    if (it != db.users.end() && it->second.checkPassword(p)) {
        User *user = &it->second;
        if (user->must_change_password) {
            cout << "Temporary password detected. Please set a new password:\n";
            cout << "New password: ";
            cin >> p;
            user->setPassword(p);
            user->must_change_password = false;
            db.saveUsers();
            cout << "Password updated. Please log in again.\n";
            return nullptr;
        }
        return user;
    }
    cout << "Invalid credentials.\n";
    return nullptr;
}

// Registration
void registerUser(bool asAdmin = false) {
    db.loadUsers();
    db.loadWallets();
    cout << "Enter username: ";
    string u;
    cin >> u;
    if (db.users.count(u)) {
        cout << "Username already exists.\n";
        return;
    }
    cout << "Enter password (leave blank for auto-generation): ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string pwd;
    getline(cin, pwd);
    bool forceChange = false;
    if (pwd.empty()) {
        pwd = OTPService::generateOTP(10);
        cout << "Generated password: " << pwd << "\n";
        forceChange = true;
    }
    cout << "Full name: ";
    string name;
    getline(cin, name);
    int wid = db.next_wallet_id++;
    db.users[u] = User(u, pwd, name, asAdmin, wid, forceChange);
    if (!asAdmin) {
        db.wallets[wid] = Wallet(wid);
        cout << "User '" << u << "' created with wallet ID " << wid << ".\n";
    }

    // Save to file immediately
    db.saveUsers();
    db.saveWallets();
}

// Change password
void changePassword(User &user) {
    db.loadUsers();
    cout << "Current password: ";
    string oldp;
    cin >> oldp;
    if (!user.checkPassword(oldp)) {
        cout << "Incorrect password.\n";
        return;
    }
    cout << "New password: ";
    string newp;
    cin >> newp;
    user.setPassword(newp);
    db.saveUsers();
    cout << "Password successfully changed.\n";
}

// Update personal info
void updatePersonalInfo(User &user) {
    db.loadUsers();
    cout << "Sending OTP for update...\n";
    string code = OTPService::generateOTP(6);
    cout << "OTP: " << code << "\nEnter OTP: ";
    string in;
    cin >> in;
    if (!OTPService::verifyOTP(code, in)) {
        cout << "Invalid OTP.\n";
        return;
    }
    cout << "New full name: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string name;
    getline(cin, name);
    user.full_name = name;
    db.saveUsers();
    cout << "Personal information updated.\n";
}

// View own wallet
void viewWallet(const User &user) {
    db.loadWallets();
    const Wallet &w = db.wallets.at(user.wallet_id);
    cout << "Wallet ID: " << w.id << " | Balance: " << w.balance << "\nHistory:\n";

    // Now read and filter transaction.db for this wallet
    ifstream logf("transaction.db");
    if (logf) {
        string line;
        while (getline(logf, line)) {
            // Only show lines matching "Wallet <id>:"
            string match = "Wallet " + to_string(w.id) + ":";
            if (line.find(match) != string::npos) {
                cout << " - " << line << '\n';
            }
        }
    }

    // Show in-memory history first (if needed)
    // for (const auto &e : w.history) {
    //     cout << " - [Currently] " << "Wallet " << w.id << ": " << e << "\n";
    // }
}

// Admin: view central wallet
void viewCentralWallet(const User &user) {
    if (!user.is_admin) {
        cout << "Access denied. Admins only.\n";
        return;
    }
    const Wallet &w = db.wallets.at(0);
    cout << "Central Wallet Balance: " << w.balance << "\n";
}

// Admin: top-up user wallet
void topUpWallet(const User &user) {
    if (!user.is_admin) {
        cout << "Access denied. Admins only.\n";
        return;
    }
    db.loadWallets();
    Wallet &central = db.wallets.at(0);
    cout << "Central balance before: " << central.balance << "\n";
    cout << "Enter user wallet ID: ";
    int wid;
    cin >> wid;
    if (wid == 0 || !db.wallets.count(wid)) {
        cout << "Invalid wallet ID.\n";
        return;
    }
    cout << "Amount to top-up: ";
    long long amt;
    cin >> amt;
    if (central.balance < amt) {
        cout << "Insufficient central balance.\n";
        return;
    }
    central.balance -= amt;
    Wallet &target = db.wallets.at(wid);
    target.balance += amt;
    central.log("Debited " + to_string(amt) + " to wallet " + to_string(wid));
    target.log("Received " + to_string(amt) + " from central");
    cout << "Top-up successful. Remaining central balance: " << central.balance << "\n";
}

// Transfer between user wallets
void transferPoints(User &user) {
    db.loadWallets();
    Wallet &src = db.wallets.at(user.wallet_id);
    cout << "Enter destination wallet ID: ";
    int dest_id;
    cin >> dest_id;
    if (!db.wallets.count(dest_id)) {
        cout << "Destination wallet not found.\n";
        return;
    }
    cout << "Amount: ";
    long long amount;
    cin >> amount;
    cout << "Send OTP for transaction...\n";
    string code = OTPService::generateOTP(6);
    cout << "OTP: " << code << "\nEnter OTP: ";
    string in;
    cin >> in;
    if (!OTPService::verifyOTP(code, in)) {
        cout << "Invalid OTP.\n";
        return;
    }
    if (src.balance < amount) {
        cout << "Insufficient balance.\n";
        return;
    }
    Wallet &dest = db.wallets.at(dest_id);
    src.balance -= amount;
    dest.balance += amount;
    src.log("Sent " + to_string(amount) + " to " + to_string(dest_id));
    dest.log("Received " + to_string(amount) + " from " + to_string(src.id));
    db.saveWallets();
    cout << "Transfer completed.\n";
}

// User: Request to top-up own wallet (adds to a request queue)
void userRequestTopUp(User &user) {
    cout << "Enter amount to request top-up: ";
    long long amt;
    cin >> amt;

    // Input validation: non-numeric input
    if (cin.fail()) {
        cin.clear(); // clear error flags
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard bad input
        cerr << "Invalid input must be a number.\n";
        return;
    }

    if (amt <= 0) {
        cerr << "Invalid Amount must be greater than 0.\n";
        return;
    }

    // Generate a unique request ID
    string requestID;
    requestID = OTPService::generateOTP(8);

    // Simulate saving request to "top-up requests database"
    ofstream req("topup_requests.db", ios::app);
    if (req) {
        time_t now = time(nullptr);
        req << requestID << " " << user.wallet_id << " " << amt << " " << now << "\n";
        cout << "Top-up request submitted. Please wait for admin approval.\n";
    } else {
        cerr << "Failed to save top-up request.\n";
    }
}

void adminApproveTopUps() {
    struct Request {
        string request_id;
        int wallet_id;
        long long amount;
        time_t timestamp;
    };

    vector<Request> allRequests;

    // Load all requests
    ifstream req("topup_requests.db");
    string line;
    while (getline(req, line)) {
        istringstream iss(line);
        string request_id;
        int wallet_id;
        long long amt;
        time_t t;
        if (iss >> request_id >> wallet_id >> amt >> t) {
            allRequests.push_back({request_id, wallet_id, amt, t});
        }
    }
    req.close();

    // Display all requests
    cout << "Pending Top-Up Requests:\n";
    for (const auto& r : allRequests) {
        cout << "RequestID: " << r.request_id
             << " | Wallet ID: " << r.wallet_id
             << " | Amount: " << r.amount
             << " | Requested at: " << ctime(&r.timestamp);
    }

    cout << "\nApprove by:\n1. Wallet ID\n2. Request ID\nChoose: ";
    int choice;
    cin >> choice;

    string selectedRequestID = "";
    int selectedWalletID = -1;

    if (choice == 1) {
        cout << "Enter Wallet ID: ";
        cin >> selectedWalletID;
    } else if (choice == 2) {
        cout << "Enter Request ID: ";
        cin >> selectedRequestID;
    } else {
        cerr << "Invalid choice.\n";
        return;
    }

    db.loadWallets();
    Wallet &central = db.wallets.at(0);
    ofstream temp("topup_requests_temp.db");

    vector<Request> approved;

    // Simulate approval decisions
    for (const auto& r : allRequests) {
        bool shouldApprove = false;

        if (choice == 1 && r.wallet_id == selectedWalletID)
            shouldApprove = true;
        else if (choice == 2 && r.request_id == selectedRequestID)
            shouldApprove = true;

        if (shouldApprove) {
            if (!db.wallets.count(r.wallet_id)) {
                cerr << "Wallet ID " << r.wallet_id << " not found. Request skipped.\n";
                temp << r.request_id << " " << r.wallet_id << " " << r.amount << " " << r.timestamp << "\n";
                continue;
            }
            if (central.balance < r.amount) {
                cerr << "Insufficient central balance for wallet " << r.wallet_id << ". Request kept pending.\n";
                temp << r.request_id << " " << r.wallet_id << " " << r.amount << " " << r.timestamp << "\n";
                continue;
            }
            approved.push_back(r);
        } else {
            temp << r.request_id << " " << r.wallet_id << " " << r.amount << " " << r.timestamp << "\n";
        }
    }

    // Apply changes for approved ones
    for (const auto& r : approved) {
        Wallet &target = db.wallets.at(r.wallet_id);
        central.balance -= r.amount;
        target.balance += r.amount;

        central.log("Debited " + to_string(r.amount) + " to wallet " + to_string(r.wallet_id));
        target.log("Received " + to_string(r.amount) + " from central");

        cout << "Approved top-up of " << r.amount << " to wallet " << r.wallet_id << ".\n";
    }

    db.saveWallets();

    temp.close();
    remove("topup_requests.db");
    rename("topup_requests_temp.db", "topup_requests.db");
}

// Menu for regular users
void userMenu(User &user) {
    while (true) {
        cout << "\nUser Menu (" << user.username << "):\n"
             << "1. View Info\n"
             << "2. Change Password\n"
             << "3. Update Info\n"
             << "4. View Wallet\n"
             << "5. Transfer Points\n"
             << "6. Top-up from Central\n"
             << "7. Notify update\n"
             << "8. Logout\n"
             << "Choice: ";
        int choice;
        cin >> choice;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid selection.\n";
            continue;
        }

        switch (choice) {
            case 1:
                db.loadUsers();
                cout << "Username: " << user.username << " | Name: " << user.full_name << "\n";
                break;
            case 2:
                changePassword(user);
                break;
            case 3:
                updatePersonalInfo(user);
                break;
            case 4:
                viewWallet(user);
                break;
            case 5:
                transferPoints(user);
                break;
            case 6:
                userRequestTopUp(user);
                break;
            case 7: {
                db.loadUsers();
                cout << "Pending User Update Requests:\n";

                // Đọc và hiển thị danh sách yêu cầu
                ifstream fin("admin_update_requests.db");
                if (!fin) {
                    cerr << "No pending update file found.\n";
                    break;
                }

                string line;
                vector<string> availableOtps;
                vector<string> requestLines;

                while (getline(fin, line)) {
                    stringstream ss(line);
                    string otp, username, fullname;
                    getline(ss, otp, '|');
                    getline(ss, username, '|');
                    getline(ss, fullname);

                    if (username == user.username) {
                        cout << "Username: " << username << "\n";
                        cout << "New Fullname: " << fullname << "\n";
                        cout << "OTP: " << otp << "\n";
                        cout << "--------------------------\n";
                    }

                    availableOtps.push_back(otp);
                    requestLines.push_back(line);
                }
                fin.close();

                if (availableOtps.empty()) {
                    cout << "No pending requests found.\n";
                    break;
                }

                cout << "Enter OTP to confirm user update: ";
                string otp_input;
                cin >> otp_input;

                // Duyệt lại các dòng đã đọc, xử lý cập nhật và ghi ra file tạm
                ofstream fout("temp_admin_update_requests.db");
                bool found = false;

                for (const string &entry : requestLines) {
                    stringstream ss(entry);
                    string otp, username, fullname;
                    getline(ss, otp, '|');
                    getline(ss, username, '|');
                    getline(ss, fullname);

                    if (otp == otp_input && username == user.username) {
                        db.loadUsers();
                        auto user_it = db.users.find(username);
                        if (user_it != db.users.end()) {
                            user_it->second.full_name = fullname;
                            db.saveUsers();
                            cout << "Updated successfully for user'" << username << "'.\n";
                        } else {
                            cout << "User not found in database.\n";
                        }
                        found = true;
                        // Không ghi lại dòng này nữa => tức là xoá khỏi file
                    } else {
                        fout << entry << "\n"; // Giữ lại dòng chưa xử lý
                    }
                }

                fout.close();

                // Ghi đè file cũ
                remove("admin_update_requests.db");
                rename("temp_admin_update_requests.db", "admin_update_requests.db");

                if (!found) {
                    cout << "Invalid or expired OTP.\n";
                }

                break;
            }
            case 8:
                return;
            default:
                cout << "Invalid selection.\n";
        }
    }
}

// Menu for admin users
void adminMenu(User &user) {
    while (true) {
        cout << "\nAdmin Menu (" << user.username << "):\n"
             << "1. List Users\n"
             << "2. Create User\n"
             << "3. Modify User Info\n"
             << "4. View Central Wallet\n"
             << "5. Top-up User Wallet\n"
             << "6. Approve Top-up\n"
             << "7. Logout\n"
             << "Choice: ";
        int choice;
        cin >> choice;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid selection.\n";
            continue;
        }

        switch (choice) {
            case 1:
                for (const auto &p : db.users) {
                    cout << p.first << " (" << (p.second.is_admin ? "Admin" : "User") << ")\n";
                }
                break;
            case 2:
                registerUser(false);
                break;
            case 3: {
                cout << "Enter username to modify: ";
                string uname;
                cin >> uname;
                auto it = db.users.find(uname);
                if (it == db.users.end()) {
                    cout << "User not found.\n";
                    break;
                }

                cout << "Enter new fullname: ";
                string new_fullname;
                cin.ignore();
                getline(cin, new_fullname);

                string otp = OTPService::generateOTP();
                PendingUpdate pending{uname, new_fullname, otp};

                // Ghi vào file
                ofstream req("admin_update_requests.db", ios::app);
                if (req) {
                    req << pending.otp << "|" << pending.username << "|" << pending.fullname << "\n";
                    cout << "OTP " << otp << " has been generated and sent to the user.\n";
                } else {
                    cerr << "Failed to write pending update to file.\n";
                }
                break;
            }
            case 4:
                viewCentralWallet(user);
                break;
            case 5:
                topUpWallet(user);
                break;
            case 6:
                adminApproveTopUps();
                break;
            case 7:
                return;
            default:
                cout << "Invalid selection.\n";
        }
    }
}

int main() {
    while (true) {
        cout << "\nMain Menu:\n"
             << "1. Register User\n"
             << "2. Register Admin\n"
             << "3. Login\n"
             << "4. Exit\n"
             << "Choice: ";
        int choice;
        cin >> choice;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid selection.\n";
            continue;
        }

        switch (choice) {
            case 1:
                registerUser(false);
                break;
            case 2:
                registerUser(true);
                break;
            case 3: {
                User *u = login();
                if (u) {
                    if (u->is_admin) adminMenu(*u);
                    else userMenu(*u);
                }
                break;
            }
            case 4:
                return 0;
            default:
                cout << "Invalid selection.\n";
        }
    }
    return 0;
}
