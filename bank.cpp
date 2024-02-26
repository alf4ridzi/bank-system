/* 
BANK SYSTEM - ALF4RIDZI 
This ah ugly code its my first C++ Project.
*/

#include <iostream>
#include <openssl/md5.h> // use openssl 3.0
#include <string>
#include <fstream>
#include <nlohmann/json.hpp> // use https://github.com/nlohmann/json (please install)
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <list>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <functional>
#include <filesystem>
// #include <header.cpp>

// they said using namespace std is bad....
using std::cout, std::cin, std::vector, std::endl, std::ifstream, std::string, std::cerr, std::to_string, std::exception, std::random_device;
using std::ofstream, std::ostream;
using json = nlohmann::json;


const string jsonfile = "data/profile.json";
const string jsonfilerek = "data/rekening.json";


struct Profile
{
    string em;
    string password;
};

void clear() {
    #ifdef _WIN64
    system("cls");
    #elif _WIN32
    system("cls")
    #else
    system("clear");
    #endif
}

json getJson() {
    ifstream file(jsonfile);
    if (!file.is_open()) {
        std::cerr << "Failed to get profile" << endl;
    }

    json jsonData;

    try {
        file >> jsonData;
    } catch (const json::parse_error& e) {
        std::cerr << "Error When Parsing" << endl;

    }
    file.close();

    return jsonData;

}

json getJsonRekening() {
    ifstream file(jsonfilerek);
    if (!file.is_open()) {
        cerr << "Failed to open rekening.json" << endl;
    }
    json jsonData;

    try {
        file >> jsonData;
    } catch (const json::parse_error& e) {
        cerr << "Error when parsing" << endl;
    }
    file.close();

    return jsonData;
}

string getUsername(const string& email, json data = getJson())
{
    // json data = getJson();
    // cout << profil.em << endl;
    return data[email]["username"];

}

double getMount(const string& email) {
    json data = getJson();
    return data[email]["saldo"];
}

int check_rek(const string& email) {
    json data = getJson();
    // for (auto it = data["rekening"].begin(); it != data["rekening"].end(); ++it) {
    //     string em = it.value()["user"].get<string>();
    //     if (em == email) {
    //         string rekeningNumber = it.key();
    //         return rekeningNumber;
    //     }
    // }

    // return "null";
    return data[email]["rekening_number"];

}

void create_folder(){
    
    if (!std::filesystem::exists(jsonfile)){
        ofstream {jsonfile};
    } 
    if (!std::filesystem::exists(jsonfilerek)){
        ofstream {jsonfilerek};
    }
}

string searchRekening(const int& rekening) {
    json data = getJsonRekening();
    // if (data["rekening"].find(to_string(rekening)) != data["rekening"].end()) {
    //     string email = data["rekening"][to_string(rekening)]["user"];
    //     string username = getUsername(email);
    //     return username;
    // } else {
    //     return "null";
    // }
    // return "null";
    try {
        return data["rekening"][to_string(rekening)]["user"];
    } catch(exception) {
        return "null";
    }
    return "null";
    
}

string searchRekeningEmail(const int& rekening) {
    json data = getJsonRekening();
    if (data["rekening"].find(to_string(rekening)) != data["rekening"].end()) {
        // string email = data["rekening"][to_string(rekening)]["user"];
        // return email;
        return data["rekening"][to_string(rekening)]["user"];
    } else {
        return "null";
    }
}

string emailtorekening(const string& email, json data = getJsonRekening()) {
    for (auto it = data["rekening"].begin(); it != data["rekening"].end(); ++it) {
        string em = it.value()["user"].get<string>();
        if (em == email) {
            string rekeningNumber = it.key();
            return rekeningNumber;
        }
    }

    return "null";
}

bool saveTranscationHistory(const string& email, const string& emailTujuan, const json& dataTransaction, const json& dataTransactionTujuan, json data = getJson()) {
    try {
        data[email]["transaction_history"].push_back(dataTransaction);
        if (dataTransactionTujuan.empty() == false){
            data[emailTujuan]["transaction_history"].push_back(dataTransactionTujuan);
        }
        
        ofstream file(jsonfile);
        file << data.dump(4);
        //file << data.dump(4);
        file.close();
        return true;
    } catch (exception& e) {
        return false;
    }
    return false;
}

int getPin(const string& email) {
    json data = getJson();
    return data[email]["pin"];
}

long long createUniqeID(){
    auto currentTime = std::chrono::system_clock::now().time_since_epoch().count();
    random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9999);
    auto randomNumber = dis(gen);
    string combined = to_string(currentTime) + to_string(randomNumber);
    std::hash<string> hasher;
    size_t hashedValue = hasher(combined);
    return hashedValue;
}

string gettimestamp() {
    auto currentTime = std::chrono::system_clock::now();
    time_t currentTIme_t = std::chrono::system_clock::to_time_t(currentTime);
    tm tm = *std::localtime(&currentTIme_t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

bool sendRequestTransfer(const string email, const string& emailTujuan, const double& mount, const string& note, json data = getJson()) {
    try {
        ofstream profil(jsonfile);
        int getmount = data[emailTujuan]["saldo"];
        int getmountowner = data[email]["saldo"];
        data[emailTujuan]["saldo"] = getmount + mount;
        // profil << data.dump(4);
        data[email]["saldo"] = getmountowner - mount;
        profil << data.dump(4);
        profil.close();
        long long id_trx = createUniqeID();
        string timestamp = gettimestamp();
        json dataTransaction = {
            {"ID", id_trx},
            {"mount", mount},
            {"tipe", "SEND"},
            {"status", "SUCCESS"},
            {"note", note},
            {"to_rekening", emailtorekening(emailTujuan)},
            {"timestamp", timestamp},
        };
        json dataTransactionTujuan = {
            {"ID", id_trx},
            {"mount", mount},
            {"tipe", "RECIEVE"},
            {"status", "SUCCESS"},
            {"note", note},
            {"from_rekening", emailtorekening(email)},
            {"timestamp", timestamp},
        };
        //bool saveTransaction = saveTranscationHistory(email, emailTujuan, dataTransaction, dataTransactionTujuan); 
        if (saveTranscationHistory(email, emailTujuan, dataTransaction, dataTransactionTujuan)){
            return true;
        } 
        return false;
    }
    catch (exception& e) {
        return false;
    }
}

// bool saveHistory(){
//     return false;
// }

bool withdraw(const string& email, const float& mount, json data = getJson()) {
    try {
        ofstream profil(jsonfile);
        float saldo = data[email]["saldo"];
        data[email]["saldo"] = saldo - mount;
        profil << data.dump(4);
        profil.close();
        return true;
    } catch (exception) {
        return false;
    }

}

bool Deposit(const string& email, const int& mount, json data = getJson()) {
    ofstream file(jsonfile);
    int mount_user = data[email]["saldo"];
    data[email]["saldo"] = mount_user + mount;
    file << data.dump(4);
    file.close();
    return true;

}

bool saveWithdrawWallet(const string& email, const json& data) {
    //ofstream file(jsonfile);
    try {
        json rek = getJson();
        // json wd = rek[email]["withdraw"];
        // // vector<json> wd = rek[email]["withdraw"];
        // wd.push_back(data);
        // rek[email]["withdraw"] = wd;
        //cout << rek << endl;
        rek[email]["withdraw"].push_back(data);
        ofstream file(jsonfile);
        file << rek.dump(4);
        file.close();
        return true;
    } catch (exception) {
        return false;
    }
    
}

void Dashboard(string email) {
    dash:
    Profile profil;
    int choose;
    json getjson = getJson();
    string username = getUsername(email);
    double getmount = getMount(email);
    std::cout << "\nWelcome " << username << endl;
    cout << "Mount : $" << getmount << endl;
    menu:
    // json getjson = getJson();
    printf("\nMenu : \n[1] Transfer\n[2] Withdraw\n[3] Deposit\n[4] Histrory Transaction\n[5] Check Rekening Number\n[6] Configuration Wallet For Withdraw \n[99] Refesh Dashboard\n\n");
    cout << "Choose : ";
    cin >> choose;
    if (choose == 1) {
        // transfer(email);
        int amount;
        int pin;
        cout << "\nTransfer" << endl;
        rek:
        int Torekening;
        cout << "Input Rekening Number : ";
        cin >> Torekening;
        string getRek = searchRekening(Torekening);
        if (getRek == "null") {
            cerr << "Rekening Not Found!. Please try again..." << endl;
            goto rek;
        } else {
            string RekeningToEmail = searchRekeningEmail(Torekening);
            cout << "Destination account to " << getRek << endl;
            mount:
            cout << "Mount to transfer : $";
            cin >> amount;
            if (getmount < amount) {
                cerr << "Mount not enough to make transfer" << endl;
                goto mount;
            }
            for (int cok = 3; cok > 0; --cok)
            {
                cout << "PIN : ";
                cin >> pin;
                if (pin != getPin(email)) {
                    cerr << "Pin Wrong.. Attempt : " << cok - 1 << endl;
                } else {
                    string note;
                    cout << "Note (opsional): ";
                    getline(cin, note);
                    if (sendRequestTransfer(email, RekeningToEmail, amount, note)) {
                        cout << "Transfer $" << amount << "to " << Torekening << "(" << searchRekening << ")" << "Success!" << endl; goto dash; break;
                    }
                    cout << "Transfer $" << amount << "to " << Torekening << "(" << searchRekening << ")" << "Failed!" << endl; goto dash; break;
                }
                cerr << "Transfer requests rejected." << endl;
            }
        }


    } else if (choose == 2)
    {
        json getjson = getJson();
        double wdmount, tax = 0.25;
        wd:
        cout << "\nWithdraw\nTax Withdraw: " << tax << endl;
        cout << "mount : ";
        cin >> wdmount;
        double total = wdmount + tax;
        cout << "Total : $" << total << endl;
        if (getmount < total) {
            cerr << "Sorry, mount not enough to make transaction." << endl;
            goto wd;
        }
        json data = getjson[email]["withdraw"];
        // cout << data << endl;
        if (data.empty()) {
            cout << "\nWallet to withdraw not found.. please configuration wallet to withdraw first!" << endl;
            goto dash;
        } else {
            int choice = 1;
            for (const auto& item : data) {
                string wallet = item["wallet"];
                long long wallet_number = item["number"];
                string wallet_name = item["wallet_name"];
                cout << choice << ". " << wallet << " - " << wallet_number << " - " << wallet_name << std::endl;
                // cout << wallet << endl;
                choice++;
                // cout << item << endl;

            }
            int chc;
            chcwallet:
            cout << "Choice : ";
            cin >> chc;
            if (chc >= 1 && chc <= data.size()) {
                string walletSelected = data[chc - 1]["wallet"];
                string walletNameSelected = data[chc - 1]["wallet_name"];
                long long walletNumberSelected = data[chc - 1]["number"];
                // cout << "Selected wallet: " << walletSelected << std::endl;
                cout << "\nWithdraw Details : \nWithdraw to : " << walletSelected << " - " << walletNameSelected << "\nRekening Number : " << walletNumberSelected << endl;
                char yn;
                cout << "Continue to withdraw ? [y/n] ";
                cin >> yn;
                if (yn != 'y') {
                    cerr << "\nWithdraw canceled..." << endl;
                    goto dash;
                } else {
                    bool sendWithdraw = withdraw(email, total);
                    if (sendWithdraw) {
                        cout << "Withdraw to " << walletNumberSelected << " (" << walletNameSelected << ") Success!. " << endl;
                        goto dash;
                    } else {
                        cerr << "Withdraw to " << walletNumberSelected << " (" << walletNameSelected << ") Failed!. " << endl;
                        goto dash;
                    }
                }

            } else {
                cerr << "Wallet not found. Please try again" << endl;
                goto chcwallet;
            }
        
        }

    } else if(choose == 3) {
        int jumlah;
        depo:
        cout << "Mount To Deposit (min 5$) : ";
        cin >> jumlah;
        if (jumlah < 5) {
            cerr << "Minimum Deposit is $5" << endl;
            goto depo;
        }
        bool deposit = Deposit(email, jumlah);
        if (deposit) {
            cout << "\nDeposit to your account success!!.." << endl;
            goto dash;
        } else {
            cerr << "\nUnknown Error When Deposit" << endl;
            goto dash;
        }
    } else if(choose == 5) {
        int rek = check_rek(email);
        cout << "\nRekening Number : " << rek << endl;
        goto dash;
    } else if(choose == 6) {
        wallet:
        string wallet_type;
        string wallet_name;
        long long wallet_number;
        cout << "\nConfiguration Wallet" << endl;
        cout << "Wallet Type (bank,wallet): ";
        cin >> wallet_type;
        if (wallet_type != "bank" && wallet_type != "wallet") {
            cerr << "Please choose bank/wallet.." << endl;
            goto wallet;
        }
        vector<std::string> banks = {"BCA", "BRI", "Mandiri", "Danamon", "CIMB Niaga", "Permata", "Panin", "Maybank", "OCBC NISP", "DBS"};
        vector<std::string> wallets = {"OVO","GoPay","DANA","LinkAja","ShopeePay","TCash","Doku Wallet","Jenius","iSaku","Paytren"};
        // Anda dapat menambahkan lebih banyak dompet digital jika diperlukan};
        if (wallet_type == "bank") {
            bank_choice:
            for (int i = 0; i < banks.size(); ++i) {
                cout << i + 1 << ". " << banks[i] << endl;
            }
            int bank_choice;
            cout << "Select bank : ";
            cin >> bank_choice;
            if (bank_choice >= 1 && bank_choice <= banks.size()) {
                // cout << "You choose : " << banks[bank_choice - 1] << endl;
                cout << "Bank Number : ";
                cin >> wallet_number;
                cout << "Bank Name : ";
                cin.ignore();
                getline(cin, wallet_name);
                json newData = {
                    {"wallet", wallet_type},
                    {"bank_name", banks[bank_choice - 1]},
                    {"number", wallet_number},
                    {"wallet_name", wallet_name},
                };
                bool wd = saveWithdrawWallet(email, newData);
                if (wd) {
                    cout << "\nWallet withdraw successfully saved...." << endl;
                    goto menu;
                } else {
                    cerr << "\nUnknown Error When Save Withdraw Wallet" << endl;
                    goto menu;
                }
            } else {
                cerr << "Pilihan bank tidak ada" << endl;
                goto bank_choice;
            } 
        } else if(wallet_type == "wallet"){
            wallet_choice:
            for (int i = 0; i < wallets.size(); i++) {
                cout << i+1 << ". " << wallets[i] << endl;
            }
            int wallet_ch;
            cout << "Select Wallet : ";
            cin >> wallet_ch;
            if (wallet_ch >= 1 && wallet_ch <= wallets.size()) {
                cout << "Wallet Number : ";
                cin >> wallet_number;
                cout << "Wallet Name : ";
                // cin >> wallet_name;
                cin.ignore();
                getline(cin, wallet_name);
                json NewData = {
                    {"wallet", wallet_type},
                    {"bank_name", wallets[wallet_ch - 1]},
                    {"number", wallet_number},
                    {"wallet_name", wallet_name},
                };
                bool wd = saveWithdrawWallet(email, NewData);
                if (wd) {
                    cout << "\nWallet withdraw successfully saved...." << endl;
                    goto menu;
                } else {
                    cerr << "\nUnknown Error When Save Withdraw Wallet" << endl;
                    goto menu;
                } 

            } else {
                cerr << "Pilihan wallet tidak ada." << endl;
                goto wallet_choice;
            }
        }   
    } else if (choose == 4) {
        json data = getJson();
        json transaction_conf = data[email]["transaction_history"];
        if (transaction_conf.empty()) {
            cout << "no transactions have been made yet." << endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            goto dash;
        }
        json getjson = getJson();
        cout << "\nTransaction History" << endl;
        string rekening, tipe_rek;
        for (const auto& rek_history : getjson[email]["transaction_history"]) {
            long long id = rek_history["ID"];
            string date_trx = rek_history["timestamp"];
            long mount = rek_history["mount"];
            string status = rek_history["status"];
            string tipe = rek_history["tipe"];
            if (rek_history.contains("to_rekening")) {
                string to_rekening = rek_history["to_rekening"];
                rekening = "To Rekening  : " + to_rekening;
                //tipe_rek = "To Rekening"
            } else {
                string from_rekening = rek_history["from_rekening"];
                rekening = "From Rekening : " + from_rekening;
            }
            //string from_rekening = rek_history["from_rekening"];
            string note = rek_history["note"];

            
            cout << "ID            : " << id << endl;
            std::istringstream iss(date_trx);
            string date, sec;
            iss >> date >> sec;
            cout << "Date          : " << date_trx << endl;
            cout << "Mount         : " << mount << endl;
            cout << "Status        : " << status << endl;
            cout << "Tipe          : " << tipe << endl;
            cout << rekening << endl;
            
            
            cout << "Note          : " << note << "\n" << endl;
        }
        cin.ignore();
        cout << "\nPress enter to back main menu" << endl;
        getchar();
        goto dash;
        

    } else if(choose == 99) {
        goto dash;
    } else {
        cerr << "Menu not valid. Please try again..";
        goto dash;
    }

    // printf("\nWelcome %s\n", username);
}


string tekstomd5(const string& text) {
    // MD5_CTX md5Context;
    // MD5_Init(&md5Context);

    // // Add text data to the MD5 context
    // MD5_Update(&md5Context, text.c_str(), text.length());

    // // Generate MD5 hash
    // unsigned char md5Hash[MD5_DIGEST_LENGTH];
    // MD5_Final(md5Hash, &md5Context);

    // // Convert MD5 hash to a string
    // char md5String[33];
    // for (int i = 0; i < 16; ++i) {
    //     sprintf(&md5String[i * 2], "%02x", (unsigned int)md5Hash[i]);
    // }
    // md5String[32] = '\0';

    // return string(md5String);

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_md5();
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;

    EVP_DigestInit(mdctx, md);
    EVP_DigestUpdate(mdctx, text.c_str(), text.size());
    EVP_DigestFinal(mdctx, md_value, &md_len);
    EVP_MD_CTX_free(mdctx);

    std::stringstream ss;
    for (unsigned int i = 0; i < md_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(md_value[i]);
    }

    return ss.str();
}

string getPassword(const string& email, const json& data) {
    // Profile profil;
    // json getjson = getJson();
    // string password = getjson[profil.em]["password"];
    // return password;
    return data[email]["password"];

}

bool checkblock(const string&email, const json& data) {
    return data[email]["block"];
}

bool cekuser(const string& email, const json& data) {
    // Profile profil;
    // json getjson = getJson();
    // std::cout << getjson << endl;
    // if (getjson.contains(profil.em)) {
    //     return true;
    // } else {
    //     return false;
    // }
    return data.find(email) != data.end();
}

bool getRekening(const int& rekening) {
    json getjson = getJsonRekening();
    string rek = to_string(rekening);
    // auto it = getjson.find(rekening);
    if (getjson.contains(rek)) {
        return true;
    } else {
        return false;
    }
    return false;
}

void Log() {
    clear();
    Profile profil;
    // string username;
    // string password = "62fcca6b01a28a193261a26c0df24a60";
    // string email = "jancok@gmail.com";
    // int attempt = 3;
    printf("\nLogin To Mini Bank!\n\n");
    for(int attempt = 3; attempt > 0; --attempt) {
        std::cout << "Email    : ";
        std::cin >> profil.em;
        // std::cout << profil.em << endl;
        std::cout << "Password : ";
        std::cin >> profil.password;
        string pwmd5 = tekstomd5(profil.password);
        // std::cout << pwmd5 << endl;
        // bool cekemail = cekuser();
        json getjson = getJson();
        // std::cout << cekemail << endl;
        if (cekuser(profil.em, getjson)) {
            string password = getPassword(profil.em, getjson);
            if (pwmd5 == password) {
                if (checkblock(profil.em, getjson)) {
                    cerr << "Account has been disabled." << endl;
                    break;
                }
                else {
                    clear();
                    Dashboard(profil.em);
                    return;
                }
            }
            else {
                std::cerr << "Email or Password Not Match. Please try again ... Attemp : "  << attempt - 1 << endl;
            }
        } else {
            std::cerr << "Email or Password Not Match. Please try again ... Attemp : "  << attempt - 1 << endl;
        }
    }
    std::cerr << "Login Failed. Please try another time" << endl;

}

bool writeNewUser(const string& email, const string&password, const int& pin, const int& rek, const string& username) {
    json data = getJson();
    json rekjson = getJsonRekening();
    json newUser = {
        {"saldo", 0},
        {"username", username},
        {"block", false},
        {"pin", pin},
        {"password", password},
        {"rekening_number", rek},
        {"withdraw", json::array()},
        {"transaction_history", json::array()},
    };
    json newRek = {
        {"user", email},
    };
    string rekstr = to_string(rek);
    data[email] = newUser;
    rekjson["rekening"][rekstr] = newRek;
    ofstream outputFile(jsonfile);
    ofstream outputFileRek(jsonfilerek);
    if (outputFile.is_open() && outputFileRek.is_open()) {
        outputFile << data.dump(4);
        outputFile.close();
        outputFileRek << rekjson.dump(1);
        outputFileRek.close();
    } else {
        return false;
    }
    return true;
}

int genRandomRek() {
    srand(static_cast<unsigned>(time(nullptr)));

    int randomNum = rand();
    return randomNum;
}

int Regis(){
    Profile profil;
    string repeat;
    int pin;
    int pinrepeat;
    json getjson = getJson();
    email:
    std::cout << "Register" << endl;
    std::cout << "Email : ";
    std::cin >> profil.em;
    if (bool geta = profil.em.find("@") == string::npos) {
        cerr << "Please type correct email!." << endl;
        goto email;
    }
    ulang:
    while(true) {
        if(cekuser(profil.em, getjson)) {
            std::cerr << "Error Email already registered.. please use another email." << endl;
            break;
        } else {
            string username;
            user:
            cout << "Username : ";
            cin >> username;
            for (char c : username) {
                if(isupper(c) || isspace(c)) {
                    cerr << "Please dont user uppercase or space in username.. try again" << endl;
                    goto user;
                }
            }
            input_pw:
            std::cout << "Password : ";
            std::cin >> profil.password;
            if (profil.password.length() < 5) {
                cerr << "Password Atleast 5 Character" << endl;
                goto input_pw;
            }
            std::cout << "Type again : ";
            std::cin >> repeat;
            if (profil.password == repeat) {
                string pwhash = tekstomd5(profil.password);
                input_pin:
                std::cout << "PIN : ";
                std::cin >> pin;
                string jmlhpin = to_string(pin);
                if (int jumlahpin = jmlhpin.length() != 4) {
                    cerr << "PIN Must 4 digit" << endl;
                    goto input_pin;
                }
                std::cout << "Type PIN again : ";
                std::cin >> pinrepeat;
                if (pin == pinrepeat) {
                    int genRek = genRandomRek();
                    bool wuser = writeNewUser(profil.em, pwhash, pin, genRek, username);
                    if (wuser) {
                        std::cout << "Register Success, Please Login.\n";
                        Log();
                        break;

                    } else {
                        cerr << "Register Error. Unknown Error";
                        break;
                    }

                } else {
                    std::cerr << "PIN Dont Match. Please try again..." << endl;
                    goto input_pin;
                }
            }
            else {
                std::cerr << "Password Dont Match. Please try again..." << endl;
                goto input_pw;
            }
        }
    }
    // std::cerr << "Register Failed" << endl;
    // exit(0);
    return 1;

}

int main() {
    clear();
    create_folder();
    int select;
    printf("Welcome To Mini Bank!\n\n[1] Login\n[2] Register\n[3] Exit\n\n");
    select:
    std::cout << "Select : ";
    std::cin >> select;
    if (select == 1) {
        Log();
    }
    else if (select == 2){
        Regis();
    }
    else if (select == 3){
        std::cout << "Bye Bye... (Nino Sound)" << endl;
        return 0;
    }
    else {
        std::cerr << "Please select... 1/2/3" << endl;
        goto select;
    }
    return 0;

}
