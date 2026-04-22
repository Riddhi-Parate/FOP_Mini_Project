#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define MAX_ALARMS 10
#define LABEL_SIZE 64

typedef struct {
    int used;
    int enabled;
    int repeatDaily;
    int hour;
    int minute;
    int snoozeMinutes;
    int ringing;
    char label[LABEL_SIZE];
    time_t scheduledFor;
    time_t snoozedUntil;
    time_t lastTriggeredAt;
} Alarm;

static Alarm alarms[MAX_ALARMS];
static int activeAlarmIndex = -1;

void copyText(char *destination, size_t size, const char *source) {
    if (size == 0) {
        return;
    }

    strncpy(destination, source, size - 1);
    destination[size - 1] = '\0';
}

void toLocalTime(time_t timestamp, struct tm *output) {
    struct tm *temp = localtime(&timestamp);

    if (temp != NULL) {
        *output = *temp;
    } else {
        memset(output, 0, sizeof(*output));
    }
}

void clearScreen(void) {
    system("cls");
}

void pauseBriefly(int milliseconds) {
    Sleep(milliseconds);
}

void trimNewline(char *text) {
    size_t length = strlen(text);
    if (length > 0 && text[length - 1] == '\n') {
        text[length - 1] = '\0';
    }
}

void readLine(char *buffer, size_t size) {
    if (fgets(buffer, (int)size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }
    trimNewline(buffer);
}

int readIntInRange(const char *prompt, int minimum, int maximum) {
    char buffer[64];
    int value;
    char extra;

    while (1) {
        printf("%s", prompt);
        readLine(buffer, sizeof(buffer));

        if (sscanf(buffer, "%d %c", &value, &extra) == 1 &&
            value >= minimum && value <= maximum) {
            return value;
        }

        printf("  Invalid input. Enter a value between %d and %d.\n", minimum, maximum);
    }
}

int readYesNo(const char *prompt) {
    char buffer[16];

    while (1) {
        printf("%s", prompt);
        readLine(buffer, sizeof(buffer));

        if (buffer[0] == '\0') {
            continue;
        }

        if (tolower((unsigned char)buffer[0]) == 'y') {
            return 1;
        }

        if (tolower((unsigned char)buffer[0]) == 'n') {
            return 0;
        }

        printf("  Please enter y or n.\n");
    }
}

int countUsedAlarms(void) {
    int count = 0;
    int i;

    for (i = 0; i < MAX_ALARMS; i++) {
        if (alarms[i].used) {
            count++;
        }
    }

    return count;
}

int countEnabledAlarms(void) {
    int count = 0;
    int i;

    for (i = 0; i < MAX_ALARMS; i++) {
        if (alarms[i].used && alarms[i].enabled) {
            count++;
        }
    }

    return count;
}

time_t buildNextTimestamp(int hour, int minute, time_t fromTime) {
    struct tm schedule;
    time_t target;

    schedule = *localtime(&fromTime);
    schedule.tm_hour = hour;
    schedule.tm_min = minute;
    schedule.tm_sec = 0;
    target = mktime(&schedule);

    if (target <= fromTime) {
        schedule.tm_mday += 1;
        target = mktime(&schedule);
    }

    return target;
}

time_t getNextOccurrence(const Alarm *alarm, time_t now) {
    if (alarm->snoozedUntil > now) {
        return alarm->snoozedUntil;
    }

    if (!alarm->repeatDaily) {
        return alarm->scheduledFor;
    }

    return buildNextTimestamp(alarm->hour, alarm->minute, now);
}

void formatClock(time_t timestamp, char *buffer, size_t size) {
    struct tm currentTime;
    toLocalTime(timestamp, &currentTime);
    strftime(buffer, size, "%I:%M:%S %p", &currentTime);
}

void formatDate(time_t timestamp, char *buffer, size_t size) {
    struct tm currentTime;
    toLocalTime(timestamp, &currentTime);
    strftime(buffer, size, "%A, %d %b %Y", &currentTime);
}

void formatNextAlarmText(const Alarm *alarm, time_t now, char *buffer, size_t size) {
    if (!alarm || !alarm->enabled) {
        snprintf(buffer, size, "No active alarm");
        return;
    }

    if (alarm->snoozedUntil > now) {
        struct tm snoozeTime;
        toLocalTime(alarm->snoozedUntil, &snoozeTime);
        strftime(buffer, size, "Snoozed until %a %I:%M %p", &snoozeTime);
        return;
    }

    if (alarm->repeatDaily) {
        snprintf(buffer, size, "%02d:%02d daily - %s", alarm->hour, alarm->minute, alarm->label);
        return;
    }

    {
        struct tm scheduledTime;
        toLocalTime(alarm->scheduledFor, &scheduledTime);
        strftime(buffer, size, "%a %I:%M %p", &scheduledTime);
    }
}

Alarm *findNextActiveAlarm(time_t now) {
    Alarm *nextAlarm = NULL;
    time_t nextTime = 0;
    int i;

    for (i = 0; i < MAX_ALARMS; i++) {
        if (!alarms[i].used || !alarms[i].enabled) {
            continue;
        }

        if (nextAlarm == NULL || getNextOccurrence(&alarms[i], now) < nextTime) {
            nextAlarm = &alarms[i];
            nextTime = getNextOccurrence(&alarms[i], now);
        }
    }

    return nextAlarm;
}

void printDivider(void) {
    printf("==========================================================================\n");
}

void printHeader(void) {
    printf("SMART ALARM CONSOLE\n");
    printf("Clean terminal alarm clock with multiple alarms, labels, snooze, and repeat.\n");
    printDivider();
}

void printAlarmRow(int index, const Alarm *alarm, time_t now) {
    char nextText[64];

    if (!alarm->used) {
        return;
    }

    if (!alarm->enabled) {
        snprintf(nextText, sizeof(nextText), "Disabled");
    } else if (alarm->snoozedUntil > now) {
        struct tm snoozeTime;
        toLocalTime(alarm->snoozedUntil, &snoozeTime);
        strftime(nextText, sizeof(nextText), "Snoozed until %I:%M %p", &snoozeTime);
    } else if (alarm->repeatDaily) {
        snprintf(nextText, sizeof(nextText), "Repeats daily");
    } else {
        struct tm scheduleTime;
        toLocalTime(alarm->scheduledFor, &scheduleTime);
        strftime(nextText, sizeof(nextText), "One-time %a %I:%M %p", &scheduleTime);
    }

    printf(" %d. [%s] %02d:%02d  %-18s  %-24s  snooze %2d min%s\n",
           index + 1,
           alarm->enabled ? "ON " : "OFF",
           alarm->hour,
           alarm->minute,
           alarm->label,
           nextText,
           alarm->snoozeMinutes,
           alarm->ringing ? "  <RINGING>" : "");
}

void renderDashboard(const char *statusMessage) {
    time_t now = time(NULL);
    char timeText[32];
    char dateText[64];
    char nextAlarmText[80];
    Alarm *nextAlarm = findNextActiveAlarm(now);
    int i;

    clearScreen();
    formatClock(now, timeText, sizeof(timeText));
    formatDate(now, dateText, sizeof(dateText));
    formatNextAlarmText(nextAlarm, now, nextAlarmText, sizeof(nextAlarmText));

    printHeader();
    printf("Current time : %s\n", timeText);
    printf("Today        : %s\n", dateText);
    printf("Active alarms: %d / %d\n", countEnabledAlarms(), countUsedAlarms());
    printf("Next alarm   : %s\n", nextAlarmText);
    printDivider();
    printf("Alarms\n");

    if (countUsedAlarms() == 0) {
        printf(" No alarms yet. Press A to add one.\n");
    } else {
        for (i = 0; i < MAX_ALARMS; i++) {
            printAlarmRow(i, &alarms[i], now);
        }
    }

    printDivider();
    printf("Controls: [A]dd  [T]oggle  [D]elete  S[n]ooze  S[t]op  [Q]uit\n");
    printf("Status  : %s\n", statusMessage);

    if (activeAlarmIndex >= 0 && alarms[activeAlarmIndex].used && alarms[activeAlarmIndex].ringing) {
        printf("\nALARM RINGING: %s at %02d:%02d\n",
               alarms[activeAlarmIndex].label,
               alarms[activeAlarmIndex].hour,
               alarms[activeAlarmIndex].minute);
        printf("Press T to stop or N to snooze.\n");
    }
}

int findFreeSlot(void) {
    int i;

    for (i = 0; i < MAX_ALARMS; i++) {
        if (!alarms[i].used) {
            return i;
        }
    }

    return -1;
}

void addAlarmInteractive(char *statusMessage, size_t statusSize) {
    int index = findFreeSlot();
    char label[LABEL_SIZE];
    int hour;
    int minute;
    int repeatDaily;
    int snoozeMinutes;

    if (index < 0) {
        snprintf(statusMessage, statusSize, "Alarm list is full. Delete one before adding another.");
        return;
    }

    clearScreen();
    printHeader();
    printf("Create Alarm\n");
    printDivider();

    printf("Label (leave blank for Alarm): ");
    readLine(label, sizeof(label));
    if (label[0] == '\0') {
        copyText(label, sizeof(label), "Alarm");
    }

    hour = readIntInRange("Hour (0-23): ", 0, 23);
    minute = readIntInRange("Minute (0-59): ", 0, 59);
    repeatDaily = readYesNo("Repeat daily? (y/n): ");
    snoozeMinutes = readIntInRange("Snooze minutes (1-30): ", 1, 30);

    alarms[index].used = 1;
    alarms[index].enabled = 1;
    alarms[index].repeatDaily = repeatDaily;
    alarms[index].hour = hour;
    alarms[index].minute = minute;
    alarms[index].snoozeMinutes = snoozeMinutes;
    alarms[index].ringing = 0;
    alarms[index].snoozedUntil = 0;
    alarms[index].lastTriggeredAt = 0;
    copyText(alarms[index].label, sizeof(alarms[index].label), label);
    alarms[index].scheduledFor = repeatDaily ? 0 : buildNextTimestamp(hour, minute, time(NULL));

    snprintf(statusMessage, statusSize, "Alarm '%s' added for %02d:%02d.", label, hour, minute);
}

int chooseAlarmIndex(const char *title) {
    int number;

    clearScreen();
    printHeader();
    printf("%s\n", title);
    printDivider();

    if (countUsedAlarms() == 0) {
        printf("No alarms available.\n");
        printf("Press any key to return.");
        _getch();
        return -1;
    }

    {
        time_t now = time(NULL);
        int i;
        for (i = 0; i < MAX_ALARMS; i++) {
            printAlarmRow(i, &alarms[i], now);
        }
    }

    printDivider();
    number = readIntInRange("Choose alarm number: ", 1, MAX_ALARMS) - 1;
    if (!alarms[number].used) {
        printf("That slot is empty. Press any key to return.");
        _getch();
        return -1;
    }

    return number;
}

void toggleAlarmInteractive(char *statusMessage, size_t statusSize) {
    int index = chooseAlarmIndex("Toggle Alarm");

    if (index < 0) {
        snprintf(statusMessage, statusSize, "Toggle cancelled.");
        return;
    }

    alarms[index].enabled = !alarms[index].enabled;
    if (alarms[index].enabled && !alarms[index].repeatDaily && alarms[index].snoozedUntil == 0) {
        alarms[index].scheduledFor = buildNextTimestamp(alarms[index].hour, alarms[index].minute, time(NULL));
    }
    if (!alarms[index].enabled) {
        alarms[index].ringing = 0;
        alarms[index].snoozedUntil = 0;
        if (activeAlarmIndex == index) {
            activeAlarmIndex = -1;
        }
    }

    snprintf(statusMessage, statusSize, "Alarm '%s' %s.",
             alarms[index].label,
             alarms[index].enabled ? "enabled" : "disabled");
}

void deleteAlarmInteractive(char *statusMessage, size_t statusSize) {
    int index = chooseAlarmIndex("Delete Alarm");

    if (index < 0) {
        snprintf(statusMessage, statusSize, "Delete cancelled.");
        return;
    }

    snprintf(statusMessage, statusSize, "Alarm '%s' deleted.", alarms[index].label);
    memset(&alarms[index], 0, sizeof(Alarm));

    if (activeAlarmIndex == index) {
        activeAlarmIndex = -1;
    }
}

void stopActiveAlarm(char *statusMessage, size_t statusSize) {
    if (activeAlarmIndex < 0 || !alarms[activeAlarmIndex].used || !alarms[activeAlarmIndex].ringing) {
        snprintf(statusMessage, statusSize, "No alarm is currently ringing.");
        return;
    }

    alarms[activeAlarmIndex].ringing = 0;
    if (!alarms[activeAlarmIndex].repeatDaily) {
        alarms[activeAlarmIndex].enabled = 0;
    }

    snprintf(statusMessage, statusSize, "Alarm '%s' stopped.", alarms[activeAlarmIndex].label);
    activeAlarmIndex = -1;
}

void snoozeActiveAlarm(char *statusMessage, size_t statusSize) {
    time_t now = time(NULL);

    if (activeAlarmIndex < 0 || !alarms[activeAlarmIndex].used || !alarms[activeAlarmIndex].ringing) {
        snprintf(statusMessage, statusSize, "No alarm is currently ringing.");
        return;
    }

    alarms[activeAlarmIndex].ringing = 0;
    alarms[activeAlarmIndex].snoozedUntil = now + alarms[activeAlarmIndex].snoozeMinutes * 60;
    alarms[activeAlarmIndex].lastTriggeredAt = now;

    snprintf(statusMessage, statusSize, "Alarm '%s' snoozed for %d minutes.",
             alarms[activeAlarmIndex].label,
             alarms[activeAlarmIndex].snoozeMinutes);
    activeAlarmIndex = -1;
}

void refreshExpiredOneTimeAlarms(void) {
    time_t now = time(NULL);
    int i;

    for (i = 0; i < MAX_ALARMS; i++) {
        if (!alarms[i].used || !alarms[i].enabled || alarms[i].repeatDaily) {
            continue;
        }

        if (alarms[i].snoozedUntil == 0 &&
            alarms[i].scheduledFor != 0 &&
            now > alarms[i].scheduledFor + 60 &&
            !alarms[i].ringing) {
            alarms[i].enabled = 0;
        }
    }
}

void triggerAlarmIfDue(char *statusMessage, size_t statusSize) {
    time_t now = time(NULL);
    int i;

    if (activeAlarmIndex >= 0) {
        return;
    }

    for (i = 0; i < MAX_ALARMS; i++) {
        if (!alarms[i].used || !alarms[i].enabled) {
            continue;
        }

        if (alarms[i].lastTriggeredAt != 0 && now - alarms[i].lastTriggeredAt < 45) {
            continue;
        }

        if (alarms[i].snoozedUntil > 0) {
            if (now >= alarms[i].snoozedUntil) {
                alarms[i].ringing = 1;
                alarms[i].snoozedUntil = 0;
                alarms[i].lastTriggeredAt = now;
                activeAlarmIndex = i;
                snprintf(statusMessage, statusSize, "Alarm '%s' is ringing.", alarms[i].label);
                return;
            }
            continue;
        }

        if (alarms[i].repeatDaily) {
            struct tm currentTime;
            toLocalTime(now, &currentTime);
            if (currentTime.tm_hour == alarms[i].hour &&
                currentTime.tm_min == alarms[i].minute &&
                currentTime.tm_sec == 0) {
                alarms[i].ringing = 1;
                alarms[i].lastTriggeredAt = now;
                activeAlarmIndex = i;
                snprintf(statusMessage, statusSize, "Alarm '%s' is ringing.", alarms[i].label);
                return;
            }
        } else if (alarms[i].scheduledFor != 0 && now >= alarms[i].scheduledFor) {
            alarms[i].ringing = 1;
            alarms[i].lastTriggeredAt = now;
            activeAlarmIndex = i;
            snprintf(statusMessage, statusSize, "Alarm '%s' is ringing.", alarms[i].label);
            return;
        }
    }
}

void handleKeypress(char key, char *statusMessage, size_t statusSize, int *running) {
    switch (toupper((unsigned char)key)) {
        case 'A':
            addAlarmInteractive(statusMessage, statusSize);
            break;
        case 'T':
            if (activeAlarmIndex >= 0 && alarms[activeAlarmIndex].ringing) {
                stopActiveAlarm(statusMessage, statusSize);
            } else {
                toggleAlarmInteractive(statusMessage, statusSize);
            }
            break;
        case 'D':
            deleteAlarmInteractive(statusMessage, statusSize);
            break;
        case 'N':
            snoozeActiveAlarm(statusMessage, statusSize);
            break;
        case 'Q':
            *running = 0;
            break;
        default:
            snprintf(statusMessage, statusSize, "Unknown command. Use A, T, D, N, T, or Q.");
            break;
    }
}

int main(void) {
    char statusMessage[128] = "Ready. Press A to add your first alarm.";
    int running = 1;

    SetConsoleOutputCP(CP_UTF8);

    while (running) {
        refreshExpiredOneTimeAlarms();
        triggerAlarmIfDue(statusMessage, sizeof(statusMessage));
        renderDashboard(statusMessage);

        if (activeAlarmIndex >= 0 && alarms[activeAlarmIndex].ringing) {
            Beep(1000, 180);
        }

        {
            int elapsed = 0;
            while (elapsed < 1000) {
                if (_kbhit()) {
                    char key = (char)_getch();
                    handleKeypress(key, statusMessage, sizeof(statusMessage), &running);
                    break;
                }
                pauseBriefly(100);
                elapsed += 100;
            }
        }
    }

    clearScreen();
    printHeader();
    printf("Session closed. Stay on schedule.\n");
    return 0;
}
