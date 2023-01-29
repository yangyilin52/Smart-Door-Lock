#include "stm32f10x.h"
#include <stdint.h>
#include "bsp_utils.h"
#include "oled096.h"
#include "keyboard.h"
#include "led.h"
#include "beep.h"
#include "as608.h"
#include "rc522.h"
#include "w25qxx.h"
#include "lock.h"
#include "serialDebug.h"
#include <stdio.h>

typedef enum{
    UserType_Admin,
    UserType_User
}UserType;

typedef enum{
    LedAction_Locked,
    LedAction_Unlocked,
    LedAction_AuthError
}LedAction;

typedef enum{
    Sound_KeyPressed,
    Sound_Unlocked,
    Sound_Locked,
    Sound_AuthError,
    Sound_CardRead,
    Sound_FingerprintCollected,
    Sound_Notice
}Sound;

void removeAllAccounts(void);
uint8_t readFlashPassword(UserType type, uint8_t index, uint8_t* exist, uint8_t* passwdArray);
uint8_t removePassword(UserType type, uint8_t index);
uint8_t setPassword(UserType type, uint8_t index, uint8_t* passwordArray); //Only write password to flash
uint8_t searchPassword(uint8_t* passwordArray, uint8_t* found, UserType* type, uint8_t* index);
uint8_t countAdminAccount(void);
uint8_t readFlashFingerprint(UserType type, uint8_t index, uint8_t* exist, uint8_t* fpPosition);
uint8_t recordFingerprintOp(UserType type, uint8_t index); //A group of operation
uint8_t removeFingerprint(UserType type, uint8_t index);
uint8_t readFlashCard(UserType type, uint8_t index, uint8_t* exist, uint8_t* uidArray);
uint8_t registerCardOp(UserType type, uint8_t index); //A group of operation
uint8_t removeCard(UserType type, uint8_t index);
uint8_t searchCard(uint8_t* uid, uint8_t* found, UserType* type, uint8_t* index);
uint8_t saveNoticeDelay(uint8_t on, uint8_t time);
uint8_t readNoticeDelay(uint8_t* on, uint8_t* time);
void displayWindow(uint8_t wID);
void setLedAction(LedAction action);
void playSound(Sound s);

char str[128];

uint8_t aboutPageLine = 0;
uint8_t aboutPageLineNum = 7;
char aboutText[7][17] = {
    //Page 0
    "< SmartLock >",
    "",
    "Ver:0.2 beta",
    //Page 1
    "Created By",
    "Ath. Y.",
    "2020/Dec/24",
    //Page 3
    "No License"
};

uint8_t otherSettingsLine = 0;
uint8_t otherSettingsLineNum = 4;
char otherSettingsText[4][17] = {
    //Page 0
    "Set Notice",
    "Remove All Users",
    "Factory Reset",
    //Page 1
    "About"
};

uint8_t passwdContainer[10];
uint8_t passwdContainerLength = 0;

uint8_t inputContainer[2];
uint8_t inputContainerLength = 0; //Max length = 2;

uint8_t hiddenMode_w50 = 0;

uint8_t noticeOn = 1;
uint8_t noticeDelay = 10;
uint8_t noticeDelayMin = 10;
uint8_t noticeDelayMax = 60;

uint8_t cursor_backup = 0;
uint8_t cursor = 5;
uint8_t cursorArray[5][4] = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1},
    {0, 0, 0, 0}
};

uint8_t window = 1;
/*  
    window 1: Choose admin/user/other settings
    window 2: Choose Passwords/Fingerprints/IC Cards
    window 3: Passwords list
    window 4: Add password
    window 5: Remove password
    window 6: Fingerprints list
    window 7: Add fingerprint
    window 8: Remove fingerprint
    window 9: Card list
    window 10: Add Card
    window 11: Remove Card
    window 12: Other settings
    window 13: Remove All Accounts
    window 14: About
    window 15: Set notice
    window 16: Factory reset
    window 50: Wait for unlock
    window 51: Wait for enter admin
*/

UserType flag_choosedUserType = UserType_Admin;
uint8_t flag_listBegin = 0;


int main(){
    delay_init();
    OLED096_init();
    Keyboard_init();
    Beep_init();
    Led_init();
    AS608_init();
    RC522_init();
    W25QXX_init();
    Lock_init();
    sDebug_init();
    
    uint8_t key;
    
    readNoticeDelay(&noticeOn, &noticeDelay);
    
    cursor = 3;
    window = 50;
    displayWindow(window);
    setLedAction(LedAction_Locked);
    Lock_setLock(1);
    while(1){
        key = Keyboard_scan();
        if(key != 255){
            playSound(Sound_KeyPressed);
        }
        if(window == 1){ //window 1: Choose admin/user/other settings
            if(key == 1){ //up
                if(cursor > 0){
                    cursor--;
                    displayWindow(window);
                }
            }
            else if(key == 7){ //down
                if(cursor < 2){
                    cursor++;
                    displayWindow(window);
                }
            }
            else if(key == 11){ //enter
                if(cursor == 0){
                    flag_choosedUserType = UserType_Admin;
                    window = 2;
                    cursor = 0;
                    displayWindow(window);
                }
                else if(cursor == 1){
                    flag_choosedUserType = UserType_User;
                    window = 2;
                    cursor = 0;
                    displayWindow(window);
                }
                else if(cursor == 2){
                    window = 12;
                    otherSettingsLine = 0;
                    cursor = 0;
                    displayWindow(window);
                }
            }
            else if(key == 9){ //back
                window = 50;
                hiddenMode_w50 = 0;
                passwdContainerLength = 0;
                cursor = 3;
                displayWindow(50);
            }
        }
        else if(window == 2){ //window 2: Choose Passwords/Fingerprints/IC Cards
            if(key == 1){ //up
                if(cursor > 0){
                    cursor--;
                    displayWindow(window);
                }
            }
            else if(key == 7){ //down
                if(cursor < 2){
                    cursor++;
                    displayWindow(window);
                }
            }
            else if(key == 9){ //back
                window = 1;
                if(flag_choosedUserType == UserType_Admin){
                    cursor = 0;
                }
                else if(flag_choosedUserType == UserType_User){
                    cursor = 1;
                }
                displayWindow(window);
            }
            else if(key == 11){ //enter
                if(cursor == 0){
                    window = 3;
                    cursor = 0;
                    flag_listBegin = 0;
                    displayWindow(window);
                }
                else if(cursor == 1){
                    window = 6;
                    cursor = 0;
                    flag_listBegin = 0;
                    displayWindow(window);
                }
                else if(cursor == 2){
                    window = 9;
                    cursor = 0;
                    flag_listBegin = 0;
                    displayWindow(window);
                }
            }
        }
        else if(window == 3){ //window 3: Passwords list
            if(key == 1){ //up
                if(cursor > 0){
                    cursor--;
                    displayWindow(window);
                }
            }
            else if(key == 7){ //down
                if(cursor < 2){
                    cursor++;
                    displayWindow(window);
                }
            }
            else if(key == 3){ //left
                if(flag_listBegin >= 3){
                    flag_listBegin -= 3;
                    cursor = 0;
                    displayWindow(window);
                }
            }
            else if(key == 5){ //right
                if(flag_listBegin <= 24){
                    flag_listBegin += 3;
                    cursor = 0;
                    displayWindow(window);
                }
            }
            else if(key == 9){ //back
                window = 2;
                cursor = 0;
                displayWindow(window);
            }
            else if(key == 11){ //enter
                uint8_t exist;
                uint8_t passwd[5];
                readFlashPassword(flag_choosedUserType, flag_listBegin + cursor, &exist, passwd);
                if(exist == 0){
                    window = 4;
                    cursor_backup = cursor;
                    cursor = 4;
                    passwdContainerLength = 0;
                    displayWindow(window);
                }
                else if(exist == 1){
                    window = 5;
                    cursor_backup = cursor;
                    cursor = 4;
                    displayWindow(window);
                }
            }
        }
        else if(window == 4){ //window 4: Add password
            if(key >= 0 && key <= 8){ //1 to 9
                if(passwdContainerLength < 10){
                    passwdContainer[passwdContainerLength] = key + 1;
                    passwdContainerLength++;
                    displayWindow(window);
                }
                
            }
            else if(key == 10){ //0
                if(passwdContainerLength < 10){
                    passwdContainer[passwdContainerLength] = 0;
                    passwdContainerLength++;
                    displayWindow(window);
                }
                
            }
            else if(key == 9){ // back
                window = 3;
                cursor = cursor_backup;
                displayWindow(window);
            }
            else if(key == 11){ //enter
                if(passwdContainerLength >= 4){
                    uint8_t passwd[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
                    for(uint8_t i = 0; i < 5; i++){
                        if(i * 2 < passwdContainerLength){
                            passwd[i] += (passwdContainer[i * 2] << 4);
                        }
                        else{
                            passwd[i] += 0xF0;
                        }
                        if(i * 2 + 1 < passwdContainerLength){
                            passwd[i] += passwdContainer[i * 2 + 1];
                        }
                        else{
                            passwd[i] += 0x0F;
                        }
                    }
                    setPassword(flag_choosedUserType, flag_listBegin + cursor_backup, passwd);
                    OLED096_clearScreen();
                    OLED096_writeLine(1, OLED096_Align_Center, 0, "Password Set!");
                    delay_ms(1000);
                    window = 3;
                    cursor = cursor_backup;
                    displayWindow(window);
                }
                else{
                    OLED096_clearScreen();
                    OLED096_writeLine(1, OLED096_Align_Center, 0, "Password");
                    OLED096_writeLine(2, OLED096_Align_Center, 0, "Too Short");
                    delay_ms(1000);
                    displayWindow(window);
                }
            }
        }
        else if(window == 5){ //window 5: Remove password
            if(key == 9){ //No
                window = 3;
                cursor = cursor_backup;
                displayWindow(window);
            }
            else if(key == 11){ //Yes
                removePassword(flag_choosedUserType, flag_listBegin + cursor_backup);
                OLED096_clearScreen();
                OLED096_writeLine(1, OLED096_Align_Center, 0, "Password");
                OLED096_writeLine(2, OLED096_Align_Center, 0, "Deleted!");
                delay_ms(1000);
                window = 3;
                cursor = cursor_backup;
                displayWindow(window);
            }
        }
        else if(window == 6){ //window 6: Fingerprints list
            if(key == 1){ //up
                if(cursor > 0){
                    cursor--;
                    displayWindow(window);
                }
            }
            else if(key == 7){ //down
                if(cursor < 2){
                    cursor++;
                    displayWindow(window);
                }
            }
            else if(key == 3){ //left
                if(flag_listBegin >= 3){
                    flag_listBegin -= 3;
                    cursor = 0;
                    displayWindow(window);
                }
            }
            else if(key == 5){ //right
                if(flag_listBegin <= 24){
                    flag_listBegin += 3;
                    cursor = 0;
                    displayWindow(window);
                }
            }
            else if(key == 9){ //back
                window = 2;
                cursor = 1;
                displayWindow(window);
            }
            else if(key == 11){ //enter
                uint8_t exist;
                uint8_t fpPosition;
                readFlashFingerprint(flag_choosedUserType, flag_listBegin + cursor, &exist, &fpPosition);
                if(exist == 0){
                    window = 7;
                    cursor_backup = cursor;
                    cursor = 4;
                    displayWindow(window);
                }
                else if(exist == 1){
                    window = 8;
                    cursor_backup = cursor;
                    cursor = 4;
                    displayWindow(window);
                }
            }
        }
        else if(window == 7){ //window 7: Add fingerprint
            if(key == 9){ //back
                window = 6;
                cursor = cursor_backup;
                displayWindow(window);
            }
            else if(key == 11){ //next
                window = 6;
                recordFingerprintOp(flag_choosedUserType, flag_listBegin + cursor_backup);
                cursor = cursor_backup;
                displayWindow(window);
            }
        }
        else if(window == 8){ //window 8: Remove fingerprint
            if(key == 9){ //No
                window = 6;
                cursor = cursor_backup;
                displayWindow(window);
            }
            else if(key == 11){ //Yes
                removeFingerprint(flag_choosedUserType, flag_listBegin + cursor_backup);
                OLED096_clearScreen();
                OLED096_writeLine(1, OLED096_Align_Center, 0, "Fingerprint");
                OLED096_writeLine(2, OLED096_Align_Center, 0, "Deleted!");
                delay_ms(1000);
                window = 6;
                cursor = cursor_backup;
                displayWindow(window);
            }
        }
        else if(window == 9){ //window 9: Card list
            if(key == 1){ //up
                if(cursor > 0){
                    cursor--;
                    displayWindow(window);
                }
            }
            else if(key == 7){ //down
                if(cursor < 2){
                    cursor++;
                    displayWindow(window);
                }
            }
            else if(key == 3){ //left
                if(flag_listBegin >= 3){
                    flag_listBegin -= 3;
                    cursor = 0;
                    displayWindow(window);
                }
            }
            else if(key == 5){ //right
                if(flag_listBegin <= 24){
                    flag_listBegin += 3;
                    cursor = 0;
                    displayWindow(window);
                }
            }
            else if(key == 9){ //back
                window = 2;
                cursor = 2;
                displayWindow(window);
            }
            else if(key == 11){ //enter
                uint8_t exist;
                uint8_t uid[4];
                readFlashCard(flag_choosedUserType, flag_listBegin + cursor, &exist, uid);
                if(exist == 0){
                    window = 10;
                    cursor_backup = cursor;
                    cursor = 4;
                    displayWindow(window);
                }
                else if(exist == 1){
                    window = 11;
                    cursor_backup = cursor;
                    cursor = 4;
                    displayWindow(window);
                }
            }
        }
        else if(window == 10){ //window 10: Add Card
            if(key == 9){ //back
                window = 9;
                cursor = cursor_backup;
                displayWindow(window);
            }
            else if(key == 11){ //next
                registerCardOp(flag_choosedUserType, flag_listBegin + cursor_backup);
                window = 9;
                cursor = cursor_backup;
                displayWindow(window);
            }
        }
        else if(window == 11){ //window 11: Remove Card
            if(key == 9){ //No
                window = 9;
                cursor = cursor_backup;
                displayWindow(window);
            }
            else if(key == 11){ //Yes
                removeCard(flag_choosedUserType, flag_listBegin + cursor_backup);
                OLED096_clearScreen();
                OLED096_writeLine(1, OLED096_Align_Center, 0, "Card Removed!");
                delay_ms(1000);
                window = 9;
                cursor = cursor_backup;
                displayWindow(window);
            }
        }
        else if(window == 12){ //window 12: Other settings
            if(key == 1){ //up
                if(otherSettingsLine % 3 > 0){
                    otherSettingsLine--;
                    cursor--;
                    displayWindow(window);
                }
            }
            else if(key == 7){ //down
                if((otherSettingsLine + 1 < otherSettingsLineNum) && (otherSettingsLine % 3 != 2)){
                    otherSettingsLine++;
                    cursor++;
                    displayWindow(window);
                }
            }
            else if(key == 3){ //left
                if(otherSettingsLine >= 3){
                    otherSettingsLine = (otherSettingsLine / 3 - 1) * 3;
                    cursor = 0;
                    displayWindow(window);
                }
            }
            else if(key == 5){ //right
                if((otherSettingsLine / 3 + 1) * 3 < otherSettingsLineNum){
                    otherSettingsLine = (otherSettingsLine / 3 + 1) * 3;
                    cursor = 0;
                    displayWindow(window);
                }
            }
            else if(key == 9){ //back
                window = 1;
                cursor = 2;
                displayWindow(window);
            }
            else if(key == 11){ //enter
                if(otherSettingsLine == 0){ //Set notice
                    window = 15;
                    cursor = 0;
                    displayWindow(window);
                }
                else if(otherSettingsLine == 1){ //Remove All Users
                    window = 13;
                    cursor = 4;
                    displayWindow(window);
                }
                else if(otherSettingsLine == 2){ //Factory Reset
                    window = 16;
                    cursor = 4;
                    displayWindow(window);
                }
                else if(otherSettingsLine == 3){ //About
                    window = 14;
                    cursor = 0;
                    aboutPageLine = 0;
                    displayWindow(window);
                }
            }
        }
        else if(window == 13){ //window 13: Remove All Accounts
            if(key == 9){ //No
                window = 12;
                cursor = otherSettingsLine % 3;
                displayWindow(window);
            }
            else if(key == 11){ //Yes
                OLED096_clearScreen();
                OLED096_writeLine(1, OLED096_Align_Center, 0, "Removing All");
                OLED096_writeLine(2, OLED096_Align_Center, 0, "Accounts...");
                removeAllAccounts();
                OLED096_clearScreen();
                OLED096_writeLine(1, OLED096_Align_Center, 0, "Done.");
                delay_ms(1500);
                window = 12;
                cursor = otherSettingsLine % 3;
                displayWindow(window);
            }
        }
        else if(window == 14){ //window 14: About
            if(key == 9){ //Back
                window = 12;
                cursor = otherSettingsLine % 3;
                displayWindow(window);
            }
            else if(key == 1 || key == 3){ //Previous Page
                if(aboutPageLine >= 3){
                    aboutPageLine -= 3;
                    displayWindow(window);
                }
            }
            else if(key == 5 || key == 7){ //Next Page
                if(aboutPageLine + 3 < aboutPageLineNum){
                    aboutPageLine += 3;
                    displayWindow(window);
                }
            }
        }
        else if(window == 15){//Set notice
            if(key == 1){ //up
                if(cursor > 0){
                    cursor--;
                    displayWindow(window);
                }
            }
            else if(key == 7){ //down
                if(cursor < 1){
                    cursor++;
                    displayWindow(window);
                }
            }
            else if(key == 3){ //left
                if(cursor == 0){
                    if(noticeOn == 0){
                        noticeOn = 1;
                    }
                    else if(noticeOn == 1){
                        noticeOn = 0;
                    }
                    displayWindow(window);
                }
                else if(cursor == 1){
                    if(noticeOn == 1 && noticeDelay - 10 >= noticeDelayMin){
                        noticeDelay -= 10;
                        displayWindow(window);
                    }
                }
            }
            else if(key == 5){ //right
                if(cursor == 0){
                    if(noticeOn == 0){
                        noticeOn = 1;
                    }
                    else if(noticeOn == 1){
                        noticeOn = 0;
                    }
                    displayWindow(window);
                }
                else if(cursor == 1){
                    if(noticeOn == 1 && noticeDelay + 10 <= noticeDelayMax){
                        noticeDelay += 10;
                        displayWindow(window);
                    }
                }
            }
            else if(key == 9){ //back
                readNoticeDelay(&noticeOn, &noticeDelay);
                window = 12;
                cursor = otherSettingsLine % 3;
                displayWindow(window);
            }
            else if(key == 11){ //save
                saveNoticeDelay(noticeOn, noticeDelay);
                OLED096_clearScreen();
                OLED096_writeLine(1, OLED096_Align_Center, 0, "Saved.");
                delay_ms(1000);
                window = 12;
                cursor = otherSettingsLine % 3;
                displayWindow(window);
            }
        }
        else if(window == 16){ //window16: Factory reset
            if(key == 9){ //No
                window = 12;
                cursor = otherSettingsLine % 3;
                displayWindow(window);
            }
            else if(key == 11){ //Yes
                OLED096_clearScreen();
                OLED096_writeLine(1, OLED096_Align_Center, 0, "Resetting");
                OLED096_writeLine(2, OLED096_Align_Center, 0, "All Profile...");
                removeAllAccounts();
                saveNoticeDelay(1, 10);
                readNoticeDelay(&noticeOn, &noticeDelay);
                OLED096_clearScreen();
                OLED096_writeLine(1, OLED096_Align_Center, 0, "Done.");
                delay_ms(1500);
                window = 12;
                cursor = otherSettingsLine % 3;
                displayWindow(window);
            }
        }
        else if(window == 50){ //window50: Wait for unlock
            uint8_t uid[4];
            uint8_t cardType[2];
            if(AS608_isFingerAvailable() == 1){
                if(hiddenMode_w50 == 0){
                    delay_ms(200);
                    if(AS608_isFingerAvailable() == 1){
                        delay_ms(50); //Wait for finger position stable
                        uint8_t success = 0;
                        uint16_t receivedPageID;
                        uint16_t receivedMatchScore;
                        if(AS608_PS_GetImage() == 0){
                            if(AS608_PS_GenChar(1) == 0){
                                if(AS608_PS_Search(1, 0, 60, &receivedPageID, &receivedMatchScore) == 0){
                                    success = 1;
                                }
                            }
                        }
                        playSound(Sound_FingerprintCollected); //120ms
                        if(success == 1){
                            OLED096_clearScreen();
                            OLED096_writeLine(1, OLED096_Align_Center, 0, "Door Unlocked");
                            if(receivedPageID < 30){
                                sprintf(str, "(Admin FP%02u)", receivedPageID + 1); 
                            }
                            else if(receivedPageID >= 30 && receivedPageID < 60){
                                sprintf(str, "(User FP%02u)", receivedPageID - 30 + 1); 
                            }
                            OLED096_writeLine(2, OLED096_Align_Center, 0, str);
                            
                            setLedAction(LedAction_Unlocked);
                            Lock_setLock(0);
                            delay_ms(500);
                            playSound(Sound_Unlocked); //300ms
                            uint8_t cancelNotice = 0;
                            for(uint8_t i = 0; i < 32; i++){ //delay about 3200ms
                                if(Keyboard_scan() == 0 && cancelNotice != 1){
                                    cancelNotice = 1;
                                    OLED096_writeLine(3, OLED096_Align_Center, 0, "*Ignored*");
                                }
                                delay_ms(100);
                            }
                            setLedAction(LedAction_Locked);
                            Lock_setLock(1);
                            playSound(Sound_Locked); //300ms
                            if(noticeOn == 1 && cancelNotice != 1){
                                for(uint32_t i = 0; i < (uint32_t)noticeDelay * 100; i++){
                                    if(Lock_getDoorStatus() == Lock_DoorStatus_Closed){
                                        cancelNotice = 1;
                                        break;
                                    }
                                    delay_ms(10);
                                }
                                if(cancelNotice != 1){
                                    while(cancelNotice != 1){
                                        playSound(Sound_Notice);
                                        for(uint32_t i = 0; i < 8; i++){ //delay about 800ms
                                            if(Lock_getDoorStatus() == Lock_DoorStatus_Closed){
                                                cancelNotice = 1;
                                                break;
                                            }
                                        delay_ms(100);
                                        }
                                    }
                                }
                            }
                            RC522_pcdReset(); //To Fix Bug that RC522 being Affected by EMF
                            passwdContainerLength = 0;
                            displayWindow(window);
                        }
                        else if(success == 0){
                            OLED096_clearScreen();
                            OLED096_writeLine(1, OLED096_Align_Center, 0, "Invalid");
                            OLED096_writeLine(2, OLED096_Align_Center, 0, "Fingerprint");
                            setLedAction(LedAction_AuthError);
                            delay_ms(500);
                            playSound(Sound_AuthError); //800ms
                            delay_ms(2000);
                            setLedAction(LedAction_Locked);
                            passwdContainerLength = 0;
                            displayWindow(window);
                        }
                    }
                }
            }
            else if(RC522_pcdRequest(PICC_REQIDL, cardType) == MI_OK){
                if(hiddenMode_w50 == 0){
                    RC522_pcdRequest(PICC_REQIDL, cardType);
                    delay_ms(300);
                    if(RC522_pcdRequest(PICC_REQIDL, cardType) == MI_OK){
                        if(RC522_pcdAnticoll(uid) == MI_OK){
                            playSound(Sound_CardRead); //250ms
                            uint8_t found;
                            UserType type;
                            uint8_t cardIndex;
                            searchCard(uid, &found, &type, &cardIndex);
                            if(found == 1){
                                OLED096_clearScreen();
                                OLED096_writeLine(1, OLED096_Align_Center, 0, "Door Unlocked");
                                if(type == UserType_Admin){
                                    sprintf(str, "(Admin CD%02u)", cardIndex + 1); 
                                }
                                else if(type == UserType_User){
                                    sprintf(str, "(User CD%02u)", cardIndex + 1); 
                                }
                                OLED096_writeLine(2, OLED096_Align_Center, 0, str);
                                setLedAction(LedAction_Unlocked);
                                Lock_setLock(0);
                                delay_ms(500);
                                playSound(Sound_Unlocked); //300ms
                                uint8_t cancelNotice = 0;
                                for(uint8_t i = 0; i < 32; i++){ //delay about 3200ms
                                    if(Keyboard_scan() == 0 && cancelNotice != 1){
                                        cancelNotice = 1;
                                        OLED096_writeLine(3, OLED096_Align_Center, 0, "*Ignored*");
                                    }
                                    delay_ms(100);
                                }
                                setLedAction(LedAction_Locked);
                                Lock_setLock(1);
                                playSound(Sound_Locked); //300ms
                                if(noticeOn == 1 && cancelNotice != 1){
                                    for(uint32_t i = 0; i < (uint32_t)noticeDelay * 100; i++){
                                        if(Lock_getDoorStatus() == Lock_DoorStatus_Closed){
                                            cancelNotice = 1;
                                            break;
                                        }
                                        delay_ms(10);
                                    }
                                    if(cancelNotice != 1){
                                        while(cancelNotice != 1){
                                            playSound(Sound_Notice);
                                            for(uint32_t i = 0; i < 8; i++){ //delay about 800ms
                                                if(Lock_getDoorStatus() == Lock_DoorStatus_Closed){
                                                    cancelNotice = 1;
                                                    break;
                                                }
                                                delay_ms(100);
                                            }
                                        }
                                    }
                                }
                                RC522_pcdReset(); //To Fix Bug that RC522 being Affected by EMF
                                passwdContainerLength = 0;
                                displayWindow(window);
                            }
                            else{
                                OLED096_clearScreen();
                                OLED096_writeLine(1, OLED096_Align_Center, 0, "Invalid Card");
                                setLedAction(LedAction_AuthError);
                                delay_ms(500);
                                playSound(Sound_AuthError); //800ms
                                delay_ms(2000);
                                setLedAction(LedAction_Locked);
                                passwdContainerLength = 0;
                                displayWindow(window);
                            }
                        }
                    }
                }
            }
            else if(key == 9){ //back
                if(passwdContainerLength > 0){
                    passwdContainerLength--;
                }
                else{
                    if(hiddenMode_w50 == 0){
                        hiddenMode_w50 = 1;
                        inputContainerLength = 0;
                    }
                    else{
                        hiddenMode_w50 = 0;
                    }
                }
                displayWindow(window);
            }
            else if(key == 11){ //enter
                if(hiddenMode_w50 == 0){
                    uint8_t success = 0;
                    UserType type;
                    uint8_t passwdIndex;
                    if(passwdContainerLength >= 4){
                        uint8_t passwd[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
                        for(uint8_t i = 0; i < 5; i++){
                            if(i * 2 < passwdContainerLength){
                                passwd[i] += (passwdContainer[i * 2] << 4);
                            }
                            else{
                                passwd[i] += 0xF0;
                            }
                            if(i * 2 + 1 < passwdContainerLength){
                                passwd[i] += passwdContainer[i * 2 + 1];
                            }
                            else{
                                passwd[i] += 0x0F;
                            }
                        }
                        uint8_t found;
                        searchPassword(passwd, &found, &type, &passwdIndex);
                        if(found == 1){
                            success = 1;
                        }
                    }
                    if(success == 0){
                        OLED096_clearScreen();
                        OLED096_writeLine(1, OLED096_Align_Center, 0, "Wrong");
                        OLED096_writeLine(2, OLED096_Align_Center, 0, "Password");
                        setLedAction(LedAction_AuthError);
                        delay_ms(500);
                        playSound(Sound_AuthError); //800ms
                        delay_ms(2000);
                        setLedAction(LedAction_Locked);
                        passwdContainerLength = 0;
                        displayWindow(window);
                    }
                    else{
                        OLED096_clearScreen();
                        OLED096_writeLine(1, OLED096_Align_Center, 0, "Door Unlocked");
                        if(type == UserType_Admin){
                            sprintf(str, "(Admin PW%02u)", passwdIndex + 1);
                        }
                        else if(type == UserType_User){
                            sprintf(str, "(User PW%02u)", passwdIndex + 1);
                        }
                        OLED096_writeLine(2, OLED096_Align_Center, 0, str);
                        setLedAction(LedAction_Unlocked);
                        Lock_setLock(0);
                        delay_ms(500);
                        playSound(Sound_Unlocked); //300ms
                        uint8_t cancelNotice = 0;
                        for(uint8_t i = 0; i < 32; i++){ //delay about 3200ms
                            if(Keyboard_scan() == 0 && cancelNotice != 1){
                                cancelNotice = 1;
                                OLED096_writeLine(3, OLED096_Align_Center, 0, "*Ignored*");
                            }
                            delay_ms(100);
                        }
                        setLedAction(LedAction_Locked);
                        Lock_setLock(1);
                        playSound(Sound_Locked); //300ms
                        if(noticeOn == 1 && cancelNotice != 1){
                            for(uint32_t i = 0; i < (uint32_t)noticeDelay * 100; i++){
                                if(Lock_getDoorStatus() == Lock_DoorStatus_Closed){
                                    cancelNotice = 1;
                                    break;
                                }
                                delay_ms(10);
                            }
                            if(cancelNotice != 1){
                                while(cancelNotice != 1){
                                    playSound(Sound_Notice);
                                    for(uint32_t i = 0; i < 8; i++){ //delay about 800ms
                                        if(Lock_getDoorStatus() == Lock_DoorStatus_Closed){
                                            cancelNotice = 1;
                                            break;
                                        }
                                        delay_ms(100);
                                    }
                                }
                            }
                        }
                        RC522_pcdReset(); //To Fix Bug that RC522 being Affected by EMF
                        passwdContainerLength = 0;
                        displayWindow(window);
                    }
                }
                else{
                    uint8_t codeValid = 0;
                    uint8_t code = 0;
                    if(inputContainerLength == 2){
                        codeValid = 1;
                        code = inputContainer[0] * 10 + inputContainer[1];
                    }
                    if(codeValid == 0){
                        hiddenMode_w50 = 0;
                        displayWindow(window);
                    }
                    else{
                        if(code == 0){ //C00: Jump to Admin Authenticate(window 51)
                            window = 51;
                            displayWindow(window);
                        }
                        else{
                            hiddenMode_w50 = 0;
                            displayWindow(window);
                        }
                    }
                }
            }
            else if(key >= 0 && key <= 8){ //1 to 9
                if(hiddenMode_w50 == 0){
                    if(passwdContainerLength < 10){
                        passwdContainer[passwdContainerLength] = key + 1;
                        passwdContainerLength++;
                    }
                }
                else{
                    if(inputContainerLength < 2){
                        inputContainer[inputContainerLength] = key + 1;
                        inputContainerLength++;
                    }
                }
                displayWindow(window);
            }
            else if(key == 10){ //0
                if(hiddenMode_w50 == 0){
                    if(passwdContainerLength < 10){
                        passwdContainer[passwdContainerLength] = 0;
                        passwdContainerLength++;
                    }
                }
                else{
                    if(inputContainerLength < 2){
                        inputContainer[inputContainerLength] = 0;
                        inputContainerLength++;
                    }
                }
                displayWindow(window);
            }
        }
        else if(window == 51){ //window51: Wait for enter admin
            uint8_t uid[4];
            uint8_t cardType[2];
            if(AS608_isFingerAvailable() == 1){
                delay_ms(200);
                if(AS608_isFingerAvailable() == 1){
                    delay_ms(50); //Wait for finger position stable
                    uint8_t success = 0;
                    uint16_t receivedPageID;
                    uint16_t receivedMatchScore;
                    if(AS608_PS_GetImage() == 0){
                        if(AS608_PS_GenChar(1) == 0){
                            if(AS608_PS_Search(1, 0, 60, &receivedPageID, &receivedMatchScore) == 0){
                                success = 1;
                            }
                        }
                    }
                    playSound(Sound_FingerprintCollected); //120ms
                    if(success == 1 && receivedPageID < 30){
                        OLED096_clearScreen();
                        OLED096_writeLine(1, OLED096_Align_Center, 0, "Authenticated");
                        sprintf(str, "(Admin FP%02u)", receivedPageID + 1);
                        OLED096_writeLine(2, OLED096_Align_Center, 0, str);
                        delay_ms(2000);
                        window = 1;
                        cursor = 0;
                        displayWindow(window);
                    }
                    else{
                        OLED096_clearScreen();
                        OLED096_writeLine(1, OLED096_Align_Center, 0, "Invalid");
                        OLED096_writeLine(2, OLED096_Align_Center, 0, "Fingerprint");
                        delay_ms(2000);
                        passwdContainerLength = 0;
                        displayWindow(window);
                    }
                }
            }
            else if(RC522_pcdRequest(PICC_REQIDL, cardType) == MI_OK){
                RC522_pcdRequest(PICC_REQIDL, cardType);
                delay_ms(300);
                if(RC522_pcdRequest(PICC_REQIDL, cardType) == MI_OK){
                    if(RC522_pcdAnticoll(uid) == MI_OK){
                        playSound(Sound_CardRead); //250ms
                        uint8_t found;
                        UserType type;
                        uint8_t cardIndex;
                        searchCard(uid, &found, &type, &cardIndex);
                        if(found == 1 && type == UserType_Admin){
                            OLED096_clearScreen();
                            OLED096_writeLine(1, OLED096_Align_Center, 0, "Authenticated");
                            sprintf(str, "(Admin CD%02u)", cardIndex + 1); 
                            OLED096_writeLine(2, OLED096_Align_Center, 0, str);
                            delay_ms(2000);
                            window = 1;
                            cursor = 0;
                            displayWindow(window);
                        }
                        else{
                            OLED096_clearScreen();
                            OLED096_writeLine(1, OLED096_Align_Center, 0, "Invalid Card");
                            delay_ms(2000);
                            passwdContainerLength = 0;
                            displayWindow(window);
                        }
                    }
                }
            }
            else if(key == 9){ //back
                if(passwdContainerLength > 0){
                    passwdContainerLength--;
                    displayWindow(window);
                }
                else{
                    window = 50;
                    hiddenMode_w50 = 0;
                    displayWindow(window);
                }
            }
            else if(key == 11){ //enter
                uint8_t found = 0;
                UserType type;
                uint8_t passwdIndex;
                if(passwdContainerLength >= 4){
                    uint8_t passwd[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
                    for(uint8_t i = 0; i < 5; i++){
                        if(i * 2 < passwdContainerLength){
                            passwd[i] += (passwdContainer[i * 2] << 4);
                        }
                        else{
                            passwd[i] += 0xF0;
                        }
                        if(i * 2 + 1 < passwdContainerLength){
                            passwd[i] += passwdContainer[i * 2 + 1];
                        }
                        else{
                            passwd[i] += 0x0F;
                        }
                    }
                    searchPassword(passwd, &found, &type, &passwdIndex);
                }
                if(countAdminAccount() == 0){
                    uint8_t defaultPasswd[5] = {0x12, 0x34, 0x56, 0x78, 0xFF};
                    uint8_t passwd[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
                    for(uint8_t i = 0; i < 5; i++){
                        if(i * 2 < passwdContainerLength){
                            passwd[i] += (passwdContainer[i * 2] << 4);
                        }
                        else{
                            passwd[i] += 0xF0;
                        }
                        if(i * 2 + 1 < passwdContainerLength){
                            passwd[i] += passwdContainer[i * 2 + 1];
                        }
                        else{
                            passwd[i] += 0x0F;
                        }
                    }
                    uint8_t diff = 0;
                    for(uint8_t i = 0; i < 5; i++){
                        if(passwd[i] != defaultPasswd[i]){
                            diff++;
                            break;
                        }
                    }
                    if(diff == 0){
                        OLED096_clearScreen();
                        OLED096_writeLine(1, OLED096_Align_Center, 0, "Authenticated");
                        OLED096_writeLine(2, OLED096_Align_Center, 0, "(Default)");
                        delay_ms(2000);
                        window = 1;
                        cursor = 0;
                        displayWindow(window);
                    }
                    else{
                        OLED096_clearScreen();
                        OLED096_writeLine(1, OLED096_Align_Center, 0, "Wrong");
                        OLED096_writeLine(2, OLED096_Align_Center, 0, "Password");
                        delay_ms(2000);
                        passwdContainerLength = 0;
                        displayWindow(window);
                    }
                }
                else{
                    if(found == 1 && type == UserType_Admin){
                        OLED096_clearScreen();
                        OLED096_writeLine(1, OLED096_Align_Center, 0, "Authenticated");
                        sprintf(str, "(Admin PW%02u)", passwdIndex + 1);
                        OLED096_writeLine(2, OLED096_Align_Center, 0, str);
                        delay_ms(2000);
                        window = 1;
                        cursor = 0;
                        displayWindow(window);
                    }
                    else{
                        OLED096_clearScreen();
                        OLED096_writeLine(1, OLED096_Align_Center, 0, "Wrong");
                        OLED096_writeLine(2, OLED096_Align_Center, 0, "Password");
                        delay_ms(2000);
                        passwdContainerLength = 0;
                        displayWindow(window);
                    }
                }
            }
            else if(key >= 0 && key <= 8){ //1 to 9
                if(passwdContainerLength < 10){
                    passwdContainer[passwdContainerLength] = key + 1;
                    passwdContainerLength++;
                }
                displayWindow(window);
            }
            else if(key == 10){ //0
                if(passwdContainerLength < 10){
                    passwdContainer[passwdContainerLength] = 0;
                    passwdContainerLength++;
                }
                displayWindow(window);
            }
        }
    }
}

void removeAllAccounts(){
    //Remove all items from flash
    uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint32_t addr = 0x00000000;
    uint16_t i;
    for(i = 0; i < 256 * 3 * 2 / 8; i++){
        W25QXX_write(data, addr, 8);
        addr += 0x08;
    }
    //Remove fingerprints from module
    AS608_PS_DeleteChar(0, 60);
}

uint8_t readFlashPassword(UserType type, uint8_t index, uint8_t* exist, uint8_t* passwdArray){
    if(index < 30){
        uint8_t data[8];
        uint32_t addr;
        if(type == UserType_Admin){
            addr = 0x00000000 + 0x08 * (uint32_t)index;
        }
        else if(type == UserType_User){
            addr = 0x00000300 + 0x08 * (uint32_t)index; 
        }
        W25QXX_read(data, addr, 8);
        if(data[0] == 0x00){
            *exist = 0;
        }
        else if(data[0] == 0xFF){
            *exist = 1;
            for(uint8_t i = 0; i < 5; i++){
                passwdArray[i] = data[3 + i];
            }
        }
        return 0;
    }
    else{
        return 1;
    }
}

uint8_t removePassword(UserType type, uint8_t index){
    if(index < 30){
        uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint32_t addr;
        if(type == UserType_Admin){
            addr = 0x00000000 + 0x08 * (uint32_t)index;
        }
        else if(type == UserType_User){
            addr = 0x00000300 + 0x08 * (uint32_t)index; 
        }
        W25QXX_write(data, addr, 8);
        return 0;
    }
    else{
        return 1;
    }
}

uint8_t setPassword(UserType type, uint8_t index, uint8_t* passwordArray){
    if(index < 30){
        uint8_t data[8] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        for(uint8_t i = 0; i < 5; i++){
            data[3 + i] = passwordArray[i];
        }
        uint32_t addr;
        if(type == UserType_Admin){
            addr = 0x00000000 + 0x08 * (uint32_t)index;
        }
        else if(type == UserType_User){
            addr = 0x00000300 + 0x08 * (uint32_t)index; 
        }
        W25QXX_write(data, addr, 8);
        return 0;
    }
    else{
        return 1;
    }
}

uint8_t searchPassword(uint8_t* passwordArray, uint8_t* found, UserType* type, uint8_t* index){
    uint8_t data[8];
    uint32_t addr;
    uint8_t f = 0;
    //Search Admin
    if(f == 0){
        addr = 0x00000000;
        for(uint8_t i = 0; i < 30; i++){
            W25QXX_read(data, addr, 8);
            if(data[0] == 0xFF){
                uint8_t diff = 0;
                for(uint8_t j = 0; j < 5; j++){
                    if(passwordArray[j] != data[3 + j]){
                        diff++;
                        break;
                    }
                }
                if(diff == 0){
                    f = 1;
                    (*type) = UserType_Admin;
                    (*index) = i;
                    break;
                }
            }
            addr += 0x08;
        }
    }
    //Search User
    if(f == 0){
        addr = 0x00000300;
        for(uint8_t i = 0; i < 30; i++){
            W25QXX_read(data, addr, 8);
            if(data[0] == 0xFF){
                uint8_t diff = 0;
                for(uint8_t j = 0; j < 5; j++){
                    if(passwordArray[j] != data[3 + j]){
                        diff++;
                        break;
                    }
                }
                if(diff == 0){
                    f = 1;
                    (*type) = UserType_User;
                    (*index) = i;
                    break;
                }
            }
            addr += 0x08;
        }
    }
    (*found) = f;
    return 0;
}

uint8_t countAdminAccount(){
    uint8_t data[8];
    uint32_t addr;
    uint8_t count = 0;
    for(uint8_t i = 0; i < 3; i++){
        addr = 0x00000000 + 0x00000100 * (uint32_t)i;
        for(uint8_t j = 0; j < 30; j++){
            W25QXX_read(data, addr, 8);
            if(data[0] == 0xFF){
                count++;
            }
            addr += 0x08;
        }
    }
    return count;
}

uint8_t readFlashFingerprint(UserType type, uint8_t index, uint8_t* exist, uint8_t* fpPosition){
    if(index < 30){
        uint8_t data[8];
        uint32_t addr;
        if(type == UserType_Admin){
            addr = 0x00000000 + 0x00000100 + 0x08 * (uint32_t)index;
        }
        else if(type == UserType_User){
            addr = 0x00000300 + 0x00000100 + 0x08 * (uint32_t)index;
        }
        W25QXX_read(data, addr, 8);
        if(data[0] == 0x00){
            *exist = 0;
        }
        else if(data[0] == 0xFF){
            *exist = 1;
            *fpPosition = data[7];
        }
        return 0;
    }
    else{
        return 1;
    }
}

uint8_t recordFingerprintOp(UserType type, uint8_t index){
    if(index < 30){
        for(uint8_t i = 0; i < 2; i++){
            while(1){
                OLED096_clearScreen();
                OLED096_writeLine(1, OLED096_Align_Center, 0, "Press Finger");
                sprintf(str, "on Sensor (%u/2)", i + 1);
                OLED096_writeLine(2, OLED096_Align_Center, 0, str);
                while(1){
                    if(AS608_isFingerAvailable() == 1){
                        delay_ms(200);
                        if(AS608_isFingerAvailable() == 1){
                            break;
                        }
                    }
                }
                OLED096_clearScreen();
                OLED096_writeLine(1, OLED096_Align_Center, 0, "Recording...");
                delay_ms(800); //Wait for finger position stable
                uint8_t code = 0;
                code += AS608_PS_GetImage();
                code += AS608_PS_GenChar(i + 1);
                playSound(Sound_FingerprintCollected); //120ms
                if(code == 0){
                    OLED096_writeLine(2, OLED096_Align_Center, 0, "Done.");
                    delay_ms(1000);
                    /*
                    while(1){
                        if(AS608_isFingerAvailable() == 0){
                            delay_ms(200);
                            if(AS608_isFingerAvailable() == 0){
                                break;
                            }
                        }
                    }
                    */
                    break;
                }
                else{
                    OLED096_writeLine(2, OLED096_Align_Center, 0, "Failed.");
                    delay_ms(1000);
                    while(1){
                        if(AS608_isFingerAvailable() == 0){
                        delay_ms(200);
                            if(AS608_isFingerAvailable() == 0){
                                break;
                            }
                        }
                    }
                }
            }
        }
        if(AS608_PS_RegModel() == 0){
            OLED096_clearScreen();
            OLED096_writeLine(1, OLED096_Align_Center, 0, "Saving...");
            uint32_t addr;
            uint8_t data[8] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            if(type == UserType_Admin){
                AS608_PS_StoreChar(1, index);
                addr = 0x00000000 + 0x0000100 + (uint32_t)index * 0x08;
                data[7] = index;
            }
            else if(type == UserType_User){
                AS608_PS_StoreChar(1, index + 30);
                addr = 0x00000300 + 0x0000100 + (uint32_t)index * 0x08;
                data[7] = index + 30;
            }
            W25QXX_write(data, addr, 8);
            OLED096_clearScreen();
            OLED096_writeLine(1, OLED096_Align_Center, 0, "Fingerprint");
            OLED096_writeLine(2, OLED096_Align_Center, 0, "Saved.");
            delay_ms(2000);
        }
        else{
            OLED096_clearScreen();
            OLED096_writeLine(1, OLED096_Align_Center, 0, "Failed to Add");
            OLED096_writeLine(2, OLED096_Align_Center, 0, "Fingerprint.");
            delay_ms(2000);
        }
        return 0;
    }
    else{
        return 1;
    }
}

uint8_t removeFingerprint(UserType type, uint8_t index){
    if(index < 30){
        uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint32_t addr;
        if(type == UserType_Admin){
            addr = 0x00000000 + 0x00000100 + (uint32_t)index * 0x08;
            AS608_PS_DeleteChar(index, 1);
        }
        else if(type == UserType_User){
            addr = 0x00000300 + 0x00000100 + (uint32_t)index * 0x08;
            AS608_PS_DeleteChar(index + 30, 1);
        }
        W25QXX_write(data, addr, 8);
        return 0;
    }
    else{
        return 1;
    }
}

uint8_t readFlashCard(UserType type, uint8_t index, uint8_t* exist, uint8_t* uidArray){
    if(index < 30){
        uint8_t data[8];
        uint32_t addr;
        if(type == UserType_Admin){
            addr = 0x00000000 + 0x00000200 + (uint32_t)index * 0x08;
        }
        else if(type == UserType_User){
            addr = 0x00000300 + 0x00000200 + (uint32_t)index * 0x08;
        }
        W25QXX_read(data, addr, 8);
        if(data[0] == 0x00){
            *exist = 0;
        }
        else if(data[0] == 0xFF){
            *exist = 1;
            for(uint8_t i = 0; i < 4; i++){
                uidArray[i] = data[i + 4];
            }
        }
        return 0;
    }
    else{
        return 1;
    }
}

uint8_t registerCardOp(UserType type, uint8_t index){
    if(index < 30){
        OLED096_clearScreen();
        OLED096_writeLine(1, OLED096_Align_Center, 0, "Put New Card");
        OLED096_writeLine(2, OLED096_Align_Center, 0, "on NFC Reader.");
        uint8_t uid[4];
        uint8_t cardType[2];
        while(1){
            if(RC522_pcdRequest(PICC_REQIDL, cardType) == MI_OK){
                RC522_pcdRequest(PICC_REQIDL, cardType);
                delay_ms(300);
                if(RC522_pcdRequest(PICC_REQIDL, cardType) == MI_OK){
                    if(RC522_pcdAnticoll(uid) == MI_OK){
                        break;
                    }
                }
            }
            delay_ms(50);
        }
        playSound(Sound_CardRead); //250ms
        OLED096_clearScreen();
        OLED096_writeLine(1, OLED096_Align_Center, 0, "Found Card.");
        delay_ms(500);
        OLED096_clearScreen();
        OLED096_writeLine(1, OLED096_Align_Center, 0, "Registering");
        OLED096_writeLine(2, OLED096_Align_Center, 0, "...");
        uint8_t data[8] = {0xFF, 0x00, 0x00, 0x00, uid[0], uid[1], uid[2], uid[3]};
        uint32_t addr;
        if(type == UserType_Admin){
            addr = 0x00000000 + 0x00000200 + (uint32_t)index * 0x08;
        }
        else if(type == UserType_User){
            addr = 0x00000300 + 0x00000200 + (uint32_t)index * 0x08;
        }
        W25QXX_write(data, addr, 8);
        delay_ms(1000);
        OLED096_clearScreen();
        OLED096_writeLine(1, OLED096_Align_Center, 0, "Card Registered.");
        delay_ms(2000);
        return 0;
    }
    else{
        return 1;
    }
}

uint8_t removeCard(UserType type, uint8_t index){
    if(index < 30){
        uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint32_t addr;
        if(type == UserType_Admin){
            addr = 0x00000000 + 0x00000200 + (uint32_t)index * 0x08;
        }
        else if(type == UserType_User){
            addr = 0x00000300 + 0x00000200 + (uint32_t)index * 0x08;
        }
        W25QXX_write(data, addr, 8);
        return 0;
    }
    else{
        return 1;
    }
}

uint8_t searchCard(uint8_t* uid, uint8_t* found, UserType* type, uint8_t* index){
    uint8_t data[8];
    uint32_t addr;
    uint8_t f = 0;
    //Search Admin
    if(f == 0){
        addr = 0x00000000 + 0x00000200;
        for(uint8_t i = 0; i < 30; i++){
            W25QXX_read(data, addr, 8);
            if(data[0] == 0xFF){
                uint8_t diff = 0;
                for(uint8_t j = 0; j < 4; j++){
                    if(uid[j] != data[4 + j]){
                        diff++;
                        break;
                    }
                }
                if(diff == 0){
                    f = 1;
                    (*type) = UserType_Admin;
                    (*index) = i;
                    break;
                }
            }
            addr += 0x08;
        }
    }
    //Search User
    if(f == 0){
        addr = 0x00000300 + 0x00000200;
        for(uint8_t i = 0; i < 30; i++){
            W25QXX_read(data, addr, 8);
            if(data[0] == 0xFF){
                uint8_t diff = 0;
                for(uint8_t j = 0; j < 4; j++){
                    if(uid[j] != data[4 + j]){
                        diff++;
                        break;
                    }
                }
                if(diff == 0){
                    f = 1;
                    (*type) = UserType_User;
                    (*index) = i;
                    break;
                }
            }
            addr += 0x08;
        }
    }
    (*found) = f;
    return 0;
}

uint8_t saveNoticeDelay(uint8_t on, uint8_t time){
    uint32_t addr = 0x00000600;
    uint8_t data[2] = {0x00, time};
    if(on != 0){
       data[0] = 0xFF;
    }
    W25QXX_write(data, addr, 2);
    return 0;
}

uint8_t readNoticeDelay(uint8_t* on, uint8_t* time){
    uint32_t addr = 0x00000600;
    uint8_t data[2];
    W25QXX_read(data, addr, 2);
    if(data[0] == 0xFF){
        (*on) = 1;
    }
    else{
        (*on) = 0;
    }
    (*time) = data[1];
    return 0;
}

void displayWindow(uint8_t wID){
    if(wID == 1){
        OLED096_writeLine(0, OLED096_Align_Center, cursorArray[cursor][0], "Admin Accounts");
        OLED096_writeLine(1, OLED096_Align_Center, cursorArray[cursor][1], "User Accounts");
        OLED096_writeLine(2, OLED096_Align_Center, cursorArray[cursor][2], "Other Settings");
        OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], "Back       Enter");
    }
    else if(wID == 2){
        OLED096_writeLine(0, OLED096_Align_Center, cursorArray[cursor][0], "Passwords");
        OLED096_writeLine(1, OLED096_Align_Center, cursorArray[cursor][1], "Fingerprints");
        OLED096_writeLine(2, OLED096_Align_Center, cursorArray[cursor][2], "IC Cards");
        OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], "Back       Enter");
    }
    else if(wID == 3){
        for(uint8_t i = 0; i < 3; i++){
            uint8_t exist;
            uint8_t passwd[5];
            readFlashPassword(flag_choosedUserType, flag_listBegin + i, &exist, passwd);
            if(exist == 0){
                sprintf(str, "[PW%02u] Empty", flag_listBegin + i + 1);
            }
            else if(exist == 1){
                sprintf(str, "[PW%02u]%02X%02X%02X%02X%02X", flag_listBegin + i + 1, passwd[0], passwd[1], passwd[2], passwd[3], passwd[4]);
            }
            OLED096_writeLine(i, OLED096_Align_Left, cursorArray[cursor][i], str);
        }
        OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], "Back       Enter");
    }
    else if(wID == 4){
        for(uint8_t i = 0; i < passwdContainerLength; i++){
            str[i] = passwdContainer[i] + 48;
        }
        str[passwdContainerLength] = '\0';
        OLED096_writeLine(0, OLED096_Align_Center, cursorArray[cursor][0], "New Password:");
        OLED096_writeLine(1, OLED096_Align_Center, cursorArray[cursor][1], str);
        OLED096_writeLine(2, OLED096_Align_Center, cursorArray[cursor][2], "");
        OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], "Back       Enter");
    }
    else if(wID == 5){
        OLED096_writeLine(0, OLED096_Align_Center, cursorArray[cursor][0], "Delete");
        OLED096_writeLine(1, OLED096_Align_Center, cursorArray[cursor][1], "Password?");
        OLED096_writeLine(2, OLED096_Align_Center, cursorArray[cursor][2], "");
        OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], "No           Yes");
    }
    else if(wID == 6){
        for(uint8_t i = 0 ; i < 3; i++){
            uint8_t exist;
            uint8_t fpPosition;
            readFlashFingerprint(flag_choosedUserType, flag_listBegin + i, &exist, &fpPosition);
            if(exist == 1){
                sprintf(str, "[FP%02u] %02u", flag_listBegin + i + 1, fpPosition);
            }
            else if(exist == 0){
                sprintf(str, "[FP%02u] Empty", flag_listBegin + i + 1);
            }
            OLED096_writeLine(i, OLED096_Align_Left, cursorArray[cursor][i], str);
        }
        OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], "Back       Enter");
    }
    else if(wID == 7){
        OLED096_writeLine(0, OLED096_Align_Center, cursorArray[cursor][0], "Ready to Record");
        OLED096_writeLine(1, OLED096_Align_Center, cursorArray[cursor][1], "Fingerprint.");
        OLED096_writeLine(2, OLED096_Align_Center, cursorArray[cursor][2], "");
        OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], "Back        Next");
    }
    else if(wID == 8){
        OLED096_writeLine(0, OLED096_Align_Center, cursorArray[cursor][0], "Delete");
        OLED096_writeLine(1, OLED096_Align_Center, cursorArray[cursor][1], "Fingerprint?");
        OLED096_writeLine(2, OLED096_Align_Center, cursorArray[cursor][2], "");
        OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], "No           Yes");
    }
    else if(wID == 9){
        for(uint8_t i = 0; i < 3; i++){
            uint8_t exist;
            uint8_t uid[4];
            readFlashCard(flag_choosedUserType, flag_listBegin + i, &exist, uid);
            if(exist == 0){
                sprintf(str, "[CD%02u] Empty", flag_listBegin + i + 1);
            }
            else if(exist == 1){
                sprintf(str, "[CD%02u] %02X%02X%02X%02X", flag_listBegin + i + 1, uid[0], uid[1], uid[2], uid[3]);
            }
            OLED096_writeLine(i, OLED096_Align_Left, cursorArray[cursor][i], str);
        }
        OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], "Back       Enter");
    }
    else if(wID == 10){
        OLED096_writeLine(0, OLED096_Align_Center, cursorArray[cursor][0], "Ready to Add");
        OLED096_writeLine(1, OLED096_Align_Center, cursorArray[cursor][1], "New Card.");
        OLED096_writeLine(2, OLED096_Align_Center, cursorArray[cursor][2], "");
        OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], "Back        Next");
    }
    else if(wID == 11){
        OLED096_writeLine(0, OLED096_Align_Center, cursorArray[cursor][0], "");
        OLED096_writeLine(1, OLED096_Align_Center, cursorArray[cursor][1], "Remove Card?");
        OLED096_writeLine(2, OLED096_Align_Center, cursorArray[cursor][2], "");
        OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], "No           Yes");
    }
    else if(wID == 12){
        for(uint8_t i = 0; i < 3; i++){
            if(otherSettingsLine / 3 * 3 + i < otherSettingsLineNum){
                OLED096_writeLine(i, OLED096_Align_Center, cursorArray[cursor][i], otherSettingsText[otherSettingsLine / 3 * 3 + i]);
            }
            else{
                OLED096_writeLine(i, OLED096_Align_Center, cursorArray[cursor][i], "");
            }
        }
        OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], "Back       Enter");
    }
    else if(wID == 13){
        OLED096_writeLine(0, OLED096_Align_Center, cursorArray[cursor][0], "Are you sure");
        OLED096_writeLine(1, OLED096_Align_Center, cursorArray[cursor][1], "to remove all");
        OLED096_writeLine(2, OLED096_Align_Center, cursorArray[cursor][2], "accounts?");
        OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], "No           Yes");
    }
    else if(wID == 14){
        OLED096_writeLine(0, OLED096_Align_Center, cursorArray[cursor][0], "About");
        for(uint8_t i = 0; i < 3; i++){
            if(aboutPageLine / 3 * 3 + i < aboutPageLineNum){
                OLED096_writeLine(i + 1, OLED096_Align_Center, cursorArray[cursor][i + 1], aboutText[aboutPageLine / 3 * 3 + i]);
            }
            else{
                OLED096_writeLine(i + 1, OLED096_Align_Center, cursorArray[cursor][i + 1], "");
            }
        }
    }
    else if(wID == 15){
        if(noticeOn == 0){
            OLED096_writeLine(0, OLED096_Align_Center, cursorArray[cursor][0], "Mode  <  Off >");
            OLED096_writeLine(1, OLED096_Align_Center, cursorArray[cursor][1], "Delay <  NA  >");
        }
        else{
            OLED096_writeLine(0, OLED096_Align_Center, cursorArray[cursor][0], "Mode  <  On  >");
            sprintf(str, "Delay <  %us >", noticeDelay);
            OLED096_writeLine(1, OLED096_Align_Center, cursorArray[cursor][1], str);
        }
        OLED096_writeLine(2, OLED096_Align_Center, cursorArray[cursor][2], "");
        OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], "Back        Save");
    }
    else if(wID == 16){
        OLED096_writeLine(0, OLED096_Align_Center, cursorArray[cursor][0], "Are you sure");
        OLED096_writeLine(1, OLED096_Align_Center, cursorArray[cursor][1], "to reset all");
        OLED096_writeLine(2, OLED096_Align_Center, cursorArray[cursor][2], "settings?");
        OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], "No           Yes");
    }
    else if(wID == 50){
        OLED096_writeLine(0, OLED096_Align_Center, cursorArray[cursor][0], "");
        OLED096_writeLine(1, OLED096_Align_Center, cursorArray[cursor][1], "Door Locked");
        OLED096_writeLine(2, OLED096_Align_Center, cursorArray[cursor][2], "");
        if(hiddenMode_w50 == 0){
            for(uint8_t i = 0; i < passwdContainerLength; i++){
                str[i] = passwdContainer[i] + 48;
            }
            str[passwdContainerLength] = '\0';
            OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], str);
        }
        else{
            str[0] = 'C';
            for(uint8_t i = 0; i < inputContainerLength; i++){
                str[i + 1] = inputContainer[i] + 48;
            }
            str[inputContainerLength + 1] = '\0';
            OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], str);
        }
    }
    else if(wID == 51){
        OLED096_writeLine(0, OLED096_Align_Center, cursorArray[cursor][0], "");
        OLED096_writeLine(1, OLED096_Align_Center, cursorArray[cursor][1], "Settings");
        OLED096_writeLine(2, OLED096_Align_Center, cursorArray[cursor][2], "");
        for(uint8_t i = 0; i < passwdContainerLength; i++){
            str[i] = passwdContainer[i] + 48;
        }
        str[passwdContainerLength] = '\0';
        OLED096_writeLine(3, OLED096_Align_Center, cursorArray[cursor][3], str);
    }
}

void setLedAction(LedAction action){
    if(action == LedAction_Locked){
        Led_setColor(Led_Color_Red);
        Led_setBlink(1, 200, 1800);
    }
    else if(action == LedAction_Unlocked){
        Led_setColor(Led_Color_Green);
        Led_setBlink(0, 0, 0);
    }
    else if(action == LedAction_AuthError){
        Led_setColor(Led_Color_Yellow);
        Led_setBlink(1, 250, 250);
    }
}

void playSound(Sound s){
    if(1){
    if(s == Sound_KeyPressed){ //70ms
        Beep_setFrequency(1520);
        Beep_setEnable(1);
        delay_ms(70);
        Beep_setEnable(0);
    }
    else if(s == Sound_Unlocked){ //300ms
        Beep_setFrequency(1278);
        Beep_setEnable(1);
        delay_ms(80);
        Beep_setEnable(0);
        delay_ms(20);
        Beep_setFrequency(1520);
        Beep_setEnable(1);
        delay_ms(80);
        Beep_setEnable(0);
        delay_ms(20);
        Beep_setFrequency(2028);
        Beep_setEnable(1);
        delay_ms(80);
        Beep_setEnable(0);
        delay_ms(20);
    }
    else if(s == Sound_Locked){ //300ms
        Beep_setFrequency(1520);
        Beep_setEnable(1);
        delay_ms(80);
        Beep_setEnable(0);
        delay_ms(20);
        Beep_setFrequency(1278);
        Beep_setEnable(1);
        delay_ms(80);
        Beep_setEnable(0);
        delay_ms(20);
        Beep_setFrequency(1014);
        Beep_setEnable(1);
        delay_ms(80);
        Beep_setEnable(0);
        delay_ms(20);
    }
    else if(s == Sound_AuthError){ //800ms
        Beep_setFrequency(1520);
        Beep_setEnable(1);
        delay_ms(800);
        Beep_setEnable(0);
    }
    else if(s == Sound_CardRead){ //250ms
        Beep_setFrequency(2700);
        Beep_setEnable(1);
        delay_ms(250);
        Beep_setEnable(0);
    }
    else if(s == Sound_FingerprintCollected){ //120ms
        Beep_setFrequency(920);
        Beep_setEnable(1);
        delay_ms(125);
        Beep_setEnable(0);
    }
    else if(s == Sound_Notice){
        Beep_setFrequency(2850);
        Beep_setEnable(1);
        delay_ms(100);
        Beep_setEnable(0);
        delay_ms(50);
        Beep_setFrequency(2150);
        Beep_setEnable(1);
        delay_ms(100);
        Beep_setEnable(0);
        delay_ms(50);
        //delay_ms(800);
    }
    }
}
