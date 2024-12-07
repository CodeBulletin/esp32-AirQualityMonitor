#ifndef TIME_CONTROLLER_H
#define TIME_CONTROLLER_H

#include <time.h>
#include <WiFi.h>
#include <lwip/apps/sntp.h>
#include "secrets.h"
#include "debug.h"

int syncTimeFlag = 0;
int isTimeSynced = 0;
int timeSyncFailureCount = 0;

void syncFromNTP()
{
    esp_netif_init();
    if(sntp_enabled()){
        sntp_stop();
    }
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    char *server[NTP_SERVERS_SIZE] = NTP_SERVERS;
    for (int i = 0; i < NTP_SERVERS_SIZE; i++) {
        sntp_setservername(i, server[i]);
    }
    sntp_init();
}

void syncTime() {
    DEBUG_PRINTLN("INFO: Syncing time with NTP server...");
    syncFromNTP();
}

bool getTime(struct tm &timeInfo) {
    if (!getLocalTime(&timeInfo)) {
        return false;
    }
    return true;
}

String readTime() {
    struct tm timeInfo;
    if (!getLocalTime(&timeInfo)) {
        return "Failed to obtain time";
    }
    char timeStr[50];
    // dd-mm-yyyy hh:mm:ss
    sprintf(timeStr, "%04d-%02d-%02d %02d:%02d:%02d", timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday, timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
    return String(timeStr);
}

unsigned long getCurrentTime() {
    return millis();
}

unsigned long getCurrenTimeDiff(int time) {
    return getCurrentTime() - time;
}

DEBUG_ONLY(
    void printLocalTime() {
        struct tm timeInfo;
        if (!getLocalTime(&timeInfo)) {
            DEBUG_PRINTLN("ERROR: Failed to obtain time");
            return;
        }
        DEBUG_PRINTF("Time: %d:%d:%d\n", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
    }
)

int syncTimeToServer(bool connected) {
    if (!syncTimeFlag || !isTimeSynced) {
        if (connected) {
            syncTime();
            struct tm timeInfo;
            if (getTime(timeInfo)) {
                isTimeSynced = 1;
                syncTimeFlag = TIME_SYNC_INTERVAL;
                timeSyncFailureCount = 0;
                DEBUG_PRINTLN("INFO: Time synced with NTP server...");
                return 2;
            } else {
                syncTimeFlag = TIME_SYNC_FAIL_INTERVAL;
                timeSyncFailureCount++;
                DEBUG_PRINTLN("WARNING: Time sync failed with NTP server...");
            }
        } else {
            syncTimeFlag = TIME_SYNC_FAIL_INTERVAL;
            timeSyncFailureCount++;
            DEBUG_PRINTLN("WARNING: Time sync failed due to no internet connection...");
        }

        if (timeSyncFailureCount >= TIME_SYNC_FAILURE_THRESHOLD) {
            DEBUG_PRINTLN("ERROR: Time sync failed");
            timeSyncFailureCount = 0;
            isTimeSynced = 0;
        }
    }

    syncTimeFlag--;

    return isTimeSynced;
}

#endif