#include "chip8.h"
#include "ConsoleRenderer.h"

#include <windows.h>
#include <chrono>

void ClearConsoleInputBuffer() {
    INPUT_RECORD ClearingVar1[256];
    DWORD ClearingVar2;
    ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), ClearingVar1, 256, &ClearingVar2);
}

int main() {
    chip8 c8;
    //if (!c8.LoadProgram("Programs\\PONG"))
    if (!c8.LoadProgram("Programs\\PONG2")) {
        return 1;
    }

    ConsoleRenderer renderer;
    std::chrono::milliseconds lastLoopMiliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
    while (true) {
        std::chrono::milliseconds currentLoopMiliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
        if ((currentLoopMiliseconds - lastLoopMiliseconds).count() <= 2) {
            // Only execute at 500 Hz
            continue;
        }

        c8.StepEmulation((currentLoopMiliseconds - lastLoopMiliseconds).count());
        lastLoopMiliseconds = currentLoopMiliseconds;

        if (c8.NeedsRender()) {
            auto graphics = c8.GetGraphics();
            renderer.Render(graphics);
        }


        c8.SetKey(0x1, GetAsyncKeyState('1') & 0x8000);
        c8.SetKey(0x2, GetAsyncKeyState('2') & 0x8000);
        c8.SetKey(0x3, GetAsyncKeyState('3') & 0x8000);
        c8.SetKey(0xC, GetAsyncKeyState('4') & 0x8000);

        c8.SetKey(0x4, GetAsyncKeyState('Q') & 0x8000);
        c8.SetKey(0x5, GetAsyncKeyState('W') & 0x8000);
        c8.SetKey(0x6, GetAsyncKeyState('E') & 0x8000);
        c8.SetKey(0xD, GetAsyncKeyState('R') & 0x8000);

        c8.SetKey(0x7, GetAsyncKeyState('A') & 0x8000);
        c8.SetKey(0x8, GetAsyncKeyState('S') & 0x8000);
        c8.SetKey(0x9, GetAsyncKeyState('D') & 0x8000);
        c8.SetKey(0xE, GetAsyncKeyState('F') & 0x8000);

        c8.SetKey(0xA, GetAsyncKeyState('Z') & 0x8000);
        c8.SetKey(0x0, GetAsyncKeyState('X') & 0x8000);
        c8.SetKey(0xB, GetAsyncKeyState('C') & 0x8000);
        c8.SetKey(0xF, GetAsyncKeyState('V') & 0x8000);

        // TODO: Time the frames and do a specific FPS
        //Sleep(1);
    }

    return 0;
}
