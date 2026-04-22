// Close dropdowns on outside click
document.addEventListener("click", function(e) {
    if (!e.target.closest(".btn-dots") && !e.target.closest(".settings-dropdown"))
        document.getElementById("settingsDropdown").classList.remove("open");
    if (!e.target.closest(".profile-circle") && !e.target.closest(".profile-dropdown"))
        document.getElementById("profileDropdown").classList.remove("open");
});

// Dark mode
function toggleDarkMode() {
    const dark = document.body.classList.toggle("dark");
    const label = dark ? "☀️ Light Mode" : "🌙 Dark Mode";
    document.getElementById("darkModeBtn").innerText = label;
    document.getElementById("darkModeBtnUser").innerText = label;
    localStorage.setItem("darkMode", dark ? "1" : "0");
}
if (localStorage.getItem("darkMode") === "1") {
    document.body.classList.add("dark");
    document.addEventListener("DOMContentLoaded", () => {
        document.getElementById("darkModeBtn").innerText = "☀️ Light Mode";
        document.getElementById("darkModeBtnUser").innerText = "☀️ Light Mode";
    });
}

let currentUser = null;

// ── Auth ──────────────────────────────────────────────
function showCreate() {
    document.getElementById("loginSection").style.display = "none";
    document.getElementById("createSection").style.display = "block";
}

function showLogin() {
    document.getElementById("createSection").style.display = "none";
    document.getElementById("loginSection").style.display = "block";
}

function generateAccountNumber() {
    return Math.floor(1000000000 + Math.random() * 9000000000).toString();
}

function validatePassword(pwd) {
    return {
        len: pwd.length >= 8,
        upper: /[A-Z]/.test(pwd),
        lower: /[a-z]/.test(pwd),
        num: /[0-9]/.test(pwd),
        special: /[!@#$%^&*(),.?":{}|<>]/.test(pwd)
    };
}

function checkPasswordStrength() {
    const pwd = document.getElementById("newPassword").value;
    const r = validatePassword(pwd);
    document.getElementById("rule-len").className = r.len ? "pass" : "";
    document.getElementById("rule-upper").className = r.upper ? "pass" : "";
    document.getElementById("rule-lower").className = r.lower ? "pass" : "";
    document.getElementById("rule-num").className = r.num ? "pass" : "";
    document.getElementById("rule-special").className = r.special ? "pass" : "";
}

function validatePhone() {
    const val = document.getElementById("newPhone").value;
    const hint = document.getElementById("phoneHint");
    if (!val) { hint.innerText = ""; return; }
    if (!/^[6-9]\d{9}$/.test(val)) {
        hint.innerText = "Must be 10 digits and start with 6, 7, 8, or 9";
        hint.className = "field-hint error";
    } else {
        hint.innerText = "✓ Valid phone number";
        hint.className = "field-hint valid";
    }
}

function validateEmailField() {
    const val = document.getElementById("newEmail").value.trim();
    const hint = document.getElementById("emailHint");
    if (!val) { hint.innerText = ""; return; }
    if (!/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(val)) {
        hint.innerText = "Enter a valid email address";
        hint.className = "field-hint error";
    } else {
        hint.innerText = "✓ Valid email";
        hint.className = "field-hint valid";
    }
}

function createAccount() {
    const name = document.getElementById("newName").value.trim();
    const email = document.getElementById("newEmail").value.trim();
    const phone = document.getElementById("newPhone").value.trim();
    const password = document.getElementById("newPassword").value;
    const confirm = document.getElementById("confirmPassword").value;
    const pin = document.getElementById("newPin").value.trim();
    const confirmPinVal = document.getElementById("confirmPin").value.trim();
    const balance = parseFloat(document.getElementById("initialBalance").value);

    if (!name || !email || !phone || !password || !pin || isNaN(balance) || balance < 0) {
        document.getElementById("message2").innerText = "Please fill all fields with a valid balance."; return;
    }
    if (!/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(email)) {
        document.getElementById("message2").innerText = "Enter a valid email address."; return;
    }
    if (!/^[6-9]\d{9}$/.test(phone)) {
        document.getElementById("message2").innerText = "Enter a valid 10-digit phone number."; return;
    }
    const r = validatePassword(password);
    if (!r.len || !r.upper || !r.lower || !r.num || !r.special) {
        document.getElementById("message2").innerText = "Password does not meet all requirements."; return;
    }
    if (password !== confirm) {
        document.getElementById("message2").innerText = "Passwords do not match!"; return;
    }
    if (!/^\d{4}$/.test(pin)) {
        document.getElementById("message2").innerText = "PIN must be exactly 4 digits."; return;
    }
    if (pin !== confirmPinVal) {
        document.getElementById("message2").innerText = "PINs do not match!"; return;
    }
    if (localStorage.getItem(name)) {
    document.getElementById("message2").innerText = "Account name already exists!"; 
    return;
    }
    
    // Store pending user data and show card verification
    const accountNumber = generateAccountNumber();
    window._pendingUser = { name, email, phone, password, pin, balance, accountNumber,
        transactions: [{ type: "Initial Deposit", amount: balance, date: new Date().toISOString() }] };

    // Reset card modal
    document.getElementById("cardNumber").value = "";
    document.getElementById("cardExpiry").value = "";
    document.getElementById("cardCvv").value = "";
    document.getElementById("cardName").value = "";
    document.getElementById("cardNumberDisplay").innerText = "\u2022\u2022\u2022\u2022 \u2022\u2022\u2022\u2022 \u2022\u2022\u2022\u2022 \u2022\u2022\u2022\u2022";
    document.getElementById("cardExpiryDisplay").innerText = "MM/YY";
    document.getElementById("cardNameDisplay").innerText = "YOUR NAME";
    document.getElementById("cardMsg").innerText = "";
    document.getElementById("cardOverlay").classList.add("open");
    document.getElementById("cardModal").classList.add("open");
}

function formatCardNumber() {
    let val = document.getElementById("cardNumber").value.replace(/\D/g, "").substring(0, 16);
    val = val.match(/.{1,4}/g)?.join(" ") || val;
    document.getElementById("cardNumber").value = val;
    const display = val.padEnd(19, "\u2022").replace(/[0-9]/g, (d, i) => i < 12 ? "\u2022" : d);
    const parts = display.replace(/\u2022{4}/g, "\u2022\u2022\u2022\u2022");
    document.getElementById("cardNumberDisplay").innerText = val || "\u2022\u2022\u2022\u2022 \u2022\u2022\u2022\u2022 \u2022\u2022\u2022\u2022 \u2022\u2022\u2022\u2022";
}

function formatExpiry() {
    let val = document.getElementById("cardExpiry").value.replace(/\D/g, "").substring(0, 4);
    if (val.length >= 2) val = val.substring(0, 2) + "/" + val.substring(2);
    document.getElementById("cardExpiry").value = val;
    document.getElementById("cardExpiryDisplay").innerText = val || "MM/YY";
}

function updateCardName() {
    const val = document.getElementById("cardName").value.toUpperCase() || "YOUR NAME";
    document.getElementById("cardNameDisplay").innerText = val;
}

function cancelCardVerify() {
    document.getElementById("cardOverlay").classList.remove("open");
    document.getElementById("cardModal").classList.remove("open");
    window._pendingUser = null;
}

function verifyCard() {
    const number = document.getElementById("cardNumber").value.replace(/\s/g, "");
    const expiry = document.getElementById("cardExpiry").value;
    const cvv = document.getElementById("cardCvv").value;
    const name = document.getElementById("cardName").value.trim();

    if (!number || !expiry || !cvv || !name) {
        document.getElementById("cardMsg").innerText = "Please fill all card details."; return;
    }
    if (!/^\d{16}$/.test(number)) {
        document.getElementById("cardMsg").innerText = "Card number must be 16 digits."; return;
    }
    const [mm, yy] = expiry.split("/");
    const now = new Date();
    const expDate = new Date(2000 + parseInt(yy), parseInt(mm) - 1);
    if (!mm || !yy || parseInt(mm) < 1 || parseInt(mm) > 12 || expDate < now) {
        document.getElementById("cardMsg").innerText = "Invalid or expired expiry date."; return;
    }
    if (!/^\d{3}$/.test(cvv)) {
        document.getElementById("cardMsg").innerText = "CVV must be 3 digits."; return;
    }

    // Save account with the verified card linked
    const user = window._pendingUser;
    const cardNumber = document.getElementById("cardNumber").value.replace(/\s/g, "");
    user.cards = [{ number: cardNumber, expiry, name: name.toUpperCase(), type: "VISA" }];
    localStorage.setItem(user.name, JSON.stringify(user));

    cancelCardVerify(); // close the card modal
    window._pendingUser = null;

    showAccountModal(user.accountNumber);

    document.getElementById("createSection").reset();
}

function showAccountModal(accountNumber) {
    const modal = document.getElementById("accountModal");
    document.getElementById("newAccountNumber").textContent = accountNumber;
    modal.classList.add("open");
}

function closeAccountModal() {
    document.getElementById("accountModal").classList.remove("open");
}

function login() {
    const name = document.getElementById("loginName").value.trim();
    const pass = document.getElementById("loginPassword").value.trim();
    const userData = localStorage.getItem(name);

    if (!userData) { document.getElementById("message").innerText = "Account not found!"; return; }

    let user;
    try { user = JSON.parse(userData); } catch(e) {
        document.getElementById("message").innerText = "Account data corrupted. Please create a new account."; return;
    }
    if (!user || !user.password) { document.getElementById("message").innerText = "Account not found!"; return; }
    if (user.password !== pass) { document.getElementById("message").innerText = "Incorrect password!"; return; }

    const btn = document.getElementById("loginBtn");
    btn.disabled = true;
    btn.innerHTML = '<span class="spinner"></span> Logging in...';

    setTimeout(() => {
        btn.disabled = false;
        btn.innerHTML = "Sign In";

        // Migrate old locale-string dates to ISO
        if (user.transactions) {
            let migrated = false;
            user.transactions = user.transactions.map(t => {
                if (!t.date.includes("T")) { t.date = new Date().toISOString(); migrated = true; }
                return t;
            });
            if (migrated) localStorage.setItem(name, JSON.stringify(user));
        }

        currentUser = name;
        setInitials(user.name, user.accountNumber);

        document.getElementById("navBrandGuest").style.display = "none";
        document.getElementById("navBrandUser").style.display = "flex";
        document.getElementById("authPage").style.display = "none";
        document.getElementById("dashboard").style.display = "block";
        document.getElementById("navLinks").style.display = "none";
        document.getElementById("navUser").style.display = "flex";
        document.getElementById("navUserName").innerText = user.name;
        document.getElementById("userName").innerText = user.name;
        document.getElementById("accNumber").innerText = user.accountNumber;
        document.getElementById("balance").innerText = user.balance.toFixed(2);
        loadTransactions();
        loadAccountDetails();
    }, 1500);
}

function logout() {
    currentUser = null;
    document.getElementById("navBrandGuest").style.display = "flex";
    document.getElementById("navBrandUser").style.display = "none";
    document.getElementById("dashboard").style.display = "none";
    document.getElementById("authPage").style.display = "flex";
    document.getElementById("navLinks").style.display = "flex";
    document.getElementById("navUser").style.display = "none";
    showLogin();
}

// ── Profile ───────────────────────────────────────────
function getInitials(name) {
    return name.trim().split(" ").map(w => w[0]).join("").substring(0, 2).toUpperCase();
}

function setInitials(name, accountNumber) {
    const initials = getInitials(name);
    document.getElementById("profileCircle").innerText = initials;
    document.getElementById("profileCircleLg").innerText = initials;
    document.getElementById("profileDropdownName").innerText = name;
    document.getElementById("profileDropdownAcc").innerText = "Acc: " + accountNumber;
}

function toggleProfileDropdown() {
    document.getElementById("profileDropdown").classList.toggle("open");
}

function openProfileDetails() {
    document.getElementById("profileDropdown").classList.remove("open");
    const user = JSON.parse(localStorage.getItem(currentUser));
    document.getElementById("profilePanelAvatar").innerText = getInitials(user.name);
    document.getElementById("profileDetailsContent").innerHTML = `
        <div class="detail-item"><div class="detail-label">Full Name</div><div class="detail-value">${user.name}</div></div>
        <div class="detail-item"><div class="detail-label">Account Number</div><div class="detail-value">${user.accountNumber}</div></div>
        <div class="detail-item"><div class="detail-label">Email</div><div class="detail-value">${user.email || '—'}</div></div>
        <div class="detail-item"><div class="detail-label">Phone</div><div class="detail-value">${user.phone || '—'}</div></div>
        <div class="detail-item"><div class="detail-label">Current Balance</div><div class="detail-value">₹${user.balance.toFixed(2)}</div></div>
        <div class="detail-item"><div class="detail-label">Total Transactions</div><div class="detail-value">${user.transactions ? user.transactions.length : 0}</div></div>
    `;
    document.getElementById("profilePanel").classList.add("open");
    document.getElementById("profileOverlay").classList.add("open");
}

function validateUpdateEmail() {
    const val = document.getElementById("updateEmail").value.trim();
    const hint = document.getElementById("updateEmailHint");
    if (!val) { hint.innerText = ""; return; }
    if (!/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(val)) {
        hint.innerText = "Enter a valid email address";
        hint.className = "field-hint error";
    } else {
        hint.innerText = "✓ Valid email";
        hint.className = "field-hint valid";
    }
}

function validateUpdatePhone() {
    const val = document.getElementById("updatePhone").value;
    const hint = document.getElementById("updatePhoneHint");
    if (!val) { hint.innerText = ""; return; }
    if (!/^[6-9]\d{9}$/.test(val)) {
        hint.innerText = "Must be 10 digits and start with 6, 7, 8, or 9";
        hint.className = "field-hint error";
    } else {
        hint.innerText = "✓ Valid phone number";
        hint.className = "field-hint valid";
    }
}

function updateContact() {
    const email = document.getElementById("updateEmail").value.trim();
    const phone = document.getElementById("updatePhone").value.trim();
    const password = document.getElementById("updateContactPassword").value.trim();
    const user = JSON.parse(localStorage.getItem(currentUser));

    if (!email && !phone) { setMsg("contactMsg", "Enter at least one field to update.", false); return; }
    if (password !== user.password) { setMsg("contactMsg", "Incorrect password.", false); return; }

    if (email) {
        if (!/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(email)) { setMsg("contactMsg", "Enter a valid email address.", false); return; }
        user.email = email;
    }
    if (phone) {
        if (!/^[6-9]\d{9}$/.test(phone)) { setMsg("contactMsg", "Enter a valid 10-digit phone number.", false); return; }
        user.phone = phone;
    }

    localStorage.setItem(currentUser, JSON.stringify(user));
    document.getElementById("updateEmail").value = "";
    document.getElementById("updatePhone").value = "";
    document.getElementById("updateContactPassword").value = "";
    document.getElementById("updateEmailHint").innerText = "";
    document.getElementById("updatePhoneHint").innerText = "";
    setMsg("contactMsg", "Contact info updated successfully!", true);
}

// ── My Cards ─────────────────────────────────────────
const cardGradients = [
    "linear-gradient(135deg,#1a365d,#2b6cb0,#1a56db)",
    "linear-gradient(135deg,#2d3748,#4a5568,#718096)",
    "linear-gradient(135deg,#742a2a,#c53030,#e53e3e)",
    "linear-gradient(135deg,#1a4731,#276749,#38a169)",
    "linear-gradient(135deg,#44337a,#6b46c1,#9f7aea)",
];

function openMyCards() {
    document.getElementById("profileDropdown").classList.remove("open");
    renderCards();
    document.getElementById("cardsPanel").classList.add("open");
    document.getElementById("cardsOverlay").classList.add("open");
}

function closeMyCards() {
    document.getElementById("cardsPanel").classList.remove("open");
    document.getElementById("cardsOverlay").classList.remove("open");
}

function renderCards() {
    const user = JSON.parse(localStorage.getItem(currentUser));
    const grid = document.getElementById("cardsGrid");
    const cards = user.cards || [];
    grid.innerHTML = "";

    cards.forEach((c, i) => {
        const grad = cardGradients[i % cardGradients.length];
        const masked = "\u2022\u2022\u2022\u2022 \u2022\u2022\u2022\u2022 \u2022\u2022\u2022\u2022 " + c.number.slice(-4);
        const full = c.number.replace(/(\d{4})/g, '$1 ').trim();
        grid.innerHTML += `
            <div class="my-card" style="background:${grad}">
                <div class="card-top">
                    <span class="card-bank">🏦 SecureBank</span>
                    <span class="card-chip">🟡</span>
                </div>
                <div class="card-number-display" id="cardNum${i}">${masked}</div>
                <div class="card-bottom">
                    <div>
                        <div class="card-field-label">Card Holder</div>
                        <div class="card-field-value">${c.name}</div>
                    </div>
                    <div>
                        <div class="card-field-label">Expires</div>
                        <div class="card-field-value">${c.expiry}</div>
                    </div>
                    <div class="card-network">${c.type}</div>
                </div>
                <button class="card-remove-btn" onclick="removeCard(${i})">&#10005; Remove</button>
                <button class="card-eye-btn" onclick="toggleCardNumber(${i}, '${full}', '${masked}')" id="cardEye${i}">👁</button>
            </div>`;
    });

}

function toggleCardNumber(i, full, masked) {
    const el = document.getElementById("cardNum" + i);
    const btn = document.getElementById("cardEye" + i);
    if (el.innerText.includes("\u2022")) {
        el.innerText = full;
        btn.innerText = "🙈";
    } else {
        el.innerText = masked;
        btn.innerText = "👁";
    }
}

function removeCard(index) {
    if (!confirm("Remove this card?")) return;
    const user = JSON.parse(localStorage.getItem(currentUser));
    user.cards.splice(index, 1);
    localStorage.setItem(currentUser, JSON.stringify(user));
    renderCards();
}

function openAddCard() {
    document.getElementById("addCardNumber").value = "";
    document.getElementById("addCardExpiry").value = "";
    document.getElementById("addCardCvv").value = "";
    document.getElementById("addCardName").value = "";
    document.getElementById("addCardType").value = "VISA";
    document.getElementById("addCardNumberDisplay").innerText = "\u2022\u2022\u2022\u2022 \u2022\u2022\u2022\u2022 \u2022\u2022\u2022\u2022 \u2022\u2022\u2022\u2022";
    document.getElementById("addCardExpiryDisplay").innerText = "MM/YY";
    document.getElementById("addCardNameDisplay").innerText = "YOUR NAME";
    document.getElementById("addCardNetwork").innerText = "VISA";
    document.getElementById("addCardMsg").innerText = "";
    document.getElementById("addCardOverlay").classList.add("open");
    document.getElementById("addCardModal").classList.add("open");
}

function closeAddCard() {
    document.getElementById("addCardOverlay").classList.remove("open");
    document.getElementById("addCardModal").classList.remove("open");
}

function formatAddCard() {
    let val = document.getElementById("addCardNumber").value.replace(/\D/g,"").substring(0,16);
    val = val.match(/.{1,4}/g)?.join(" ") || val;
    document.getElementById("addCardNumber").value = val;
    document.getElementById("addCardNumberDisplay").innerText = val || "\u2022\u2022\u2022\u2022 \u2022\u2022\u2022\u2022 \u2022\u2022\u2022\u2022 \u2022\u2022\u2022\u2022";
}

function formatAddExpiry() {
    let val = document.getElementById("addCardExpiry").value.replace(/\D/g,"").substring(0,4);
    if (val.length >= 2) val = val.substring(0,2) + "/" + val.substring(2);
    document.getElementById("addCardExpiry").value = val;
    document.getElementById("addCardExpiryDisplay").innerText = val || "MM/YY";
}

function updateAddCardName() {
    document.getElementById("addCardNameDisplay").innerText = document.getElementById("addCardName").value.toUpperCase() || "YOUR NAME";
}

function updateAddCardNetwork() {
    document.getElementById("addCardNetwork").innerText = document.getElementById("addCardType").value;
}

function saveNewCard() {
    const number = document.getElementById("addCardNumber").value.replace(/\s/g,"");
    const expiry = document.getElementById("addCardExpiry").value;
    const cvv = document.getElementById("addCardCvv").value;
    const name = document.getElementById("addCardName").value.trim();
    const type = document.getElementById("addCardType").value;

    if (!number || !expiry || !cvv || !name) { document.getElementById("addCardMsg").innerText = "Please fill all card details."; return; }
    if (!/^\d{16}$/.test(number)) { document.getElementById("addCardMsg").innerText = "Card number must be 16 digits."; return; }
    const [mm, yy] = expiry.split("/");
    const expDate = new Date(2000 + parseInt(yy), parseInt(mm) - 1);
    if (!mm || !yy || parseInt(mm) < 1 || parseInt(mm) > 12 || expDate < new Date()) { document.getElementById("addCardMsg").innerText = "Invalid or expired expiry date."; return; }
    if (!/^\d{3}$/.test(cvv)) { document.getElementById("addCardMsg").innerText = "CVV must be 3 digits."; return; }

    const user = JSON.parse(localStorage.getItem(currentUser));
    if (!user.cards) user.cards = [];
    if (user.cards.some(c => c.number === number)) { document.getElementById("addCardMsg").innerText = "This card is already linked."; return; }

    user.cards.push({ number, expiry, name: name.toUpperCase(), type });
    localStorage.setItem(currentUser, JSON.stringify(user));
    closeAddCard();
    renderCards();
}


function closeProfileDetails() {
    document.getElementById("profilePanel").classList.remove("open");
    document.getElementById("profileOverlay").classList.remove("open");
}

// ── Settings ──────────────────────────────────────────
function toggleSettingsPanel() {
    document.getElementById("settingsDropdown").classList.toggle("open");
}

function openSettingsSection(section) {
    document.getElementById("settingsDropdown").classList.remove("open");
    ["sectionUpdateName", "sectionUpdateEmail", "sectionUpdatePhone", "sectionChangePassword", "sectionDeleteAccount"].forEach(id => {
        document.getElementById(id).style.display = "none";
    });
    const titles = { updateName: "Update Name", updateEmail: "Update Email", updatePhone: "Update Phone Number", changePassword: "Change Password", deleteAccount: "Delete Account" };
    document.getElementById("settingsPanelTitle").innerText = titles[section];
    document.getElementById("section" + section.charAt(0).toUpperCase() + section.slice(1)).style.display = "block";
    document.getElementById("settingsPanel").classList.add("open");
    document.getElementById("settingsOverlay").classList.add("open");
}

function closeSettingsPanel() {
    document.getElementById("settingsPanel").classList.remove("open");
    document.getElementById("settingsOverlay").classList.remove("open");
    document.getElementById("settingsDropdown").classList.remove("open");
}

function updateName() {
    const newName = document.getElementById("newNameUpdate").value.trim();
    const password = document.getElementById("nameUpdatePassword").value.trim();
    const user = JSON.parse(localStorage.getItem(currentUser));

    if (!newName) { setMsg("nameMsg", "Please enter a new name.", false); return; }
    if (password !== user.password) { setMsg("nameMsg", "Incorrect password.", false); return; }
    if (newName === user.name) { setMsg("nameMsg", "New name is the same as current name.", false); return; }
    if (localStorage.getItem(newName)) { setMsg("nameMsg", "That name is already taken.", false); return; }

    localStorage.removeItem(currentUser);
    user.name = newName;
    currentUser = newName;
    localStorage.setItem(currentUser, JSON.stringify(user));

    document.getElementById("userName").innerText = newName;
    document.getElementById("navUserName").innerText = newName;
    setInitials(newName, user.accountNumber);
    document.getElementById("newNameUpdate").value = "";
    document.getElementById("nameUpdatePassword").value = "";
    setMsg("nameMsg", "Name updated successfully!", true);
    loadAccountDetails();
}

function checkNewPasswordStrength() {
    const pwd = document.getElementById("newPasswordUpdate").value;
    const r = validatePassword(pwd);
    document.getElementById("upd-len").className = r.len ? "pass" : "";
    document.getElementById("upd-upper").className = r.upper ? "pass" : "";
    document.getElementById("upd-lower").className = r.lower ? "pass" : "";
    document.getElementById("upd-num").className = r.num ? "pass" : "";
    document.getElementById("upd-special").className = r.special ? "pass" : "";
}

function changePassword() {
    const current = document.getElementById("currentPassword").value.trim();
    const newPwd = document.getElementById("newPasswordUpdate").value;
    const confirm = document.getElementById("confirmNewPassword").value;
    const user = JSON.parse(localStorage.getItem(currentUser));

    if (current !== user.password) { setMsg("pwdMsg", "Current password is incorrect.", false); return; }
    const r = validatePassword(newPwd);
    if (!r.len || !r.upper || !r.lower || !r.num || !r.special) { setMsg("pwdMsg", "New password does not meet all requirements.", false); return; }
    if (newPwd !== confirm) { setMsg("pwdMsg", "Passwords do not match.", false); return; }
    if (newPwd === current) { setMsg("pwdMsg", "New password must differ from current.", false); return; }

    user.password = newPwd;
    localStorage.setItem(currentUser, JSON.stringify(user));
    document.getElementById("currentPassword").value = "";
    document.getElementById("newPasswordUpdate").value = "";
    document.getElementById("confirmNewPassword").value = "";
    setMsg("pwdMsg", "Password changed successfully!", true);
}

function deleteAccount() {
    const password = document.getElementById("deletePassword").value.trim();
    const user = JSON.parse(localStorage.getItem(currentUser));

    if (password !== user.password) { setMsg("deleteMsg", "Incorrect password.", false); return; }
    if (!confirm("Are you sure you want to permanently delete your account? This cannot be undone.")) return;

    localStorage.removeItem(currentUser);
    currentUser = null;
    closeSettingsPanel();
    document.getElementById("navBrandGuest").style.display = "flex";
    document.getElementById("navBrandUser").style.display = "none";
    document.getElementById("dashboard").style.display = "none";
    document.getElementById("authPage").style.display = "flex";
    document.getElementById("navLinks").style.display = "flex";
    document.getElementById("navUser").style.display = "none";
    showLogin();
    alert("Your account has been deleted.");
}

// ── PIN Modal ─────────────────────────────────────────
let pendingAction = null;

function openPinModal(action) {
    pendingAction = action;
    document.getElementById("pinModalTitle").innerText = action === "deposit" ? "Confirm Deposit" : "Confirm Withdrawal";
    document.getElementById("pinError").innerText = "";
    document.querySelectorAll(".pin-box").forEach(b => b.value = "");
    document.getElementById("pinOverlay").classList.add("open");
    document.getElementById("pinModal").classList.add("open");
    document.querySelectorAll(".pin-box")[0].focus();
}

function cancelPin() {
    document.getElementById("pinOverlay").classList.remove("open");
    document.getElementById("pinModal").classList.remove("open");
    pendingAction = null;
}

function confirmPin() {
    const entered = [...document.querySelectorAll(".pin-box")].map(b => b.value).join("");
    const user = JSON.parse(localStorage.getItem(currentUser));
    if (entered !== user.pin) {
        document.getElementById("pinError").innerText = "Incorrect PIN. Try again.";
        document.querySelectorAll(".pin-box").forEach(b => b.value = "");
        document.querySelectorAll(".pin-box")[0].focus();
        return;
    }
    const action = pendingAction;
    cancelPin();
    if (action === "deposit") executeDeposit();
    else if (action === "withdraw") executeWithdraw();
    if (action === "transfer") executeTransfer();
}

function pinNext(el, next) {
    el.value = el.value.replace(/[^0-9]/g, "");
    if (el.value && next <= 3) document.querySelectorAll(".pin-box")[next].focus();
    if (next === 4 && el.value) confirmPin();
}

function pinBack(e, el, prev) {
    if (e.key === "Backspace" && !el.value && prev >= 0)
        document.querySelectorAll(".pin-box")[prev].focus();
}

// ── Dashboard ─────────────────────────────────────────
function deposit() {
    const amount = parseFloat(document.getElementById("amount").value);
    if (isNaN(amount) || amount <= 0) { showActionMsg("Enter a valid amount", false); return; }
    openPinModal("deposit");
}

function withdraw() {
    const amount = parseFloat(document.getElementById("amount").value);
    const user = JSON.parse(localStorage.getItem(currentUser));
    if (isNaN(amount) || amount <= 0) { showActionMsg("Enter a valid amount", false); return; }
    if (amount > user.balance) { showActionMsg("Insufficient balance!", false); return; }
    openPinModal("withdraw");
}

function executeDeposit() {
    const amount = parseFloat(document.getElementById("amount").value);
    const user = JSON.parse(localStorage.getItem(currentUser));
    user.balance += amount;
    user.transactions.push({ type: "Deposit", amount, date: new Date().toISOString() });
    localStorage.setItem(currentUser, JSON.stringify(user));
    document.getElementById("balance").innerText = user.balance.toFixed(2);
    document.getElementById("amount").value = "";
    showTransferSuccess(
    "Self Deposit",
    user.accountNumber,
    amount,
    user.balance
    );
    loadTransactions();
    loadAccountDetails();

    showTransferSuccess(
    "Transaction Successful",
    user.accountNumber,
    amount,
    user.balance
    );
}

function executeWithdraw() {
    const amount = parseFloat(document.getElementById("amount").value);
    const user = JSON.parse(localStorage.getItem(currentUser));
    user.balance -= amount;
    user.transactions.push({ type: "Withdraw", amount, date: new Date().toISOString() });
    localStorage.setItem(currentUser, JSON.stringify(user));
    document.getElementById("balance").innerText = user.balance.toFixed(2);
    document.getElementById("amount").value = "";
    showTransferSuccess(
"Self Withdrawal",
user.accountNumber,
amount,
user.balance
);
    loadTransactions();
    loadAccountDetails();

    showActionSuccess(
    "Withdrawal Successful",
    amount,
    user.balance
    );
}

function showActionSuccess(title, amount, balance){

document.getElementById("successTitle").innerText = title;
document.getElementById("successName").innerText = "-";
document.getElementById("successAcc").innerText = "-";
document.getElementById("successAmount").innerText = "₹" + amount.toFixed(2);
document.getElementById("successBalance").innerText = "₹" + balance.toFixed(2);
document.getElementById("successDate").innerText = new Date().toLocaleString();

document.getElementById("transferSuccessOverlay").style.display = "block";
document.getElementById("transferSuccessModal").style.display = "block";
}

function switchTab(tab, btn) {
    document.querySelectorAll(".tab-btn").forEach(b => b.classList.remove("active"));
    btn.classList.add("active");
    document.getElementById("transactionsTab").style.display = tab === "transactions" ? "block" : "none";
}

let activeRange = "all";

function filterTransactions(range, btn) {
    document.querySelectorAll(".filter-btn").forEach(b => b.classList.remove("active"));
    btn.classList.add("active");
    activeRange = range;
    renderTransactions();
}

function searchTransactions() {
    renderTransactions();
}

function renderTransactions() {
    const user = JSON.parse(localStorage.getItem(currentUser));
    const list = document.getElementById("transactionList");
    const keyword = document.getElementById("txnSearch").value.trim().toLowerCase();
    const typeFilter = document.getElementById("txnTypeFilter").value;
    list.innerHTML = "";

    if (!user.transactions || user.transactions.length === 0) {
        list.innerHTML = "<p class='no-txn'>No transactions yet.</p>"; return;
    }

    const now = new Date();
    const startOfDay   = new Date(now.getFullYear(), now.getMonth(), now.getDate());
    const startOfWeek  = new Date(startOfDay); startOfWeek.setDate(startOfDay.getDate() - startOfDay.getDay());
    const startOfMonth = new Date(now.getFullYear(), now.getMonth(), 1);
    const startOfYear  = new Date(now.getFullYear(), 0, 1);

    const filtered = [...user.transactions].reverse().filter(t => {
        const d = new Date(t.date);
        const displayDate = d.toLocaleString().toLowerCase();
        const amountStr = t.amount.toFixed(2);

        // Date range filter
        if (activeRange === "today" && d < startOfDay) return false;
        if (activeRange === "week"  && d < startOfWeek) return false;
        if (activeRange === "month" && d < startOfMonth) return false;
        if (activeRange === "year"  && d < startOfYear) return false;

        // Type filter
        if (typeFilter !== "all" && t.type !== typeFilter) return false;

        // Keyword search — matches date, amount, or type
        if (keyword && !displayDate.includes(keyword) && !amountStr.includes(keyword) && !t.type.toLowerCase().includes(keyword)) return false;

        return true;
    });

    if (filtered.length === 0) {
        list.innerHTML = "<p class='no-txn'>No transactions found.</p>"; return;
    }

    filtered.forEach((t, i) => {
        const isDebit = t.type === "Withdraw" || t.type === "Transfer Sent";
        const isCredit = t.type === "Deposit" || t.type === "Transfer Received";

        const typeClass = isCredit ? "deposit" : isDebit ? "withdraw" : "initial";
        const icon = isCredit ? "⬆" : isDebit ? "⬇" : "💳";
        const sign = isDebit ? "-" : "+";
        const idx = user.transactions.length - 1 - user.transactions.slice().reverse().findIndex(x => x.date === t.date && x.amount === t.amount && x.type === t.type);
        list.innerHTML += `
            <div class="transaction-item">
                <div class="txn-left">
                    <div class="txn-icon ${typeClass}">${icon}</div>
                    <div>
                        <div class="txn-type">${t.type}</div>
                        <div class="txn-date">${new Date(t.date).toLocaleString()}</div>
                    </div>
                </div>
                <div class="txn-right">
                    <div class="txn-amount ${typeClass}">${sign}₹${t.amount.toFixed(2)}</div>
                    <button class="btn-receipt" onclick='downloadReceipt(${JSON.stringify(t)})'>&#128196; Receipt</button>
                </div>
            </div>`;
    });
}

function loadTransactions(range = "all") {
    activeRange = range;
    if (document.getElementById("txnSearch")) document.getElementById("txnSearch").value = "";
    if (document.getElementById("txnTypeFilter")) document.getElementById("txnTypeFilter").value = "all";
    renderTransactions();
}

function loadAccountDetails() {
    // Account details shown in profile panel only
}

function downloadReceipt(t) {
    const user = JSON.parse(localStorage.getItem(currentUser));
    const sign = (t.type === "Withdraw" || t.type === "Transfer Sent") ? "-" : "+";
    const color = t.type === "Withdraw" || t.type === "Transfer Sent"? "#c53030" : "#276749";
    const refNo = "SB" + Math.abs(t.date.split("").reduce((a, c) => (a << 5) - a + c.charCodeAt(0), 0) ^ Math.round(t.amount * 100)).toString(16).toUpperCase().padStart(8, "0");
    const date = new Date(t.date).toLocaleString();

    const html = `
    <!DOCTYPE html>
    <html>
    <head>
        <title>Transaction Receipt</title>
        <style>
            * { margin:0; padding:0; box-sizing:border-box; }
            body { font-family: 'Segoe UI', sans-serif; background:#f0f4f8; display:flex; justify-content:center; align-items:center; min-height:100vh; }
            .receipt { background:#fff; width:380px; border-radius:16px; overflow:hidden; box-shadow:0 8px 32px rgba(0,0,0,0.12); }
            .receipt-header { background:linear-gradient(135deg,#1a56db,#0e3a8c); color:#fff; padding:28px 24px; text-align:center; }
            .receipt-header .bank { font-size:22px; font-weight:700; margin-bottom:4px; }
            .receipt-header .title { font-size:13px; opacity:0.8; text-transform:uppercase; letter-spacing:1px; }
            .receipt-status { text-align:center; padding:20px; border-bottom:1px dashed #e2e8f0; }
            .status-icon { font-size:40px; margin-bottom:8px; }
            .status-text { font-size:13px; color:#718096; }
            .amount-big { font-size:36px; font-weight:800; color:${color}; margin:4px 0; }
            .receipt-body { padding:20px 24px; }
            .row { display:flex; justify-content:space-between; padding:10px 0; border-bottom:1px solid #f0f4f8; }
            .row:last-child { border-bottom:none; }
            .row .label { font-size:12px; color:#a0aec0; text-transform:uppercase; letter-spacing:0.5px; }
            .row .value { font-size:13px; font-weight:600; color:#2d3748; text-align:right; max-width:200px; word-break:break-all; }
            .receipt-footer { background:#f7fafc; padding:16px 24px; text-align:center; border-top:1px dashed #e2e8f0; }
            .receipt-footer p { font-size:11px; color:#a0aec0; line-height:1.6; }
            @media print {
                body { background:#fff; }
                .receipt { box-shadow:none; border-radius:0; width:100%; }
            }
        </style>
    </head>
    <body>
        <div class="receipt">
            <div class="receipt-header">
                <div class="bank">🏦 SecureBank</div>
                <div class="title">Transaction Receipt</div>
            </div>
            <div class="receipt-status">
                <div class="status-icon">${t.type === "Withdraw" ? "🟥" : "🟢"}</div>
                <div class="status-text">Transaction ${t.type === "Withdraw" ? "Debited" : "Credited"}</div>
                <div class="amount-big">${sign}₹${t.amount.toFixed(2)}</div>
            </div>
            <div class="receipt-body">
                <div class="row"><span class="label">Transaction Type</span><span class="value">${t.type}</span></div>
                <div class="row"><span class="label">Account Holder</span><span class="value">${user.name}</span></div>
                <div class="row"><span class="label">Account Number</span><span class="value">${user.accountNumber}</span></div>
                <div class="row"><span class="label">Date & Time</span><span class="value">${date}</span></div>
                <div class="row"><span class="label">Reference No.</span><span class="value">${refNo}</span></div>
                <div class="row"><span class="label">Status</span><span class="value" style="color:#38a169">✓ Successful</span></div>
            </div>
            <div class="receipt-footer">
                <p>Thank you for banking with SecureBank.<br>This is a computer-generated receipt.</p>
            </div>
        </div>
        <script>window.onload = () => { window.print(); }<\/script>
    </body>
    </html>`;

    const win = window.open("", "_blank");
    win.document.write(html);
    win.document.close();
}

function setMsg(id, msg, success) {
    const el = document.getElementById(id);
    el.innerText = msg;
    el.className = "message " + (success ? "success-msg" : "");
    if (success) setTimeout(() => el.innerText = "", 3000);
}

function openTransferPanel() {
    document.getElementById("transferOverlay").classList.add("show");
    document.getElementById("transferPanel").classList.add("open");

    // close profile dropdown
    document.getElementById("profileDropdown").classList.remove("open");
}

function closeTransfer() {
    document.getElementById("transferOverlay").classList.remove("show");
    document.getElementById("transferPanel").classList.remove("open");
}

let transferTargetUser = null;
let transferAmountValue = 0;

function transferMoney() {
    const recipientName = document.getElementById("transferTo").value.trim().toLowerCase();
    const amount = parseFloat(document.getElementById("transferAmount").value);
    const user = JSON.parse(localStorage.getItem(currentUser));

    if (!recipientName || isNaN(amount) || amount <= 0) {
        showToast("Enter valid name and amount", false);
        return;
    }

    if (recipientName === user.name.toLowerCase()) {
        showToast("Cannot transfer to your own account", false);
        return;
    }

    const recipientKey = Object.keys(localStorage).find(key => {
        const data = JSON.parse(localStorage.getItem(key));
        return data.name.toLowerCase() === recipientName;
    });

    if (!recipientKey) {
        showToast("Account name not found", false);
        return;
    }

    if (amount > user.balance) {
        showActionMsg("Insufficient balance", false);
        return;
    }

    window._pendingTransfer = { recipientKey, amount };
    openPinModal("transfer");
}

function executeTransfer() {
    const sender = JSON.parse(localStorage.getItem(currentUser));
    const { recipientKey, amount } = window._pendingTransfer;

    const receiver = JSON.parse(localStorage.getItem(recipientKey));

    // Deduct from sender
    sender.balance = parseFloat(sender.balance) - parseFloat(amount);

    // Add to receiver
    receiver.balance = parseFloat(receiver.balance) + parseFloat(amount);

    const date = new Date().toISOString();

    // Sender transaction
    sender.transactions.push({
        type: "Transfer Sent",
        amount: amount,
        to: receiver.name,
        date: date
    });

    // Receiver transaction
    receiver.transactions.push({
        type: "Transfer Received",
        amount: amount,
        from: sender.name,
        date: date
    });

    // Save BOTH accounts
    localStorage.setItem(currentUser, JSON.stringify(sender));
    localStorage.setItem(recipientKey, JSON.stringify(receiver));

    // reload updated user
    const updatedUser = JSON.parse(localStorage.getItem(currentUser));

    // update UI
    document.getElementById("balance").innerText = `${updatedUser.balance.toFixed(2)}`;

    loadAccountDetails();
    loadTransactions();

    window._pendingTransfer = null;
    closeTransfer();

    showTransferSuccess(
    receiver.name,
    receiver.accountNumber,
    amount,
    updatedUser.balance
);
}

function checkReceiver() {
    const name = document.getElementById("transferTo").value.trim().toLowerCase();
    const label = document.getElementById("receiverName");

    if (!name) {
        label.innerText = "";
        label.className = "receiver-name";
        return;
    }

    const users = Object.keys(localStorage)
        .map(k => {
            try { return JSON.parse(localStorage.getItem(k)); }
            catch { return null; }
        })
        .filter(u => u && u.name);

    const receiver = users.find(u => u.name.toLowerCase() === name);

    if (receiver) {
        label.innerHTML = `
            <div>Receiver: ${receiver.name}</div>
            <div style="font-size:12px;opacity:0.7">
                Acc No: ${receiver.accountNumber}
            </div>
        `;
        label.className = "receiver-name valid";
    } else {
        label.innerText = "Account not found";
        label.className = "receiver-name invalid";
    }
}

function showToast(message, success=true){
    const toast = document.getElementById("toast");

    toast.innerText = message;
    toast.className = "show " + (success ? "success" : "error");

    setTimeout(()=>{
        toast.classList.remove("show");
    },3000);
}

function showTransferSuccess(name, acc, amount, balance){

    document.getElementById("successName").innerText = name;
    document.getElementById("successAcc").innerText = acc;
    document.getElementById("successAmount").innerText = "₹" + amount.toFixed(2);
    document.getElementById("successBalance").innerText = "₹" + balance.toFixed(2);
    document.getElementById("successDate").innerText = new Date().toLocaleString();

    document.getElementById("transferSuccessOverlay").style.display = "block";
    document.getElementById("transferSuccessModal").style.display = "block";
}

function closeTransferSuccess(){
    document.getElementById("transferSuccessOverlay").style.display = "none";
    document.getElementById("transferSuccessModal").style.display = "none";
}