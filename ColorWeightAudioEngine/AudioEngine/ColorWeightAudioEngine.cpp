#include <AK/SoundEngine/Common/AkSoundEngine.h>                // Sound Engine
#include <AK/SoundEngine/Common/AkMemoryMgr.h>                  // Memory Manager interface
#include <AK/SoundEngine/Common/AkModule.h>                     // Default memory manager
#include <AK/SoundEngine/Common/IAkStreamMgr.h>                 // Streaming Manager
#include <AK/Tools/Common/AkPlatformFuncs.h>                    // Thread defines
#include <AkFilePackageLowLevelIOBlocking.h>                    // Sample low-level I/O implementation
#include <AK/Comm/AkCommunication.h>                            // Communicate with profiler
#include <iostream>
#include <thread>
#include <cassert>

// We're using the default Low-Level I/O implementation that's part
// of the SDK's sample code, with the file package extension
CAkFilePackageLowLevelIOBlocking g_lowLevelIO;

bool initSoundEngine();
void initSoundBanks();
void playSound();
void termSoundEngine();
void processAudio();
void connectProfiler();

namespace AK
{
    void* AllocHook(size_t s) { return malloc(s); }
    void FreeHook(void* p) { free(p); }
}

int main()
{
    if (initSoundEngine())
    {
        initSoundBanks();
        //connectProfiler();
        playSound();
        int frameCount = 0;
        while (frameCount++ <= 300)
        {
            auto time = std::chrono::milliseconds(30);
            std::this_thread::sleep_for(time);
            processAudio();
        }
        termSoundEngine();
        return 0;
    }
    else
    {
        return -1;
    }
}

#pragma region InitSoundEngine
bool initSoundEngine()
{
#pragma region Memory Manager
    // Init Memory Manager
    AkMemSettings memSettings;
    AK::MemoryMgr::GetDefaultSettings(memSettings);
    if (AK::MemoryMgr::Init(&memSettings) != AK_Success)
    {
        assert(!"Could not create the memory manager.");
        return false;
    }
#pragma endregion
#pragma region Streaming Settings
    //
   // Create and initialize an instance of the default streaming manager. Note
   // that you can override the default streaming manager with your own. 
   //
    AkStreamMgrSettings stmSettings;
    AK::StreamMgr::GetDefaultSettings(stmSettings);
    // Customize the Stream Manager settings here.
    if (!AK::StreamMgr::Create(stmSettings))
    {
        assert(!"Could not create the Streaming Manager");
        return false;
    }
#pragma endregion
#pragma region Low-level IO Streaming
    //
    // Create a streaming device with blocking low-level I/O handshaking.
    // Note that you can override the default low-level I/O module with your own. 
    //
    AkDeviceSettings deviceSettings;
    AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);
    // Customize the streaming device settings here.
    // CAkFilePackageLowLevelIOBlocking::Init() creates a streaming device
    // in the Stream Manager, and registers itself as the File Location Resolver.
    if (g_lowLevelIO.Init(deviceSettings) != AK_Success)
    {
        assert(!"Could not create the streaming device and Low-Level I/O system");
        return false;
    }
#pragma endregion
#pragma region Sound Engine
    //
   // Create the Sound Engine
   // Using default initialization parameters
   //
    AkInitSettings initSettings;
    AkPlatformInitSettings platformInitSettings;
    AK::SoundEngine::GetDefaultInitSettings(initSettings);
    AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);
    if (AK::SoundEngine::Init(&initSettings, &platformInitSettings) != AK_Success)
    {
        assert(!"Could not initialize the Sound Engine.");
        return false;
    }
    return true;
#pragma endregion
}
#pragma endregion
#pragma region TermSoundEngine
void termSoundEngine()
{
    AK::SoundEngine::UnloadBank(AKTEXT("Init.bnk"), nullptr);
    AK::SoundEngine::UnloadBank(AKTEXT("TestBank.bnk"), nullptr);
    // Terminate Sound Engine
    AK::SoundEngine::Term();
    // Terminate the streaming device and streaming manager
   // CAkFilePackageLowLevelIOBlocking::Term() destroys its associated streaming device 
   // that lives in the Stream Manager, and unregisters itself as the File Location Resolver.
    g_lowLevelIO.Term();
    
    if (AK::IAkStreamMgr::Get())
        AK::IAkStreamMgr::Get()->Destroy();
    // Terminate the Memory Manager
    AK::MemoryMgr::Term();
}
#pragma endregion
#pragma region InitBanks
void initSoundBanks()
{
    g_lowLevelIO.SetBasePath(AKTEXT("../SoundBanks"));
    AK::StreamMgr::SetCurrentLanguage(AKTEXT("English(US)"));
    AkBankID bankID;
    AK::SoundEngine::LoadBank(AKTEXT("Init.bnk"), bankID);
    AK::SoundEngine::LoadBank(AKTEXT("TestBank.bnk"), bankID);
}
#pragma endregion
#pragma region PlaySound
void playSound()
{
    AkGameObjectID myGameObj = 1;
    AK::SoundEngine::RegisterGameObj(myGameObj, "GameObj1");
    AK::SoundEngine::PostEvent(AKTEXT("PlayCombo"), myGameObj);
    AK::SoundEngine::UnregisterGameObj(myGameObj);
}
#pragma endregion
#pragma region ProcessAudio
void processAudio()
{
    AK::SoundEngine::RenderAudio();
}
#pragma endregion
#pragma region ConnectProfiler
void connectProfiler()
{
    // Connect to profiler. Waits 5 seconds.
    AkCommSettings akCommSettings;
    AK::Comm::GetDefaultInitSettings(akCommSettings);
    AK::Comm::Init(akCommSettings);
    auto time = std::chrono::seconds(5);
    std::this_thread::sleep_for(time);
}
#pragma endregion



