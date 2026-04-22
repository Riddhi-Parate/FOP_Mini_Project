#include <conio.h>
#include <ctype.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ACCOUNTS_FILE "accounts.csv"
#define TRANSACTIONS_FILE "transactions.csv"
#define TEMP_ACCOUNTS_FILE "accounts.tmp"
#define MAX_ACCOUNTS 500
#define MAX_TRANSACTIONS 2000
#define FIELD 128

typedef struct {
    char name[FIELD];
    char email[FIELD];
    char phone[32];
    char password[FIELD];
    char pin[32];
    char accountNumber[24];
    double balance;
} Account;

typedef struct {
    char user[FIELD];
    char timestamp[40];
    char type[64];
    double amount;
    char note[FIELD];
    double balanceAfter;
} Transaction;

static Account sessionUser;
static int isLoggedIn = 0;

void trimNewline(char *value);
void readLine(const char *prompt, char *buffer, size_t size);
void readSecretLine(const char *prompt, char *buffer, size_t size);
int readInt(const char *prompt);
double readDouble(const char *prompt);
void pauseScreen(void);
void clearScreenVisual(void);
void toLowerCopy(char *dest, const char *src, size_t size);
int equalsIgnoreCase(const char *a, const char *b);
int validateEmail(const char *email);
int validatePhone(const char *phone);
int validatePassword(const char *password);
int validatePin(const char *pin);
void encryptField(const char *plainText, char *cipherText, size_t size);
void decryptField(const char *cipherText, char *plainText, size_t size);
int isEncryptedField(const char *value);
void currentTimestamp(char *buffer, size_t size);
void generateAccountNumber(char *buffer, size_t size);
int loadAccounts(Account accounts[], int maxAccounts);
int saveAccounts(const Account accounts[], int count);
int loadTransactions(Transaction transactions[], int maxTransactions);
void appendTransaction(const char *user, const char *type, double amount, const char *note, double balanceAfter);
int findAccountIndexByName(Account accounts[], int count, const char *name);
int findAccountIndexByAccountNumber(Account accounts[], int count, const char *accountNumber);
void printWelcomeMenu(void);
void printDashboardMenu(void);
void createAccount(void);
void login(void);
void logout(void);
void depositMoney(void);
void withdrawMoney(void);
void transferMoney(void);
void checkBalance(void);
void showTransactionHistory(void);
void showAccountDetails(void);
void updateContact(void);
void changePassword(void);
void deleteAccount(void);
void ensureLegacyDetails(Account *account, Account accounts[], int totalAccounts, int index);

void trimNewline(char *value) {
    size_t len = strlen(value);
    while (len > 0 && (value[len - 1] == '\n' || value[len - 1] == '\r')) {
        value[len - 1] = '\0';
        len--;
    }
}

void readLine(const char *prompt, char *buffer, size_t size) {
    if (prompt != NULL) {
        printf("%s", prompt);
    }

    if (fgets(buffer, (int)size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }

    trimNewline(buffer);
}

void readSecretLine(const char *prompt, char *buffer, size_t size) {
    size_t len = 0;
    int ch;

    if (prompt != NULL) {
        printf("%s", prompt);
    }

    if (!_isatty(_fileno(stdin))) {
        if (fgets(buffer, (int)size, stdin) == NULL) {
            buffer[0] = '\0';
            return;
        }
        trimNewline(buffer);
        return;
    }

    while ((ch = _getch()) != '\r' && ch != '\n') {
        if ((ch == '\b' || ch == 127) && len > 0) {
            len--;
            printf("\b \b");
            continue;
        }

        if (isprint(ch) && len + 1 < size) {
            buffer[len++] = (char)ch;
            printf("*");
        }
    }

    buffer[len] = '\0';
    printf("\n");
}

int readInt(const char *prompt) {
    char buffer[64];
    char *endptr;
    long value;

    while (1) {
        readLine(prompt, buffer, sizeof(buffer));
        value = strtol(buffer, &endptr, 10);
        if (endptr != buffer && *endptr == '\0') {
            return (int)value;
        }
        printf("Invalid number. Please try again.\n");
    }
}

double readDouble(const char *prompt) {
    char buffer[64];
    char *endptr;
    double value;

    while (1) {
        readLine(prompt, buffer, sizeof(buffer));
        value = strtod(buffer, &endptr);
        if (endptr != buffer && *endptr == '\0') {
            return value;
        }
        printf("Invalid amount. Please try again.\n");
    }
}

void pauseScreen(void) {
    char buffer[8];
    readLine("\nPress Enter to continue...", buffer, sizeof(buffer));
}

void clearScreenVisual(void) {
    printf("\n\n");
}

void toLowerCopy(char *dest, const char *src, size_t size) {
    size_t i;
    for (i = 0; i + 1 < size && src[i] != '\0'; i++) {
        dest[i] = (char)tolower((unsigned char)src[i]);
    }
    dest[i] = '\0';
}

int equalsIgnoreCase(const char *a, const char *b) {
    char left[FIELD];
    char right[FIELD];
    toLowerCopy(left, a, sizeof(left));
    toLowerCopy(right, b, sizeof(right));
    return strcmp(left, right) == 0;
}

int validateEmail(const char *email) {
    const char *at = strchr(email, '@');
    const char *dot = strrchr(email, '.');
    return at != NULL && dot != NULL && at > email && dot > at + 1 && dot[1] != '\0';
}

int validatePhone(const char *phone) {
    size_t i;
    if (strlen(phone) != 10) {
        return 0;
    }

    for (i = 0; phone[i] != '\0'; i++) {
        if (!isdigit((unsigned char)phone[i])) {
            return 0;
        }
    }

    return 1;
}

int validatePassword(const char *password) {
    int hasUpper = 0;
    int hasLower = 0;
    int hasDigit = 0;
    int hasSpecial = 0;
    size_t i;

    if (strlen(password) < 8) {
        return 0;
    }

    for (i = 0; password[i] != '\0'; i++) {
        if (isupper((unsigned char)password[i])) {
            hasUpper = 1;
        } else if (islower((unsigned char)password[i])) {
            hasLower = 1;
        } else if (isdigit((unsigned char)password[i])) {
            hasDigit = 1;
        } else {
            hasSpecial = 1;
        }
    }

    return hasUpper && hasLower && hasDigit && hasSpecial;
}

int validatePin(const char *pin) {
    size_t i;
    if (strlen(pin) != 4) {
        return 0;
    }

    for (i = 0; pin[i] != '\0'; i++) {
        if (!isdigit((unsigned char)pin[i])) {
            return 0;
        }
    }

    return 1;
}

int isEncryptedField(const char *value) {
    return strncmp(value, "ENC:", 4) == 0;
}

void encryptField(const char *plainText, char *cipherText, size_t size) {
    static const unsigned char key[] = { 0x5A, 0x21, 0x43, 0x17, 0x6D };
    size_t i;
    size_t required = strlen(plainText) * 2 + 5;

    if (size < required) {
        cipherText[0] = '\0';
        return;
    }

    strcpy(cipherText, "ENC:");
    for (i = 0; plainText[i] != '\0'; i++) {
        unsigned char encoded = ((unsigned char)plainText[i]) ^ key[i % (sizeof(key) / sizeof(key[0]))];
        snprintf(cipherText + 4 + (i * 2), size - 4 - (i * 2), "%02X", encoded);
    }
}

void decryptField(const char *cipherText, char *plainText, size_t size) {
    static const unsigned char key[] = { 0x5A, 0x21, 0x43, 0x17, 0x6D };
    size_t i;
    size_t dataLen;

    if (!isEncryptedField(cipherText)) {
        snprintf(plainText, size, "%s", cipherText);
        return;
    }

    dataLen = strlen(cipherText + 4);
    for (i = 0; i + 1 < dataLen && (i / 2) + 1 < size; i += 2) {
        unsigned int byteValue = 0;
        sscanf(cipherText + 4 + i, "%2X", &byteValue);
        plainText[i / 2] = (char)(((unsigned char)byteValue) ^ key[(i / 2) % (sizeof(key) / sizeof(key[0]))]);
    }
    plainText[i / 2] = '\0';
}

void currentTimestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *timeInfo = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", timeInfo);
}

void generateAccountNumber(char *buffer, size_t size) {
    unsigned long long base = 1000000000ULL + (unsigned long long)(rand() % 900000000);
    snprintf(buffer, size, "%llu", base + (unsigned long long)(rand() % 100));
}

int loadAccounts(Account accounts[], int maxAccounts) {
    FILE *file = fopen(ACCOUNTS_FILE, "r");
    char line[512];
    char decodedPassword[FIELD];
    char decodedPin[32];
    int count = 0;

    if (file == NULL) {
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL && count < maxAccounts) {
        Account account;
        int parsed;

        trimNewline(line);
        if (line[0] == '\0') {
            continue;
        }

        memset(&account, 0, sizeof(account));
        parsed = sscanf(
            line,
            "%127[^,],%127[^,],%31[^,],%127[^,],%31[^,],%23[^,],%lf",
            account.name,
            account.email,
            account.phone,
            account.password,
            account.pin,
            account.accountNumber,
            &account.balance
        );

        if (parsed == 7) {
            decryptField(account.password, decodedPassword, sizeof(decodedPassword));
            decryptField(account.pin, decodedPin, sizeof(decodedPin));
            strcpy(account.password, decodedPassword);
            strcpy(account.pin, decodedPin);
            accounts[count++] = account;
            continue;
        }

        memset(&account, 0, sizeof(account));
        parsed = sscanf(
            line,
            "%127[^,],%127[^,],%23[^,],%lf",
            account.name,
            account.password,
            account.accountNumber,
            &account.balance
        );

        if (parsed == 4) {
            strcpy(account.email, "not-set");
            strcpy(account.phone, "0000000000");
            account.pin[0] = '\0';
            accounts[count++] = account;
        }
    }

    fclose(file);
    return count;
}

int saveAccounts(const Account accounts[], int count) {
    FILE *file = fopen(TEMP_ACCOUNTS_FILE, "w");
    char encryptedPassword[(FIELD * 2) + 8];
    char encryptedPin[32];
    int i;

    if (file == NULL) {
        return 0;
    }

    for (i = 0; i < count; i++) {
        encryptField(accounts[i].password, encryptedPassword, sizeof(encryptedPassword));
        encryptField(accounts[i].pin, encryptedPin, sizeof(encryptedPin));
        fprintf(
            file,
            "%s,%s,%s,%s,%s,%s,%.2f\n",
            accounts[i].name,
            accounts[i].email,
            accounts[i].phone,
            encryptedPassword,
            encryptedPin,
            accounts[i].accountNumber,
            accounts[i].balance
        );
    }

    fclose(file);
    remove(ACCOUNTS_FILE);
    rename(TEMP_ACCOUNTS_FILE, ACCOUNTS_FILE);
    return 1;
}

int loadTransactions(Transaction transactions[], int maxTransactions) {
    FILE *file = fopen(TRANSACTIONS_FILE, "r");
    char line[512];
    int count = 0;

    if (file == NULL) {
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL && count < maxTransactions) {
        Transaction transaction;
        int parsed;

        trimNewline(line);
        if (line[0] == '\0') {
            continue;
        }

        memset(&transaction, 0, sizeof(transaction));
        parsed = sscanf(
            line,
            "%127[^,],%39[^,],%63[^,],%lf,%127[^,],%lf",
            transaction.user,
            transaction.timestamp,
            transaction.type,
            &transaction.amount,
            transaction.note,
            &transaction.balanceAfter
        );

        if (parsed == 6) {
            transactions[count++] = transaction;
            continue;
        }

        memset(&transaction, 0, sizeof(transaction));
        parsed = sscanf(
            line,
            "%127[^,],%39[^,],%63[^,],%lf",
            transaction.user,
            transaction.timestamp,
            transaction.type,
            &transaction.amount
        );

        if (parsed == 4) {
            strcpy(transaction.note, "-");
            transaction.balanceAfter = 0.0;
            transactions[count++] = transaction;
        }
    }

    fclose(file);
    return count;
}

void appendTransaction(const char *user, const char *type, double amount, const char *note, double balanceAfter) {
    FILE *file = fopen(TRANSACTIONS_FILE, "a");
    char timestamp[40];

    if (file == NULL) {
        return;
    }

    currentTimestamp(timestamp, sizeof(timestamp));
    fprintf(file, "%s,%s,%s,%.2f,%s,%.2f\n", user, timestamp, type, amount, note, balanceAfter);
    fclose(file);
}

int findAccountIndexByName(Account accounts[], int count, const char *name) {
    int i;
    for (i = 0; i < count; i++) {
        if (equalsIgnoreCase(accounts[i].name, name)) {
            return i;
        }
    }
    return -1;
}

int findAccountIndexByAccountNumber(Account accounts[], int count, const char *accountNumber) {
    int i;
    for (i = 0; i < count; i++) {
        if (strcmp(accounts[i].accountNumber, accountNumber) == 0) {
            return i;
        }
    }
    return -1;
}

void printWelcomeMenu(void) {
    printf("=============================================\n");
    printf("         DIGITAL BANK CONSOLE PORTAL         \n");
    printf("=============================================\n");
    printf("1. Open Account\n");
    printf("2. Login\n");
    printf("3. Exit\n");
    printf("=============================================\n");
}

void printDashboardMenu(void) {
    printf("=============================================\n");
    printf("Dashboard for %s\n", sessionUser.name);
    printf("Account No: %s\n", sessionUser.accountNumber);
    printf("Balance   : Rs %.2f\n", sessionUser.balance);
    printf("=============================================\n");
    printf("1. Deposit Money\n");
    printf("2. Withdraw Money\n");
    printf("3. Transfer Money\n");
    printf("4. Check Balance\n");
    printf("5. Transaction History\n");
    printf("6. Account Details\n");
    printf("7. Update Contact Details\n");
    printf("8. Change Password\n");
    printf("9. Delete Account\n");
    printf("10. Logout\n");
    printf("=============================================\n");
}

void createAccount(void) {
    Account accounts[MAX_ACCOUNTS];
    Account account;
    char confirmPassword[FIELD];
    char confirmPin[8];
    int count = loadAccounts(accounts, MAX_ACCOUNTS);

    memset(&account, 0, sizeof(account));
    clearScreenVisual();
    printf("Open New Account\n");
    printf("----------------\n");

    readLine("Full Name: ", account.name, sizeof(account.name));
    if (account.name[0] == '\0') {
        printf("Name cannot be empty.\n");
        pauseScreen();
        return;
    }

    if (findAccountIndexByName(accounts, count, account.name) != -1) {
        printf("An account with this name already exists.\n");
        pauseScreen();
        return;
    }

    readLine("Email: ", account.email, sizeof(account.email));
    if (!validateEmail(account.email)) {
        printf("Invalid email address.\n");
        pauseScreen();
        return;
    }

    readLine("Phone (10 digits): ", account.phone, sizeof(account.phone));
    if (!validatePhone(account.phone)) {
        printf("Invalid phone number.\n");
        pauseScreen();
        return;
    }

    readSecretLine("Create Password: ", account.password, sizeof(account.password));
    if (!validatePassword(account.password)) {
        printf("Password must be at least 8 chars and include upper, lower, digit, and special character.\n");
        pauseScreen();
        return;
    }

    readSecretLine("Confirm Password: ", confirmPassword, sizeof(confirmPassword));
    if (strcmp(account.password, confirmPassword) != 0) {
        printf("Passwords do not match.\n");
        pauseScreen();
        return;
    }

    readSecretLine("Create 4-digit PIN: ", account.pin, sizeof(account.pin));
    if (!validatePin(account.pin)) {
        printf("PIN must be exactly 4 digits.\n");
        pauseScreen();
        return;
    }

    readSecretLine("Confirm PIN: ", confirmPin, sizeof(confirmPin));
    if (strcmp(account.pin, confirmPin) != 0) {
        printf("PIN confirmation failed.\n");
        pauseScreen();
        return;
    }

    account.balance = readDouble("Initial Deposit (minimum 0): ");
    if (account.balance < 0) {
        printf("Initial balance cannot be negative.\n");
        pauseScreen();
        return;
    }

    do {
        generateAccountNumber(account.accountNumber, sizeof(account.accountNumber));
    } while (findAccountIndexByAccountNumber(accounts, count, account.accountNumber) != -1);

    if (count >= MAX_ACCOUNTS) {
        printf("Account storage is full.\n");
        pauseScreen();
        return;
    }

    accounts[count++] = account;
    if (!saveAccounts(accounts, count)) {
        printf("Failed to save account.\n");
        pauseScreen();
        return;
    }

    appendTransaction(account.name, "Initial Deposit", account.balance, "Account created", account.balance);

    printf("\nAccount created successfully.\n");
    printf("Account Number: %s\n", account.accountNumber);
    printf("You can now login from the main menu.\n");
    pauseScreen();
}

void ensureLegacyDetails(Account *account, Account accounts[], int totalAccounts, int index) {
    char buffer[FIELD];
    int changed = 0;

    if (strcmp(account->email, "not-set") == 0 || !validateEmail(account->email)) {
        printf("\nThis account was created with the older C version.\n");
        readLine("Add your email: ", buffer, sizeof(buffer));
        if (validateEmail(buffer)) {
            strcpy(account->email, buffer);
            changed = 1;
        }
    }

    if (strcmp(account->phone, "0000000000") == 0 || !validatePhone(account->phone)) {
        readLine("Add your phone number (10 digits): ", buffer, sizeof(buffer));
        if (validatePhone(buffer)) {
            strcpy(account->phone, buffer);
            changed = 1;
        }
    }

    if (!validatePin(account->pin)) {
        readSecretLine("Set a new 4-digit PIN for secure actions: ", buffer, sizeof(buffer));
        if (validatePin(buffer)) {
            strcpy(account->pin, buffer);
            changed = 1;
        }
    }

    if (changed) {
        accounts[index] = *account;
        saveAccounts(accounts, totalAccounts);
        printf("Profile upgraded for the new console flow.\n");
    }
}

void login(void) {
    Account accounts[MAX_ACCOUNTS];
    char name[FIELD];
    char password[FIELD];
    int count = loadAccounts(accounts, MAX_ACCOUNTS);
    int index;

    clearScreenVisual();
    printf("Login\n");
    printf("-----\n");

    if (count == 0) {
        printf("No accounts found. Please create one first.\n");
        pauseScreen();
        return;
    }

    readLine("Name: ", name, sizeof(name));
    readSecretLine("Password: ", password, sizeof(password));

    index = findAccountIndexByName(accounts, count, name);
    if (index == -1) {
        printf("Account not found.\n");
        pauseScreen();
        return;
    }

    if (strcmp(accounts[index].password, password) != 0) {
        printf("Incorrect password.\n");
        pauseScreen();
        return;
    }

    sessionUser = accounts[index];
    isLoggedIn = 1;
    ensureLegacyDetails(&sessionUser, accounts, count, index);
    printf("Login successful.\n");
    pauseScreen();
}

void logout(void) {
    memset(&sessionUser, 0, sizeof(sessionUser));
    isLoggedIn = 0;
    printf("Logged out successfully.\n");
    pauseScreen();
}

void depositMoney(void) {
    Account accounts[MAX_ACCOUNTS];
    char pin[8];
    double amount;
    int count = loadAccounts(accounts, MAX_ACCOUNTS);
    int index = findAccountIndexByAccountNumber(accounts, count, sessionUser.accountNumber);

    if (index == -1) {
        printf("Session account could not be found.\n");
        pauseScreen();
        return;
    }

    amount = readDouble("Amount to deposit: ");
    if (amount <= 0) {
        printf("Deposit amount must be greater than zero.\n");
        pauseScreen();
        return;
    }

    readSecretLine("Enter 4-digit PIN to confirm: ", pin, sizeof(pin));
    if (strcmp(accounts[index].pin, pin) != 0) {
        printf("Invalid PIN.\n");
        pauseScreen();
        return;
    }

    accounts[index].balance += amount;
    if (!saveAccounts(accounts, count)) {
        printf("Could not save deposit.\n");
        pauseScreen();
        return;
    }

    sessionUser = accounts[index];
    appendTransaction(sessionUser.name, "Deposit", amount, "Self deposit", sessionUser.balance);
    printf("Deposit successful. New balance: Rs %.2f\n", sessionUser.balance);
    pauseScreen();
}

void withdrawMoney(void) {
    Account accounts[MAX_ACCOUNTS];
    char pin[8];
    double amount;
    int count = loadAccounts(accounts, MAX_ACCOUNTS);
    int index = findAccountIndexByAccountNumber(accounts, count, sessionUser.accountNumber);

    if (index == -1) {
        printf("Session account could not be found.\n");
        pauseScreen();
        return;
    }

    amount = readDouble("Amount to withdraw: ");
    if (amount <= 0) {
        printf("Withdrawal amount must be greater than zero.\n");
        pauseScreen();
        return;
    }

    if (accounts[index].balance < amount) {
        printf("Insufficient balance.\n");
        pauseScreen();
        return;
    }

    readSecretLine("Enter 4-digit PIN to confirm: ", pin, sizeof(pin));
    if (strcmp(accounts[index].pin, pin) != 0) {
        printf("Invalid PIN.\n");
        pauseScreen();
        return;
    }

    accounts[index].balance -= amount;
    if (!saveAccounts(accounts, count)) {
        printf("Could not save withdrawal.\n");
        pauseScreen();
        return;
    }

    sessionUser = accounts[index];
    appendTransaction(sessionUser.name, "Withdraw", amount, "Cash withdrawal", sessionUser.balance);
    printf("Withdrawal successful. Remaining balance: Rs %.2f\n", sessionUser.balance);
    pauseScreen();
}

void transferMoney(void) {
    Account accounts[MAX_ACCOUNTS];
    char recipientName[FIELD];
    char pin[8];
    char note[FIELD];
    double amount;
    int count = loadAccounts(accounts, MAX_ACCOUNTS);
    int senderIndex = findAccountIndexByAccountNumber(accounts, count, sessionUser.accountNumber);
    int receiverIndex;

    if (senderIndex == -1) {
        printf("Session account could not be found.\n");
        pauseScreen();
        return;
    }

    readLine("Recipient account name: ", recipientName, sizeof(recipientName));
    receiverIndex = findAccountIndexByName(accounts, count, recipientName);
    if (receiverIndex == -1) {
        printf("Recipient account not found.\n");
        pauseScreen();
        return;
    }

    if (receiverIndex == senderIndex) {
        printf("You cannot transfer money to your own account.\n");
        pauseScreen();
        return;
    }

    amount = readDouble("Transfer amount: ");
    if (amount <= 0) {
        printf("Transfer amount must be greater than zero.\n");
        pauseScreen();
        return;
    }

    if (accounts[senderIndex].balance < amount) {
        printf("Insufficient balance.\n");
        pauseScreen();
        return;
    }

    readSecretLine("Enter 4-digit PIN to confirm: ", pin, sizeof(pin));
    if (strcmp(accounts[senderIndex].pin, pin) != 0) {
        printf("Invalid PIN.\n");
        pauseScreen();
        return;
    }

    accounts[senderIndex].balance -= amount;
    accounts[receiverIndex].balance += amount;

    if (!saveAccounts(accounts, count)) {
        printf("Transfer could not be completed.\n");
        pauseScreen();
        return;
    }

    snprintf(note, sizeof(note), "Sent to %s", accounts[receiverIndex].name);
    appendTransaction(accounts[senderIndex].name, "Transfer Sent", amount, note, accounts[senderIndex].balance);

    snprintf(note, sizeof(note), "Received from %s", accounts[senderIndex].name);
    appendTransaction(accounts[receiverIndex].name, "Transfer Received", amount, note, accounts[receiverIndex].balance);

    sessionUser = accounts[senderIndex];
    printf("Transfer successful.\n");
    printf("Recipient: %s (%s)\n", accounts[receiverIndex].name, accounts[receiverIndex].accountNumber);
    printf("Remaining balance: Rs %.2f\n", sessionUser.balance);
    pauseScreen();
}

void checkBalance(void) {
    Account accounts[MAX_ACCOUNTS];
    int count = loadAccounts(accounts, MAX_ACCOUNTS);
    int index = findAccountIndexByAccountNumber(accounts, count, sessionUser.accountNumber);

    if (index == -1) {
        printf("Session account could not be found.\n");
        pauseScreen();
        return;
    }

    sessionUser = accounts[index];
    printf("Current balance: Rs %.2f\n", sessionUser.balance);
    pauseScreen();
}

void showTransactionHistory(void) {
    Transaction transactions[MAX_TRANSACTIONS];
    char filter[32];
    int count = loadTransactions(transactions, MAX_TRANSACTIONS);
    int i;
    int found = 0;

    clearScreenVisual();
    printf("Transaction History\n");
    printf("-------------------\n");
    readLine("Filter by type (all/deposit/withdraw/transfer): ", filter, sizeof(filter));

    for (i = count - 1; i >= 0; i--) {
        int include = 0;

        if (!equalsIgnoreCase(transactions[i].user, sessionUser.name)) {
            continue;
        }

        if (equalsIgnoreCase(filter, "all") || filter[0] == '\0') {
            include = 1;
        } else if (equalsIgnoreCase(filter, "deposit") &&
                   (equalsIgnoreCase(transactions[i].type, "Deposit") ||
                    equalsIgnoreCase(transactions[i].type, "Initial Deposit"))) {
            include = 1;
        } else if (equalsIgnoreCase(filter, "withdraw") && equalsIgnoreCase(transactions[i].type, "Withdraw")) {
            include = 1;
        } else if (equalsIgnoreCase(filter, "transfer") &&
                   (equalsIgnoreCase(transactions[i].type, "Transfer Sent") ||
                    equalsIgnoreCase(transactions[i].type, "Transfer Received"))) {
            include = 1;
        }

        if (include) {
            printf("%s | %-17s | Rs %-10.2f | %-20s | Balance: Rs %.2f\n",
                   transactions[i].timestamp,
                   transactions[i].type,
                   transactions[i].amount,
                   transactions[i].note[0] ? transactions[i].note : "-",
                   transactions[i].balanceAfter);
            found = 1;
        }
    }

    if (!found) {
        printf("No matching transactions found.\n");
    }

    pauseScreen();
}

void showAccountDetails(void) {
    Transaction transactions[MAX_TRANSACTIONS];
    int transactionCount = 0;
    int i;
    int count = loadTransactions(transactions, MAX_TRANSACTIONS);

    for (i = 0; i < count; i++) {
        if (equalsIgnoreCase(transactions[i].user, sessionUser.name)) {
            transactionCount++;
        }
    }

    clearScreenVisual();
    printf("Account Details\n");
    printf("---------------\n");
    printf("Name               : %s\n", sessionUser.name);
    printf("Email              : %s\n", sessionUser.email);
    printf("Phone              : %s\n", sessionUser.phone);
    printf("Account Number     : %s\n", sessionUser.accountNumber);
    printf("Current Balance    : Rs %.2f\n", sessionUser.balance);
    printf("Total Transactions : %d\n", transactionCount);
    pauseScreen();
}

void updateContact(void) {
    Account accounts[MAX_ACCOUNTS];
    char email[FIELD];
    char phone[32];
    char password[FIELD];
    int count = loadAccounts(accounts, MAX_ACCOUNTS);
    int index = findAccountIndexByAccountNumber(accounts, count, sessionUser.accountNumber);

    if (index == -1) {
        printf("Session account could not be found.\n");
        pauseScreen();
        return;
    }

    readLine("New email: ", email, sizeof(email));
    readLine("New phone (10 digits): ", phone, sizeof(phone));
    readSecretLine("Enter password to confirm: ", password, sizeof(password));

    if (!validateEmail(email)) {
        printf("Invalid email address.\n");
        pauseScreen();
        return;
    }

    if (!validatePhone(phone)) {
        printf("Invalid phone number.\n");
        pauseScreen();
        return;
    }

    if (strcmp(accounts[index].password, password) != 0) {
        printf("Incorrect password.\n");
        pauseScreen();
        return;
    }

    strcpy(accounts[index].email, email);
    strcpy(accounts[index].phone, phone);

    if (!saveAccounts(accounts, count)) {
        printf("Could not update contact details.\n");
        pauseScreen();
        return;
    }

    sessionUser = accounts[index];
    printf("Contact details updated successfully.\n");
    pauseScreen();
}

void changePassword(void) {
    Account accounts[MAX_ACCOUNTS];
    char current[FIELD];
    char newPassword[FIELD];
    char confirm[FIELD];
    int count = loadAccounts(accounts, MAX_ACCOUNTS);
    int index = findAccountIndexByAccountNumber(accounts, count, sessionUser.accountNumber);

    if (index == -1) {
        printf("Session account could not be found.\n");
        pauseScreen();
        return;
    }

    readSecretLine("Current password: ", current, sizeof(current));
    if (strcmp(accounts[index].password, current) != 0) {
        printf("Current password is incorrect.\n");
        pauseScreen();
        return;
    }

    readSecretLine("New password: ", newPassword, sizeof(newPassword));
    if (!validatePassword(newPassword)) {
        printf("New password does not meet the strength rules.\n");
        pauseScreen();
        return;
    }

    readSecretLine("Confirm new password: ", confirm, sizeof(confirm));
    if (strcmp(newPassword, confirm) != 0) {
        printf("Password confirmation failed.\n");
        pauseScreen();
        return;
    }

    strcpy(accounts[index].password, newPassword);
    if (!saveAccounts(accounts, count)) {
        printf("Could not update password.\n");
        pauseScreen();
        return;
    }

    sessionUser = accounts[index];
    printf("Password changed successfully.\n");
    pauseScreen();
}

void deleteAccount(void) {
    Account accounts[MAX_ACCOUNTS];
    Transaction transactions[MAX_TRANSACTIONS];
    char password[FIELD];
    char confirm[16];
    int count = loadAccounts(accounts, MAX_ACCOUNTS);
    int index = findAccountIndexByAccountNumber(accounts, count, sessionUser.accountNumber);
    int i;
    int transactionCount;

    if (index == -1) {
        printf("Session account could not be found.\n");
        pauseScreen();
        return;
    }

    readSecretLine("Enter password to delete account: ", password, sizeof(password));
    readLine("Type DELETE to confirm permanently: ", confirm, sizeof(confirm));

    if (strcmp(accounts[index].password, password) != 0 || strcmp(confirm, "DELETE") != 0) {
        printf("Account deletion cancelled.\n");
        pauseScreen();
        return;
    }

    for (i = index; i < count - 1; i++) {
        accounts[i] = accounts[i + 1];
    }
    count--;

    if (!saveAccounts(accounts, count)) {
        printf("Could not delete account.\n");
        pauseScreen();
        return;
    }

    transactionCount = loadTransactions(transactions, MAX_TRANSACTIONS);
    {
        FILE *file = fopen(TRANSACTIONS_FILE, "w");
        if (file != NULL) {
            for (i = 0; i < transactionCount; i++) {
                if (!equalsIgnoreCase(transactions[i].user, sessionUser.name)) {
                    fprintf(file, "%s,%s,%s,%.2f,%s,%.2f\n",
                            transactions[i].user,
                            transactions[i].timestamp,
                            transactions[i].type,
                            transactions[i].amount,
                            transactions[i].note,
                            transactions[i].balanceAfter);
                }
            }
            fclose(file);
        }
    }

    memset(&sessionUser, 0, sizeof(sessionUser));
    isLoggedIn = 0;
    printf("Account deleted successfully.\n");
    pauseScreen();
}

int main(void) {
    int choice;

    srand((unsigned int)time(NULL));

    while (1) {
        clearScreenVisual();

        if (!isLoggedIn) {
            printWelcomeMenu();
            choice = readInt("Choose an option: ");

            switch (choice) {
                case 1:
                    createAccount();
                    break;
                case 2:
                    login();
                    break;
                case 3:
                    printf("Thank you for using the Digital Bank Console.\n");
                    return 0;
                default:
                    printf("Invalid choice.\n");
                    pauseScreen();
            }
        } else {
            printDashboardMenu();
            choice = readInt("Choose an option: ");

            switch (choice) {
                case 1:
                    depositMoney();
                    break;
                case 2:
                    withdrawMoney();
                    break;
                case 3:
                    transferMoney();
                    break;
                case 4:
                    checkBalance();
                    break;
                case 5:
                    showTransactionHistory();
                    break;
                case 6:
                    showAccountDetails();
                    break;
                case 7:
                    updateContact();
                    break;
                case 8:
                    changePassword();
                    break;
                case 9:
                    deleteAccount();
                    break;
                case 10:
                    logout();
                    break;
                default:
                    printf("Invalid choice.\n");
                    pauseScreen();
            }
        }
    }
}
