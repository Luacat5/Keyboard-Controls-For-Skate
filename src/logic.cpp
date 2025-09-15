#include "logic.h"
#include "mapping.h"
#include "config.h"

#include <iostream>
#include <Xinput.h>
#include <VigEm/Client.h>
#include <unordered_map>
#include <windows.h>
#include <fstream>

// #pragma comment (lib, "setupapi.lib")



static PVIGEM_CLIENT client = nullptr;
static PVIGEM_TARGET pad = nullptr;

bool initController()
{
    if (!client)
    {
        client = vigem_alloc();
        if (!client)
        {
            std::cerr << "Failed to allocate ViGEm client!" << std::endl;
            return false;
        }

        const auto retval = vigem_connect(client);
        if (!VIGEM_SUCCESS(retval))
        {
            std::cerr << "ViGEmBus connection failed: 0x" << std::hex << retval << std::endl;
            vigem_free(client);
            client = nullptr;
            return false;
        }
    }

    if (!pad)
    {
        pad = vigem_target_x360_alloc();
        const auto pir = vigem_target_add(client, pad);
        if (!VIGEM_SUCCESS(pir))
        {
            std::cerr << "Failed to add X360 target: 0x" << std::hex << pir << std::endl;
            vigem_target_free(pad);
            pad = nullptr;
            return false;
        }
    }

    std::cout << "Controller initialized and persistent target ready!\n";
    return true;
}

const int maxThumbRadius = 32767;


int RStickVelocity[2] = {0,0};
int LStickVelocity[2] = {0,0};

int AbsClamp(int x, int c){
    if (x > c){
        return c;
    } if (x < -c){
        return -c;
    }
    return x;
}

void ReturnToZeros(int Vect[2], float speed){
    for (int i = 0; i < 2; i++){
        if (Vect[i] > 200 || Vect[0] < 200){
            Vect[i] *= speed;
        } else {
            Vect[i] = 0;
        }
    }
}


void mainLogicLoop()
{

   if (!client || !pad)
    {
        std::cerr << "Controller not initialized!\n";
        return;
    }

    std::cout << "Running main logic loop...\n";

    loadConfig("controller_config.json");
    std::unordered_map<std::string, int> keyMap = MakeKeyMap();

    while (true)
    {
        // DEBUG
        // std::cout << "LStick:" << LStickVelocity[0] << ", " << LStickVelocity[1] << std::endl;
        // std::cout << "RStick:" << RStickVelocity[0] << ", " << RStickVelocity[1] << std::endl;
    
        XUSB_REPORT report;
        ZeroMemory(&report, sizeof(XUSB_REPORT));
        
        const float leftAccel = config["left_stick"]["acceleration"].get<float>();
        const float rightAccel = config["right_stick"]["acceleration"].get<float>();

        bool lMovement = false;

        ReturnToZeros(RStickVelocity, 0.7);
        ReturnToZeros(LStickVelocity, 0.7);

        // left stick:
        if (GetAsyncKeyState(keyMap["left_stick_up"]) & 0x8000) {
            LStickVelocity[1] += maxThumbRadius * leftAccel; // max up
            lMovement = true;
        }
        if (GetAsyncKeyState(keyMap["left_stick_down"]) & 0x8000) {
            LStickVelocity[1] -= maxThumbRadius * leftAccel; // max down
            lMovement = true;
        }
        if (GetAsyncKeyState(keyMap["left_stick_left"]) & 0x8000) {
            LStickVelocity[0] -= maxThumbRadius * leftAccel;
            lMovement = true;
        }
        if (GetAsyncKeyState(keyMap["left_stick_right"]) & 0x8000) {
            LStickVelocity[0] += maxThumbRadius * leftAccel;
            lMovement = true;
        }
        
       
        
        LStickVelocity[0] = AbsClamp(LStickVelocity[0], maxThumbRadius);
        LStickVelocity[1] = AbsClamp(LStickVelocity[1], maxThumbRadius);


        report.sThumbLX = LStickVelocity[0];
        report.sThumbLY = LStickVelocity[1];

        float scaling = 1.0;
        if (GetAsyncKeyState(keyMap["right_stick_multiplier"]) & 0x8000) {
            scaling = config["special_keys"]["right_stick_multiplier_value"].get<float>();
        }

        bool rMovement = false;

        // right stick:
        if (GetAsyncKeyState(keyMap["right_stick_up"]) & 0x8000) {
            RStickVelocity[1] += maxThumbRadius * rightAccel * scaling;
            rMovement = true; 
        }
        if (GetAsyncKeyState(keyMap["right_stick_down"]) & 0x8000) {
            RStickVelocity[1] -= maxThumbRadius * rightAccel * scaling; 
            rMovement = true; 
        }
        if (GetAsyncKeyState(keyMap["right_stick_left"]) & 0x8000) {
            RStickVelocity[0] -= maxThumbRadius * rightAccel * scaling;
            rMovement = true;
        }
        if (GetAsyncKeyState(keyMap["right_stick_right"]) & 0x8000) {
            RStickVelocity[0] += maxThumbRadius * rightAccel * scaling;
            rMovement = true;
        }

       
        RStickVelocity[0] = AbsClamp(RStickVelocity[0], maxThumbRadius * scaling);
        RStickVelocity[1] = AbsClamp(RStickVelocity[1], maxThumbRadius * scaling);

        report.sThumbRX = RStickVelocity[0];
        report.sThumbRY = RStickVelocity[1];

        // face buttons
        if (GetAsyncKeyState(keyMap["Square"]) & 0x8000) {
            report.wButtons |= XUSB_GAMEPAD_X; // Square
        }
        if (GetAsyncKeyState(keyMap["X"]) & 0x8000) {
            report.wButtons |= XUSB_GAMEPAD_A; // X
        }
        if (GetAsyncKeyState(keyMap["Circle"]) & 0x8000) {
            report.wButtons |= XUSB_GAMEPAD_B; // Circle
        }
        if (GetAsyncKeyState(keyMap["Triangle"]) & 0x8000) {
            report.wButtons |= XUSB_GAMEPAD_Y; // Triangle
        }

        //dpad
        if (GetAsyncKeyState(keyMap["dpad_up"]) & 0x8000) {
            report.wButtons |= XUSB_GAMEPAD_DPAD_UP; 
        }
        if (GetAsyncKeyState(keyMap["dpad_right"]) & 0x8000) {
            report.wButtons |= XUSB_GAMEPAD_DPAD_RIGHT; 
        }
        if (GetAsyncKeyState(keyMap["dpad_left"]) & 0x8000) {
            report.wButtons |= XUSB_GAMEPAD_DPAD_LEFT; 
        }
        if (GetAsyncKeyState(keyMap["dpad_down"]) & 0x8000) {
            report.wButtons |= XUSB_GAMEPAD_DPAD_DOWN; 
        }
        
        
        //triggers:

        if (GetAsyncKeyState(keyMap["L2"]) & 0x8000) {
            report.bLeftTrigger = 255; 
        }
        if (GetAsyncKeyState(keyMap["R2"]) & 0x8000) {
            report.bRightTrigger = 255; 
        }


    
        if (GetAsyncKeyState(keyMap["L1"]) & 0x8000) {
            report.wButtons |= XUSB_GAMEPAD_LEFT_SHOULDER; 
        }
        if (GetAsyncKeyState(keyMap["R1"]) & 0x8000) {
            report.wButtons |= XUSB_GAMEPAD_RIGHT_SHOULDER; 
        }

        if (GetAsyncKeyState(keyMap["L3"]) & 0x8000) {
            report.wButtons |= XUSB_GAMEPAD_LEFT_THUMB; 
        }
        if (GetAsyncKeyState(keyMap["R3"]) & 0x8000) {
            report.wButtons |= XUSB_GAMEPAD_RIGHT_THUMB; 
        }


        if (GetAsyncKeyState(keyMap["PAUSE"]) & 0x8000) {
            report.wButtons |= XUSB_GAMEPAD_START; 
        }
        if (GetAsyncKeyState(keyMap["SELECT"]) & 0x8000) {
            report.wButtons |= XUSB_GAMEPAD_BACK; 
        }

        vigem_target_x360_update(client, pad, report);
        Sleep(config["poll_interval_ms"].get<int>());
    }

    vigem_target_remove(client, pad);
    vigem_target_free(pad);
    pad = nullptr;

    vigem_disconnect(client);
    vigem_free(client);
    client = nullptr;
}


