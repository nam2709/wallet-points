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
#include <iomanip>

using namespace std;

// ANSI Color codes for terminal formatting
namespace Colors {
    const string RESET = "\033[0m";
    const string BOLD = "\033[1m";
    const string RED = "\033[31m";
    const string GREEN = "\033[32m";
    const string YELLOW = "\033[33m";
    const string BLUE = "\033[34m";
    const string MAGENTA = "\033[35m";
    const string CYAN = "\033[36m";
    const string WHITE = "\033[37m";
    const string BRIGHT_RED = "\033[91m";
    const string BRIGHT_GREEN = "\033[92m";
    const string BRIGHT_YELLOW = "\033[93m";
    const string BRIGHT_BLUE = "\033[94m";
    const string BRIGHT_MAGENTA = "\033[95m";
    const string BRIGHT_CYAN = "\033[96m";
    
    // Terminal-friendly colors
    const string PRIMARY = "\033[38;5;33m";      // Blue
    const string SECONDARY = "\033[38;5;39m";    // Light Blue
    const string ACCENT = "\033[38;5;208m";      // Orange
    const string SUCCESS = "\033[38;5;46m";      // Green
    const string WARNING = "\033[38;5;214m";     // Yellow
    const string ERROR = "\033[38;5;196m";       // Red
    const string INFO = "\033[38;5;51m";         // Cyan
    const string HIGHLIGHT = "\033[38;5;226m";   // Bright Yellow
    const string MUTED = "\033[38;5;240m";       // Gray
}

// Utility functions for formatting
void printHeader(const string& title) {
    cout << Colors::PRIMARY << "+===============================================================+" << Colors::RESET << endl;
    cout << Colors::PRIMARY << "|" << Colors::RESET << Colors::BOLD << Colors::HIGHLIGHT << " " << title << Colors::RESET << endl;
    cout << Colors::PRIMARY << "+===============================================================+" << Colors::RESET << endl;
}

void printSubHeader(const string& title) {
    cout << Colors::ACCENT << "=================================================================" << Colors::RESET << endl;
    cout << Colors::BOLD << Colors::HIGHLIGHT << " " << title << Colors::RESET << endl;
    cout << Colors::ACCENT << "=================================================================" << Colors::RESET << endl;
}

void printSuccess(const string& message) {
    cout << Colors::SUCCESS << "[SUCCESS] " << message << Colors::RESET << endl;
}

void printError(const string& message) {
    cout << Colors::ERROR << "[ERROR] " << message << Colors::RESET << endl;
}

void printInfo(const string& message) {
    cout << Colors::INFO << "[INFO] " << message << Colors::RESET << endl;
}

void printWarning(const string& message) {
    cout << Colors::WARNING << "[WARNING] " << message << Colors::RESET << endl;
}

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

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
    clearScreen();
    printHeader("WALLET POINTS SYSTEM - LOGIN");
    cout << endl;
    
    db.loadUsers();
    cout << Colors::SECONDARY << "Username: " << Colors::RESET;
    string u, p;
    cin >> u;
    cout << Colors::SECONDARY << "Password: " << Colors::RESET;
    cin >> p;
    auto it = db.users.find(u);
    if (it != db.users.end() && it->second.checkPassword(p)) {
        User *user = &it->second;
        if (user->must_change_password) {
            printWarning("Temporary password detected. Please set a new password:");
            cout << Colors::BRIGHT_CYAN << "New password: " << Colors::RESET;
            cin >> p;
            user->setPassword(p);
            user->must_change_password = false;
            db.saveUsers();
            printSuccess("Password updated. Please log in again.");
            return nullptr;
        }
        printSuccess("Login successful! Welcome, " + user->full_name + "!");
        return user;
    }
    printError("Invalid credentials.");
    return nullptr;
}

// Registration
void registerUser(bool asAdmin = false) {
    clearScreen();
    printHeader("WALLET POINTS SYSTEM - REGISTRATION");
    cout << endl;
    
    db.loadUsers();
    db.loadWallets();
    cout << Colors::SECONDARY << "Enter username: " << Colors::RESET;
    string u;
    cin >> u;
    if (db.users.count(u)) {
        printError("Username already exists.");
        return;
    }
    cout << Colors::SECONDARY << "Enter password (leave blank for auto-generation): " << Colors::RESET;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string pwd;
    getline(cin, pwd);
    bool forceChange = false;
    if (pwd.empty()) {
        pwd = OTPService::generateOTP(10);
        cout << Colors::WARNING << "Generated password: " << Colors::SUCCESS << pwd << Colors::RESET << endl;
        forceChange = true;
    }
    cout << Colors::SECONDARY << "Full name: " << Colors::RESET;
    string name;
    getline(cin, name);
    int wid = db.next_wallet_id++;
    db.users[u] = User(u, pwd, name, asAdmin, wid, forceChange);
    if (!asAdmin) {
        db.wallets[wid] = Wallet(wid);
        printSuccess("User '" + u + "' created with wallet ID " + to_string(wid) + ".");
    }

    // Save to file immediately
    db.saveUsers();
    db.saveWallets();
}

// Change password
void changePassword(User &user) {
    clearScreen();
    printHeader("CHANGE PASSWORD");
    cout << endl;
    
    db.loadUsers();
    cout << Colors::BRIGHT_CYAN << "Current password: " << Colors::RESET;
    string oldp;
    cin >> oldp;
    if (!user.checkPassword(oldp)) {
        printError("Incorrect password.");
        return;
    }
    cout << Colors::BRIGHT_CYAN << "New password: " << Colors::RESET;
    string newp;
    cin >> newp;
    user.setPassword(newp);
    db.saveUsers();
    printSuccess("Password successfully changed.");
    
    cout << endl;
    cout << Colors::BRIGHT_CYAN << "Press Enter to continue..." << Colors::RESET;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Update personal info
void updatePersonalInfo(User &user) {
    clearScreen();
    printHeader("UPDATE PERSONAL INFORMATION");
    cout << endl;
    
    db.loadUsers();
    printInfo("Sending OTP for update...");
    string code = OTPService::generateOTP(6);
    cout << Colors::BRIGHT_YELLOW << "OTP: " << Colors::RESET << code << endl;
    cout << Colors::BRIGHT_CYAN << "Enter OTP: " << Colors::RESET;
    string in;
    cin >> in;
    if (!OTPService::verifyOTP(code, in)) {
        printError("Invalid OTP.");
        return;
    }
    cout << Colors::BRIGHT_CYAN << "New full name: " << Colors::RESET;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string name;
    getline(cin, name);
    user.full_name = name;
    db.saveUsers();
    printSuccess("Personal information updated successfully.");
    
    cout << endl;
    cout << Colors::BRIGHT_CYAN << "Press Enter to continue..." << Colors::RESET;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// View own wallet
void viewWallet(const User &user) {
    clearScreen();
    printHeader("WALLET INFORMATION");
    cout << endl;
    
    db.loadWallets();
    const Wallet &w = db.wallets.at(user.wallet_id);
    
    cout << Colors::BRIGHT_CYAN << "Wallet ID: " << Colors::RESET << w.id << endl;
    cout << Colors::BRIGHT_GREEN << "Balance: " << Colors::RESET << w.balance << " points" << endl;
    cout << endl;
    
    printSubHeader("TRANSACTION HISTORY");
    
    // Now read and filter transaction.db for this wallet
    ifstream logf("transaction.db");
    if (logf) {
        string line;
        int count = 0;
        while (getline(logf, line)) {
            // Only show lines matching "Wallet <id>:"
            string match = "Wallet " + to_string(w.id) + ":";
            if (line.find(match) != string::npos) {
                count++;
                cout << Colors::BRIGHT_CYAN << count << "." << Colors::RESET << " " << line << endl;
            }
        }
        if (count == 0) {
            printInfo("No transaction history found.");
        }
    } else {
        printInfo("No transaction history available.");
    }

    cout << endl;
    cout << Colors::BRIGHT_CYAN << "Press Enter to continue..." << Colors::RESET;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Admin: view central wallet
void viewCentralWallet(const User &user) {
    if (!user.is_admin) {
        printError("Access denied. Admins only.");
        return;
    }
    
    clearScreen();
    printHeader("CENTRAL WALLET");
    cout << endl;
    
    const Wallet &w = db.wallets.at(0);
    cout << Colors::BRIGHT_GREEN << "Central Wallet Balance: " << Colors::RESET << w.balance << " points" << endl;
    cout << endl;
    
    cout << Colors::BRIGHT_CYAN << "Press Enter to continue..." << Colors::RESET;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Admin: top-up user wallet
void topUpWallet(const User &user) {
    if (!user.is_admin) {
        printError("Access denied. Admins only.");
        return;
    }
    
    clearScreen();
    printHeader("TOP-UP USER WALLET");
    cout << endl;
    
    db.loadWallets();
    Wallet &central = db.wallets.at(0);
    cout << Colors::BRIGHT_GREEN << "Central balance: " << Colors::RESET << central.balance << " points" << endl;
    cout << endl;
    
    cout << Colors::BRIGHT_CYAN << "Enter user wallet ID: " << Colors::RESET;
    int wid;
    cin >> wid;
    if (wid == 0 || !db.wallets.count(wid)) {
        printError("Invalid wallet ID.");
        return;
    }
    
    cout << Colors::BRIGHT_CYAN << "Amount to top-up: " << Colors::RESET;
    long long amt;
    cin >> amt;
    if (central.balance < amt) {
        printError("Insufficient central balance.");
        return;
    }
    
    central.balance -= amt;
    Wallet &target = db.wallets.at(wid);
    target.balance += amt;
    central.log("Debited " + to_string(amt) + " to wallet " + to_string(wid));
    target.log("Received " + to_string(amt) + " from central");
    
    printSuccess("Top-up successful!");
    cout << Colors::BRIGHT_GREEN << "Remaining central balance: " << Colors::RESET << central.balance << " points" << endl;
    
    cout << endl;
    cout << Colors::BRIGHT_CYAN << "Press Enter to continue..." << Colors::RESET;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Transfer between user wallets
void transferPoints(User &user) {
    clearScreen();
    printHeader("TRANSFER POINTS");
    cout << endl;
    
    db.loadWallets();
    Wallet &src = db.wallets.at(user.wallet_id);
    cout << Colors::BRIGHT_GREEN << "Your balance: " << Colors::RESET << src.balance << " points" << endl;
    cout << endl;
    
    cout << Colors::BRIGHT_CYAN << "Enter destination wallet ID: " << Colors::RESET;
    int dest_id;
    cin >> dest_id;
    if (!db.wallets.count(dest_id)) {
        printError("Destination wallet not found.");
        return;
    }
    
    cout << Colors::BRIGHT_CYAN << "Amount: " << Colors::RESET;
    long long amount;
    cin >> amount;
    
    printInfo("Sending OTP for transaction...");
    string code = OTPService::generateOTP(6);
    cout << Colors::BRIGHT_YELLOW << "OTP: " << Colors::RESET << code << endl;
    cout << Colors::BRIGHT_CYAN << "Enter OTP: " << Colors::RESET;
    string in;
    cin >> in;
    
    if (!OTPService::verifyOTP(code, in)) {
        printError("Invalid OTP.");
        return;
    }
    
    if (src.balance < amount) {
        printError("Insufficient balance.");
        return;
    }
    
    Wallet &dest = db.wallets.at(dest_id);
    src.balance -= amount;
    dest.balance += amount;
    src.log("Sent " + to_string(amount) + " to " + to_string(dest_id));
    dest.log("Received " + to_string(amount) + " from " + to_string(src.id));
    db.saveWallets();
    
    printSuccess("Transfer completed successfully!");
    cout << Colors::BRIGHT_GREEN << "New balance: " << Colors::RESET << src.balance << " points" << endl;
    
    cout << endl;
    cout << Colors::BRIGHT_CYAN << "Press Enter to continue..." << Colors::RESET;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// User: Request to top-up own wallet (adds to a request queue)
void userRequestTopUp(User &user) {
    clearScreen();
    printHeader("REQUEST TOP-UP");
    cout << endl;
    
    cout << Colors::BRIGHT_CYAN << "Enter amount to request top-up: " << Colors::RESET;
    long long amt;
    cin >> amt;

    // Input validation: non-numeric input
    if (cin.fail()) {
        cin.clear(); // clear error flags
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard bad input
        printError("Invalid input. Amount must be a number.");
        return;
    }

    if (amt <= 0) {
        printError("Invalid amount. Must be greater than 0.");
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
        printSuccess("Top-up request submitted successfully!");
        cout << Colors::BRIGHT_CYAN << "Request ID: " << Colors::RESET << requestID << endl;
        cout << Colors::BRIGHT_CYAN << "Amount: " << Colors::RESET << amt << " points" << endl;
        printInfo("Please wait for admin approval.");
    } else {
        printError("Failed to save top-up request.");
    }
    
    cout << endl;
    cout << Colors::BRIGHT_CYAN << "Press Enter to continue..." << Colors::RESET;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

void adminApproveTopUps() {
    clearScreen();
    printHeader("APPROVE TOP-UP REQUESTS");
    cout << endl;
    
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

    if (allRequests.empty()) {
        printInfo("No pending top-up requests found.");
        cout << endl;
        cout << Colors::BRIGHT_CYAN << "Press Enter to continue..." << Colors::RESET;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
        return;
    }

    // Display all requests
    printSubHeader("PENDING TOP-UP REQUESTS");
    for (const auto& r : allRequests) {
        cout << Colors::BRIGHT_CYAN << "Request ID: " << Colors::RESET << r.request_id << endl;
        cout << Colors::BRIGHT_CYAN << "Wallet ID: " << Colors::RESET << r.wallet_id << endl;
        cout << Colors::BRIGHT_CYAN << "Amount: " << Colors::RESET << r.amount << " points" << endl;
        cout << Colors::BRIGHT_CYAN << "Requested at: " << Colors::RESET << ctime(&r.timestamp);
        cout << Colors::YELLOW << "=================================================================" << Colors::RESET << endl;
    }

    cout << endl;
    cout << Colors::BRIGHT_CYAN << "Approve by:\n";
    cout << "1. Wallet ID\n";
    cout << "2. Request ID\n";
    cout << "Choose: " << Colors::RESET;
    int choice;
    cin >> choice;

    string selectedRequestID = "";
    int selectedWalletID = -1;

    if (choice == 1) {
        cout << Colors::BRIGHT_CYAN << "Enter Wallet ID: " << Colors::RESET;
        cin >> selectedWalletID;
    } else if (choice == 2) {
        cout << Colors::BRIGHT_CYAN << "Enter Request ID: " << Colors::RESET;
        cin >> selectedRequestID;
    } else {
        printError("Invalid choice.");
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
                printWarning("Wallet ID " + to_string(r.wallet_id) + " not found. Request skipped.");
                temp << r.request_id << " " << r.wallet_id << " " << r.amount << " " << r.timestamp << "\n";
                continue;
            }
            if (central.balance < r.amount) {
                printWarning("Insufficient central balance for wallet " + to_string(r.wallet_id) + ". Request kept pending.");
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

        printSuccess("Approved top-up of " + to_string(r.amount) + " to wallet " + to_string(r.wallet_id) + ".");
    }

    db.saveWallets();

    temp.close();
    remove("topup_requests.db");
    rename("topup_requests_temp.db", "topup_requests.db");
    
    cout << endl;
    cout << Colors::BRIGHT_CYAN << "Press Enter to continue..." << Colors::RESET;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Menu for regular users
void userMenu(User &user) {
    while (true) {
        clearScreen();
        printHeader("USER DASHBOARD - " + user.username);
        cout << endl;
        
        cout << Colors::SECONDARY << "Welcome, " << Colors::SUCCESS << user.full_name << Colors::RESET << "!" << endl;
        cout << endl;
        
        cout << Colors::ACCENT << "+===============================================================+" << Colors::RESET << endl;
        cout << Colors::ACCENT << "|" << Colors::RESET << " " << Colors::BOLD << Colors::HIGHLIGHT << "USER MENU" << Colors::RESET << endl;
        cout << Colors::ACCENT << "+===============================================================+" << Colors::RESET << endl;
        cout << Colors::ACCENT << "|" << Colors::RESET << " " << Colors::SECONDARY << "1." << Colors::RESET << " View Profile Information" << endl;
        cout << Colors::ACCENT << "|" << Colors::RESET << " " << Colors::SECONDARY << "2." << Colors::RESET << " Change Password" << endl;
        cout << Colors::ACCENT << "|" << Colors::RESET << " " << Colors::SECONDARY << "3." << Colors::RESET << " Update Personal Information" << endl;
        cout << Colors::ACCENT << "|" << Colors::RESET << " " << Colors::SECONDARY << "4." << Colors::RESET << " View Wallet & Transaction History" << endl;
        cout << Colors::ACCENT << "|" << Colors::RESET << " " << Colors::SECONDARY << "5." << Colors::RESET << " Transfer Points to Another Wallet" << endl;
        cout << Colors::ACCENT << "|" << Colors::RESET << " " << Colors::SECONDARY << "6." << Colors::RESET << " Request Top-up from Central Wallet" << endl;
        cout << Colors::ACCENT << "|" << Colors::RESET << " " << Colors::SECONDARY << "7." << Colors::RESET << " Process Admin Update Notifications" << endl;
        cout << Colors::ACCENT << "|" << Colors::RESET << " " << Colors::ERROR << "8." << Colors::RESET << " Logout" << endl;
        cout << Colors::ACCENT << "+===============================================================+" << Colors::RESET << endl;
        cout << endl;
        
        cout << Colors::SECONDARY << "Enter your choice: " << Colors::RESET;
        int choice;
        cin >> choice;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            printError("Invalid selection.");
            continue;
        }

        switch (choice) {
            case 1:
                db.loadUsers();
                printSubHeader("PROFILE INFORMATION");
                cout << Colors::SECONDARY << "Username: " << Colors::RESET << user.username << endl;
                cout << Colors::SECONDARY << "Full Name: " << Colors::RESET << user.full_name << endl;
                cout << Colors::SECONDARY << "Wallet ID: " << Colors::RESET << user.wallet_id << endl;
                cout << endl;
                cout << Colors::SECONDARY << "Press Enter to continue..." << Colors::RESET;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cin.get();
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
                printSubHeader("PENDING UPDATE REQUESTS");

                // Đọc và hiển thị danh sách yêu cầu
                ifstream fin("admin_update_requests.db");
                if (!fin) {
                    printWarning("No pending update requests found.");
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
                        cout << Colors::BRIGHT_CYAN << "Username: " << Colors::RESET << username << endl;
                        cout << Colors::BRIGHT_CYAN << "New Fullname: " << Colors::RESET << fullname << endl;
                        cout << Colors::BRIGHT_CYAN << "OTP: " << Colors::BRIGHT_YELLOW << otp << Colors::RESET << endl;
                        cout << Colors::YELLOW << "=================================================================" << Colors::RESET << endl;
                    }

                    availableOtps.push_back(otp);
                    requestLines.push_back(line);
                }
                fin.close();

                if (availableOtps.empty()) {
                    printWarning("No pending requests found.");
                    break;
                }

                cout << Colors::BRIGHT_CYAN << "Enter OTP to confirm user update: " << Colors::RESET;
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
                            printSuccess("Updated successfully for user '" + username + "'.");
                        } else {
                            printError("User not found in database.");
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
                    printError("Invalid or expired OTP.");
                }

                cout << endl;
                cout << Colors::BRIGHT_CYAN << "Press Enter to continue..." << Colors::RESET;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cin.get();
                break;
            }
            case 8:
                printSuccess("Logged out successfully!");
                return;
            default:
                printError("Invalid selection.");
        }
    }
}

// Menu for admin users
void adminMenu(User &user) {
    while (true) {
        clearScreen();
        printHeader("ADMIN DASHBOARD - " + user.username);
        cout << endl;
        
        cout << Colors::SECONDARY << "Welcome, " << Colors::SUCCESS << user.full_name << Colors::RESET << "!" << endl;
        cout << endl;
        
        cout << Colors::PRIMARY << "+===============================================================+" << Colors::RESET << endl;
        cout << Colors::PRIMARY << "|" << Colors::RESET << " " << Colors::BOLD << Colors::HIGHLIGHT << "ADMIN MENU" << Colors::RESET << endl;
        cout << Colors::PRIMARY << "+===============================================================+" << Colors::RESET << endl;
        cout << Colors::PRIMARY << "|" << Colors::RESET << " " << Colors::SECONDARY << "1." << Colors::RESET << " List All Users" << endl;
        cout << Colors::PRIMARY << "|" << Colors::RESET << " " << Colors::SECONDARY << "2." << Colors::RESET << " Create New User" << endl;
        cout << Colors::PRIMARY << "|" << Colors::RESET << " " << Colors::SECONDARY << "3." << Colors::RESET << " Modify User Information" << endl;
        cout << Colors::PRIMARY << "|" << Colors::RESET << " " << Colors::SECONDARY << "4." << Colors::RESET << " View Central Wallet Balance" << endl;
        cout << Colors::PRIMARY << "|" << Colors::RESET << " " << Colors::SECONDARY << "5." << Colors::RESET << " Top-up User Wallet" << endl;
        cout << Colors::PRIMARY << "|" << Colors::RESET << " " << Colors::SECONDARY << "6." << Colors::RESET << " Approve Top-up Requests" << endl;
        cout << Colors::PRIMARY << "|" << Colors::RESET << " " << Colors::ERROR << "7." << Colors::RESET << " Logout" << endl;
        cout << Colors::PRIMARY << "+===============================================================+" << Colors::RESET << endl;
        cout << endl;
        
        cout << Colors::SECONDARY << "Enter your choice: " << Colors::RESET;
        int choice;
        cin >> choice;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            printError("Invalid selection.");
            continue;
        }

        switch (choice) {
            case 1:
                printSubHeader("ALL USERS");
                for (const auto &p : db.users) {
                    cout << Colors::SECONDARY << "Username: " << Colors::RESET << p.first;
                    cout << " | " << Colors::WARNING << "Type: " << Colors::RESET << (p.second.is_admin ? "Admin" : "User");
                    cout << " | " << Colors::SUCCESS << "Name: " << Colors::RESET << p.second.full_name << endl;
                }
                cout << endl;
                cout << Colors::SECONDARY << "Press Enter to continue..." << Colors::RESET;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cin.get();
                break;
            case 2:
                registerUser(false);
                break;
            case 3: {
                cout << Colors::BRIGHT_CYAN << "Enter username to modify: " << Colors::RESET;
                string uname;
                cin >> uname;
                auto it = db.users.find(uname);
                if (it == db.users.end()) {
                    printError("User not found.");
                    break;
                }

                cout << Colors::BRIGHT_CYAN << "Enter new fullname: " << Colors::RESET;
                string new_fullname;
                cin.ignore();
                getline(cin, new_fullname);

                string otp = OTPService::generateOTP();
                PendingUpdate pending{uname, new_fullname, otp};

                // Ghi vào file
                ofstream req("admin_update_requests.db", ios::app);
                if (req) {
                    req << pending.otp << "|" << pending.username << "|" << pending.fullname << "\n";
                    printSuccess("OTP " + otp + " has been generated and sent to the user.");
                } else {
                    printError("Failed to write pending update to file.");
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
                printSuccess("Logged out successfully!");
                return;
            default:
                printError("Invalid selection.");
        }
    }
}

int main() {
    while (true) {
        clearScreen();
        printHeader("WALLET POINTS SYSTEM");
        cout << endl;
        
        cout << Colors::SECONDARY << "Welcome to the Wallet Points Management System!" << Colors::RESET << endl;
        cout << endl;
        
        cout << Colors::PRIMARY << "+===============================================================+" << Colors::RESET << endl;
        cout << Colors::PRIMARY << "|" << Colors::RESET << " " << Colors::BOLD << Colors::HIGHLIGHT << "MAIN MENU" << Colors::RESET << endl;
        cout << Colors::PRIMARY << "+===============================================================+" << Colors::RESET << endl;
        cout << Colors::PRIMARY << "|" << Colors::RESET << " " << Colors::SECONDARY << "1." << Colors::RESET << " Register New User" << endl;
        cout << Colors::PRIMARY << "|" << Colors::RESET << " " << Colors::SECONDARY << "2." << Colors::RESET << " Register New Admin" << endl;
        cout << Colors::PRIMARY << "|" << Colors::RESET << " " << Colors::SECONDARY << "3." << Colors::RESET << " Login to System" << endl;
        cout << Colors::PRIMARY << "|" << Colors::RESET << " " << Colors::ERROR << "4." << Colors::RESET << " Exit System" << endl;
        cout << Colors::PRIMARY << "+===============================================================+" << Colors::RESET << endl;
        cout << endl;
        
        cout << Colors::SECONDARY << "Enter your choice: " << Colors::RESET;
        int choice;
        cin >> choice;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            printError("Invalid selection.");
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
                printSuccess("Thank you for using Wallet Points System!");
                return 0;
            default:
                printError("Invalid selection.");
        }
    }
    return 0;
}

