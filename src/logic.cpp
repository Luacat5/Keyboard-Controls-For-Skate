#include "logic.h"
#include <cmath>

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

const int maxThumbRadius = 32766;
const int stickDeadZone = 60;

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
        if (Vect[i] < stickDeadZone && Vect[i] > -stickDeadZone){
            Vect[i] = 0;
        } else {
            Vect[i] *= speed;           
        }
    }
}

float GetRightClampFac(){
    if (GetKeyDown("right_stick_multiplier")) {
        return config["special_keys"]["right_stick_multiplier_value"].get<float>();
    }

    return 1.0f;
}

float theta = 0.0;
const float thetaSpeed = M_PI/12;
const float snapDistance = M_PI/2;
const int rReturnTick = 10; // in ms (while loop iteration)
int rReturnTimer = 0;

const float rReturnSpeed = 0.7;

void rightStickSolveLegacy(float rAccel){
    ReturnToZeros(RStickVelocity, rReturnSpeed);

    float scaling = GetRightClampFac();

    if (GetKeyDown("right_stick_up")) {
            RStickVelocity[1] += maxThumbRadius * rAccel * scaling;
        }
        if (GetKeyDown("right_stick_down")) {
            RStickVelocity[1] -= maxThumbRadius * rAccel * scaling;
        }
        if (GetKeyDown("right_stick_left")) {
            RStickVelocity[0] -= maxThumbRadius * rAccel * scaling;
        }
        if (GetKeyDown("right_stick_right")) {
            RStickVelocity[0] += maxThumbRadius * rAccel * scaling;
        }

        RStickVelocity[0] = AbsClamp(RStickVelocity[0], maxThumbRadius * scaling);
        RStickVelocity[1] = AbsClamp(RStickVelocity[1], maxThumbRadius * scaling);
}

float GetTargetTheta(){

    float resultTheta = 0.0;
    double X = 0;
    double Y = 0;

    if (GetKeyDown("right_stick_up")){
        Y += 1;
    }
    if (GetKeyDown("right_stick_down")){
        Y -= 1;
    }

    if (GetKeyDown("right_stick_right")){
        X += 1;
    }
    if (GetKeyDown("right_stick_left")){
        X -= 1;
    }

    float dist = 2;
    if (X != 0 ^ Y != 0) dist = 1;
    //thanks xor

    if (X == 0 && Y == 0){
        return -808;
    }

    if (Y > 0){
        resultTheta = acos(X/dist);
    } else {
        resultTheta = 2 * M_PI - acos(X/dist);
    } // fits the theta properly

    return resultTheta;
}

void rightStickSolveRadial(float rAccel){
    float scaling = GetRightClampFac();

     float tTheta = GetTargetTheta();

    bool rMovement = GetKeyDown("right_stick_down") || GetKeyDown("right_stick_up") || GetKeyDown("right_stick_left") || GetKeyDown("right_stick_right"); 
    bool rStickDisplaced = RStickVelocity[0] != 0 || RStickVelocity[1] != 0;

    rMovement = rMovement && !(tTheta == -808);

    if (rMovement) {
        rReturnTimer = 0;
        if (abs(tTheta - theta) >= snapDistance){
            theta = tTheta;
            //flick support
            std:: cout <<  "snapped\n";
        } else {
            theta += (tTheta - theta) * 0.3; // where 0.3 is lerp speed.
        }


        RStickVelocity[0] = cos(theta) * maxThumbRadius * scaling;
        RStickVelocity[1] = sin(theta) * maxThumbRadius * scaling;

    }  else if (rStickDisplaced) {
        rReturnTimer++;

        if (rReturnTimer >= rReturnTick){
            theta = -808; //out of bounds, instant snap.
            ReturnToZeros(RStickVelocity, rReturnSpeed);
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


    loadConfig("controller_config.json");
    MakeKeyMap();


    std::cout << "Running main logic loop...\n";

    while (true)
    {
    
        XUSB_REPORT report;
        ZeroMemory(&report, sizeof(XUSB_REPORT));
        
        const float leftAccel = config["left_stick"]["acceleration"].get<float>();
        const float rightAccel = config["right_stick"]["acceleration"].get<float>();

        ReturnToZeros(LStickVelocity, 0.7);
        // left stick
        if (GetKeyDown("left_stick_up")) {
            LStickVelocity[1] += maxThumbRadius * leftAccel;
        }
        if (GetKeyDown("left_stick_down")) {
            LStickVelocity[1] -= maxThumbRadius * leftAccel;
        }
        if (GetKeyDown("left_stick_left")) {
            LStickVelocity[0] -= maxThumbRadius * leftAccel;
        }
        if (GetKeyDown("left_stick_right")) {
            LStickVelocity[0] += maxThumbRadius * leftAccel;
        }

        LStickVelocity[0] = AbsClamp(LStickVelocity[0], maxThumbRadius);
        LStickVelocity[1] = AbsClamp(LStickVelocity[1], maxThumbRadius);

        report.sThumbLX = LStickVelocity[0];
        report.sThumbLY = LStickVelocity[1];
        // right stick
        rightStickSolveRadial(rightAccel);
        report.sThumbRX = RStickVelocity[0];
        report.sThumbRY = RStickVelocity[1];

        // face buttons
        if (GetKeyDown("Square"))   report.wButtons |= XUSB_GAMEPAD_X;
        if (GetKeyDown("X"))        report.wButtons |= XUSB_GAMEPAD_A;
        if (GetKeyDown("Circle"))   report.wButtons |= XUSB_GAMEPAD_B;
        if (GetKeyDown("Triangle")) report.wButtons |= XUSB_GAMEPAD_Y;

        // dpad
        if (GetKeyDown("dpad_up"))    report.wButtons |= XUSB_GAMEPAD_DPAD_UP;
        if (GetKeyDown("dpad_right")) report.wButtons |= XUSB_GAMEPAD_DPAD_RIGHT;
        if (GetKeyDown("dpad_left"))  report.wButtons |= XUSB_GAMEPAD_DPAD_LEFT;
        if (GetKeyDown("dpad_down"))  report.wButtons |= XUSB_GAMEPAD_DPAD_DOWN;

        // triggers
        if (GetKeyDown("L2")) report.bLeftTrigger  = 255;
        if (GetKeyDown("R2")) report.bRightTrigger = 255;

        // shoulders / sticks
        if (GetKeyDown("L1")) report.wButtons |= XUSB_GAMEPAD_LEFT_SHOULDER;
        if (GetKeyDown("R1")) report.wButtons |= XUSB_GAMEPAD_RIGHT_SHOULDER;
        if (GetKeyDown("L3")) report.wButtons |= XUSB_GAMEPAD_LEFT_THUMB;
        if (GetKeyDown("R3")) report.wButtons |= XUSB_GAMEPAD_RIGHT_THUMB;

        // start/back
        if (GetKeyDown("PAUSE"))  report.wButtons |= XUSB_GAMEPAD_START;
        if (GetKeyDown("SELECT")) report.wButtons |= XUSB_GAMEPAD_BACK;

        // exit key
        if (GetKeyDown("EXIT")) {
            std::cout << "Exit key pressed, shutting down...\n";
            break;
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


