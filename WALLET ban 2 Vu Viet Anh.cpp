#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <functional>

using namespace std;

// Simple OTP service
class OTPService {
public:
    static string generateOTP() {
        srand((unsigned)time(nullptr));
        int code = rand() % 900000 + 100000; // 6-digit
        return to_string(code);
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

    User() {}
    User(string u, const string &pwd, const string &name, bool admin, int wid)
        : username(u), password_hash(hash<string>()(pwd)), full_name(name), is_admin(admin), wallet_id(wid) {}

    bool checkPassword(const string &pwd) const {
        return hash<string>()(pwd) == password_hash;
    }
    void setPassword(const string &pwd) {
        password_hash = hash<string>()(pwd);
    }
};

// Wallet class
class Wallet {
public:
    int id;
    long long balance;
    vector<string> history;

    Wallet() {}
    Wallet(int _id) : id(_id), balance(0) {}
    void log(const string &entry) {
        history.push_back(entry);
    }
};

// Simple flat-file database
class Database {
public:
    unordered_map<string, User> users;
    unordered_map<int, Wallet> wallets;
    int next_wallet_id;

    Database() : next_wallet_id(1) {
        load();
    }
    ~Database() {
        save();
    }

    void load() {
        loadUsers();
        loadWallets();
    }
    void save() {
        saveUsers();
        saveWallets();
    }

private:
    void loadUsers() {
        ifstream ifs("users.db");
        if (!ifs) return;
        string line;
        while (getline(ifs, line)) {
            istringstream ss(line);
            User u;
            ss >> u.username >> u.password_hash >> u.full_name >> u.is_admin >> u.wallet_id;
            users[u.username] = u;
            next_wallet_id = max(next_wallet_id, u.wallet_id + 1);
        }
    }
    void loadWallets() {
        ifstream ifs("wallets.db");
        if (!ifs) return;
        string line;
        while (getline(ifs, line)) {
            istringstream ss(line);
            Wallet w;
            ss >> w.id >> w.balance;
            wallets[w.id] = w;
        }
    }
    void saveUsers() {
        ofstream ofs("users.db", ios::trunc);
        for (auto &p : users) {
            User &u = p.second;
            ofs << u.username << " " << u.password_hash << " " << u.full_name << " "
                << u.is_admin << " " << u.wallet_id << "\n";
        }
    }
    void saveWallets() {
        ofstream ofs("wallets.db", ios::trunc);
        for (auto &p : wallets) {
            Wallet &w = p.second;
            ofs << w.id << " " << w.balance << "\n";
        }
    }
};

// Global DB instance
Database db;

User* login() {
    cout << "Username: "; string u;
    cin >> u;
    cout << "Password: "; string p;
    cin >> p;
    auto it = db.users.find(u);
    if (it != db.users.end() && it->second.checkPassword(p)) {
        return &it->second;
    }
    cout << "Invalid login.\n";
    return nullptr;
}

void registerUser(bool asAdmin = false) {
    cout << "Enter username: "; string u; cin >> u;
    if (db.users.count(u)) {
        cout << "Username exists.\n"; return;
    }
    string pwd;
    cout << "Enter password (leave blank to auto-generate): ";
    cin.ignore(); getline(cin, pwd);
    if (pwd.empty()) {
        pwd = OTPService::generateOTP();
        cout << "Generated password: " << pwd << endl;
    }
    cout << "Full name: "; string name; getline(cin, name);
    int wid = db.next_wallet_id++;
    User nu(u, pwd, name, asAdmin, wid);
    db.users[u] = nu;
    db.wallets[wid] = Wallet(wid);
    cout << "User " << u << " created with wallet ID " << wid << ".\n";
}

void changePassword(User &user) {
    cout << "Old password: "; string oldp; cin >> oldp;
    if (!user.checkPassword(oldp)) {
        cout << "Incorrect.\n"; return;
    }
    cout << "New password: "; string np; cin >> np;
    user.setPassword(np);
    cout << "Password updated.\n";
}

void updatePersonalInfo(User &user) {
    cout << "Requesting OTP for update...\n";
    string code = OTPService::generateOTP();
    cout << "OTP sent: " << code << "\n";
    cout << "Enter OTP: "; string in; cin >> in;
    if (!OTPService::verifyOTP(code, in)) {
        cout << "Invalid OTP.\n"; return;
    }
    cout << "New full name: "; string name; cin.ignore(); getline(cin, name);
    user.full_name = name;
    cout << "Info updated.\n";
}

void viewWallet(const User &user) {
    Wallet &w = db.wallets[user.wallet_id];
    cout << "Wallet ID: " << w.id << " Balance: " << w.balance << "\n";
    cout << "History:\n";
    for (auto &e : w.history) cout << " - " << e << "\n";
}

void transferPoints(User &user) {
    Wallet &A = db.wallets[user.wallet_id];
    cout << "Enter target wallet ID: ";
    int bid;
    cin >> bid;
    if (!db.wallets.count(bid)) {
            cout << "Wallet not found.\n";
            return;
    }
    cout << "Amount: "; long long d; cin >> d;
    cout << "OTP for transaction...\n";
    string code = OTPService::generateOTP(); cout << "OTP sent: " << code << "\n";
    cout << "Enter OTP: "; string in; cin >> in;
    if (!OTPService::verifyOTP(code, in)) {
            cout << "Invalid OTP.\n";
            return;
    }
    // Atomic
    if (A.balance < d) {
        cout << "Low balance.\n"; return;
    }
    Wallet &B = db.wallets[bid];
    A.balance -= d;
    B.balance += d;
    ostringstream recA, recB;
    recA << "Sent " << d << " to " << bid;
    recB << "Received " << d << " from " << A.id;
    A.log(recA.str()); B.log(recB.str());
    cout << "Transfer complete.\n";
}

void userMenu(User &user) {
    while (true) {
        cout << "\nUser Menu (" << user.username << "):\n";
        cout << "1. View Info\n2. Change Password\n3. Update Personal Info\n";
        cout << "4. View Wallet\n5. Transfer Points\n6. Logout\nChoice: ";
        int c; cin >> c;
        switch (c) {
            case 1: cout << "Username: " << user.username << "\nName: " << user.full_name << "\n"; break;
            case 2: changePassword(user); break;
            case 3: updatePersonalInfo(user); break;
            case 4: viewWallet(user); break;
            case 5: transferPoints(user); break;
            case 6: return;
            default: cout << "Invalid.\n";
        }
    }
}

void adminMenu(User &user) {
    while (true) {
        cout << "\nAdmin Menu (" << user.username << "):\n";
        cout << "1. List Users\n2. Create User\n3. Modify User\n4. Logout\nChoice: ";
        int c; cin >> c;
        switch (c) {
            case 1:
                for (auto &p : db.users) cout << p.first << " (" << (p.second.is_admin?"Admin":"User") << ")\n";
                break;
            case 2: registerUser(false); break;
            case 3: {
                cout << "Username to modify: "; string u; cin >> u;
                auto it = db.users.find(u);
                if (it == db.users.end()) {
                        cout << "No such user.\n"; break;
                }
                updatePersonalInfo(it->second);
            }
            break;
            case 4: return;
            default: cout << "Invalid.\n";
        }
    }
}

int main() {
    while (true) {
        cout << "\nMain Menu:\n";
        cout << "1. Register User\n";
        cout << "2. Register Admin\n";
        cout << "3. Login\n";
        cout << "4. Exit\nChoice: ";
        int c; cin >> c;
        if (c == 1) {
            registerUser(false);
        } else if (c == 2) {
            registerUser(true);
        } else if (c == 3) {
            User *u = login();
            if (u) {
                if (u->is_admin)
                    adminMenu(*u);
                else
                    userMenu(*u);
            }
        } else if (c == 4) {
            break;
        } else {
            cout << "Invalid choice.\n";
        }
    }
    return 0;
}

